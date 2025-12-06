 #include "FCFSScheduler.hpp"
#include <iostream>
#include <chrono>
#include <limits>
#include <filesystem>
#include "util/Log.hpp"
#include <fstream>
#include <iomanip>
#include <ctime>
#include "TimeUtils.hpp"

namespace {

const char* to_string_state(State state) {
    switch (state) {
        case State::Ready: return "Ready";
        case State::Running: return "Running";
        case State::Blocked: return "Blocked";
        case State::Finished: return "Finished";
        case State::Failed: return "Failed";
    }
    return "Unknown";
}

}

FCFSScheduler::FCFSScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager) {
    
    std::cout << "[FCFS] Inicializando com " << num_cores << " núcleos\n";
    initialize_cores();
    
    // Inicializar contadores atômicos
    total_simulation_cycles.store(0);
    finished_count.store(0);
    failed_count.store(0);
    total_count.store(0);
    total_execution_time.store(0);
    ready_count.store(0);
    idle_cores_count.store(num_cores);
    context_switches = 0;
    
    // Registrar instante inicial da simulação (usamos start_ns em todos os escalonadores)
    start_ns = cpu_time::now_ns();
}

FCFSScheduler::~FCFSScheduler() {
    std::cout << "[FCFS] Encerrando...\n";
    shutdown_cores();
    
    // Limpar filas
    std::scoped_lock<std::mutex, std::mutex> lock(scheduler_mutex, ready_queue_mutex);
    ready_queue.clear();
    blocked_list.clear();
    finished_list.clear();
}

void FCFSScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
    }
    
    enqueue_ready_process(process);
    total_count.fetch_add(1);
    process->set_state(State::Ready);
}

void FCFSScheduler::enqueue_ready_process(PCB* process) {
    if (!process->enter_ready_queue()) {
        return;
    }
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    ready_queue.push_back(process);
    ready_count.fetch_add(1);
}

PCB* FCFSScheduler::dequeue_ready_process() {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    if (ready_queue.empty()) {
        return nullptr;
    }
    PCB* process = ready_queue.front();
    ready_queue.pop_front();
    ready_count.fetch_sub(1);
    process->leave_ready_queue();
    return process;
}

size_t FCFSScheduler::ready_queue_size() const {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    return ready_queue.size();
}

bool FCFSScheduler::ready_queue_empty() const {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    return ready_queue.empty();
}

void FCFSScheduler::collect_finished_processes() {
    for (auto& core : cores) {
        PCB* process = core->get_current_process();
        if (process == nullptr) {
            core->increment_idle_cycles(1);
            continue;
        }
        
        const bool is_idle = core->is_idle();
        const bool thread_running = core->is_thread_running();
        if (Log::info_enabled()) {
                std::cout << "[FCFS][collect] core#" << core->get_id()
                  << " P" << process->pid
                  << " state=" << to_string_state(process->get_state())
                  << " thread_running=" << (thread_running ? "yes" : "no")
                  << " idle_flag=" << (is_idle ? "yes" : "no")
                  << " finished=" << finished_count.load() << "/" << total_count.load()
                  << " ready=" << ready_count.load()
                  << " blocked=" << blocked_list.size() << "\n";
        }

        if (!is_idle && thread_running) {
            std::cout << "[FCFS][collect] core#" << core->get_id()
                      << " ainda executando P" << process->pid << " - adiando coleta\n";
            continue;
        }

        if (thread_running) {
            core->wait_completion();
        }

        core->with_process_lock([&](PCB*& guarded_process) {
            if (guarded_process == nullptr || guarded_process != process) {
                return;
            }

            const State proc_state = process->get_state();
            switch (process->get_state()) {
                case State::Finished:
                    process->finish_time = cpu_time::now_ns();
                    finished_list.push_back(process);
                    {
                        const int new_finished = finished_count.fetch_add(1) + 1;
                        std::cout << "[FCFS][collect] P" << process->pid
                                  << " FINALIZADO! contador=" << new_finished
                                  << "/" << total_count.load() << "\n";
                    }
                    break;
                case State::Failed:
                    process->finish_time = cpu_time::now_ns();
                    finished_list.push_back(process);
                    {
                        const int new_finished = finished_count.fetch_add(1) + 1;
                        failed_count.fetch_add(1);
                        std::cout << "[FCFS][collect] P" << process->pid
                                  << " FALHOU! contador=" << new_finished
                                  << "/" << total_count.load() << "\n";
                    }
                    break;
                case State::Blocked:
                    ioManager->registerProcessWaitingForIO(process);
                    blocked_list.push_back(process);
                    std::cout << "[FCFS][collect] P" << process->pid
                              << " BLOQUEADO (I/O) - blocked_list agora="
                              << blocked_list.size() << "\n";
                    break;
                default:
                    enqueue_ready_process(process);
                    std::cout << "[FCFS][collect] P" << process->pid
                              << " RE-AGENDADO - ready_count="
                              << ready_count.load() << "\n";
                    break;
            }

            guarded_process = nullptr;
            idle_cores_count.fetch_add(1);
            std::cout << "[FCFS][collect] core#" << core->get_id()
                      << " liberado; idle_cores=" << idle_cores_count.load() << "\n";
        });
    }
}

void FCFSScheduler::schedule_cycle() {
    total_execution_time.fetch_add(1);
    total_simulation_cycles.fetch_add(1);
    uint64_t current_time = total_execution_time.load();
    
    // Coletar processos finalizados antes de atribuir novos
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Verificação rápida antes de tentar lock (fast-path)
    bool should_schedule = (ready_count.load() > 0 && idle_cores_count.load() > 0);
    
    // Batch scheduling: só trava mutex a cada N ciclos ou quando necessário
    if (current_time % batch_size == 0 || should_schedule) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        
        // Desbloqueia processos do IO
        for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
            if ((*it)->get_state() == State::Ready) {
                enqueue_ready_process(*it);
                it = blocked_list.erase(it);
            } else {
                ++it;
            }
        }
        
        // Atribui processos aos núcleos livres (FIFO)
        const size_t max_attempts = ready_queue_size() * 2;
        size_t attempts = 0;
        
        while (attempts < max_attempts) {
            bool assigned = false;
            attempts++;
            
            for (auto& core : cores) {
                if (!core->is_idle()) {
                    continue;
                }
                PCB* process = dequeue_ready_process();
                if (process == nullptr) {
                    break;
                }
                    // Urgent collect: verificar se há processo antigo não coletado
                    PCB* old_process = core->get_current_process();
                    if (old_process != nullptr) {
                        if (core->is_thread_running()) {
                            core->wait_completion();
                        }
                        
                        if (old_process->get_state() == State::Finished) {
                            old_process->finish_time = cpu_time::now_ns();
                            finished_list.push_back(old_process);
                            finished_count.fetch_add(1);
                        } else if (old_process->get_state() == State::Ready) {
                            enqueue_ready_process(old_process);
                        }
                        
                        core->clear_current_process();
                        idle_cores_count.fetch_add(1);
                    }
                    
                    if (process->start_time == 0) {
                        process->start_time = cpu_time::now_ns();
                    }
                    process->assigned_core = core->get_id();
                    
                    // FCFS é não-preemptivo: quantum infinito
                    process->quantum = 999999;
                    
                    context_switches++;
                    process->context_switches++;
                    
                    idle_cores_count.fetch_sub(1);
                    core->execute_async(process);
                    assigned = true;
            }
            
            if (!assigned) {
                bool all_busy = true;
                for (auto& core : cores) {
                    if (core->is_idle()) {
                        all_busy = false;
                        break;
                    }
                }
                if (all_busy) break;
            }

            if (ready_queue_empty()) {
                break;
            }
        }
    }
    
    std::this_thread::yield();
    
    // Segunda coleta após yield
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Coleta forçada se todos cores idle mas faltam processos
    int idle = idle_cores_count.load();
    int finished = finished_count.load();
    int total = total_count.load();
    
    if (idle >= num_cores && finished < total) {
        std::this_thread::yield();
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Sleep periódico para reduzir busy-wait
    if (current_time % 100 == 0) {
        std::this_thread::yield();
    }
}

bool FCFSScheduler::all_finished() const {
    const int total = total_count.load(std::memory_order_acquire);
    if (total == 0) {
        return true;
    }

    const int finished = finished_count.load(std::memory_order_acquire);
    if (finished < total) {
        return false;
    }

    if (!ready_queue_empty()) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        if (!blocked_list.empty()) {
            return false;
        }
    }

    for (const auto& core : cores) {
        if (core->get_current_process() != nullptr || core->is_thread_running()) {
            return false;
        }
    }

    if (ioManager && !ioManager->is_idle()) {
        return false;
    }

    return true;
}

bool FCFSScheduler::has_pending_processes() const {
    const int total = total_count.load(std::memory_order_acquire);
    if (total == 0) {
        return false;
    }

    const int finished = finished_count.load(std::memory_order_acquire);
    if (finished >= total) {
        return false;
    }

    // If there are ready processes, we have pending work
    if (!ready_queue_empty()) {
        return true;
    }

    for (const auto& core : cores) {
        if (core->get_current_process() != nullptr || core->is_thread_running()) {
            return true;
        }
    }

    // If there are blocked processes (waiting for I/O), we also have pending work
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        if (!blocked_list.empty()) {
            return true;
        }
    }

    if (ioManager && !ioManager->is_idle()) {
        return true;
    }

    // Otherwise nothing pending
    return false;
}

std::vector<std::unique_ptr<Core>>& FCFSScheduler::get_cores() { return cores; }

void FCFSScheduler::initialize_cores() {
    cores.clear();
    for (int i = 0; i < num_cores; ++i) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores.back()->reset_metrics();
    }
    idle_cores_count.store(num_cores);
}

void FCFSScheduler::shutdown_cores() {
    for (auto& core : cores) {
        core->request_stop();
    }
    for (auto& core : cores) {
        core->join_thread();
    }
}

void FCFSScheduler::drain_cores() {
    shutdown_cores();
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    int pending = 0;
    for (const auto& core : cores) {
        const bool thread_running = core->is_thread_running();
        core->with_process_lock([&](PCB*& guarded_process) {
            if (guarded_process != nullptr) {
                pending++;
                std::cout << "[FCFS][drain] core#" << core->get_id()
                          << " segurando P" << guarded_process->pid
                          << " state=" << to_string_state(guarded_process->get_state())
                          << " thread_running=" << (thread_running ? "yes" : "no")
                          << "\n";
            }
        });
    }
    const int finished_before = finished_count.load(std::memory_order_relaxed);
    collect_finished_processes();
    const int finished_after = finished_count.load(std::memory_order_relaxed);
    std::cout << "[FCFS][drain] pending cores=" << pending
              << " newly_collected=" << (finished_after - finished_before)
              << " finished=" << finished_after << "/" << total_count.load()
              << " ready=" << ready_count.load()
              << " blocked=" << blocked_list.size() << "\n";
}

FCFSScheduler::Statistics FCFSScheduler::get_statistics() const {
    Statistics s;
    
    if (finished_list.empty()) return s;
    
    uint64_t total_wait_ns = 0;
    uint64_t total_turnaround_ns = 0;
    uint64_t total_response_ns = 0;
    uint64_t earliest_arrival = std::numeric_limits<uint64_t>::max();
    uint64_t latest_finish = 0;
    uint64_t total_pipeline_cycles = 0;
    
    for (const auto* pcb : finished_list) {
        if (pcb->get_state() == State::Failed) continue; // skip failed from metrics
        total_wait_ns += pcb->total_wait_time.load();
        const uint64_t tat = pcb->get_turnaround_time();
        total_turnaround_ns += tat;
        const uint64_t start_time = pcb->start_time.load();
        const uint64_t arrival = pcb->arrival_time.load();
        if (start_time > 0 && start_time >= arrival) {
            total_response_ns += (start_time - arrival);
        }
        earliest_arrival = std::min(earliest_arrival, arrival);
        latest_finish = std::max(latest_finish, pcb->finish_time.load());
        total_pipeline_cycles += pcb->pipeline_cycles.load();
    }

    if (earliest_arrival == std::numeric_limits<uint64_t>::max()) {
        earliest_arrival = latest_finish;
    }

    // Count only non-failed finished processes for metrics
    int non_failed_finished = 0;
    for (const auto* pcb : finished_list) {
        if (pcb->get_state() != State::Failed) non_failed_finished++;
    }
    s.total_processes = non_failed_finished;
    if (s.total_processes == 0) return s;
    const double inv_count = 1.0 / s.total_processes;
    s.avg_wait_time = cpu_time::ns_to_ms(static_cast<double>(total_wait_ns) * inv_count);
    s.avg_turnaround_time = cpu_time::ns_to_ms(static_cast<double>(total_turnaround_ns) * inv_count);
    s.avg_response_time = cpu_time::ns_to_ms(static_cast<double>(total_response_ns) * inv_count);
    
    const uint64_t span_ns = (latest_finish > earliest_arrival)
        ? (latest_finish - earliest_arrival)
        : 0;
    const double elapsed_seconds = cpu_time::ns_to_seconds(span_ns);
    if (elapsed_seconds > 0.0) {
        s.throughput = s.total_processes / elapsed_seconds;
    }
    
    // Prefer pipeline-cycle based metric for CPU utilization (align to base implementation)
    const uint64_t total_cycles_estimated = total_pipeline_cycles + (s.total_processes * 100);
    if (total_cycles_estimated > 0) {
        s.avg_cpu_utilization = (100.0 * static_cast<double>(total_pipeline_cycles) / static_cast<double>(total_cycles_estimated));
    } else {
        uint64_t total_busy_cycles = 0;
        uint64_t total_idle_cycles = 0;
        for (const auto& core : cores) {
            total_busy_cycles += core->get_busy_cycles();
            total_idle_cycles += core->get_idle_cycles();
        }
        const uint64_t capacity_cycles = total_busy_cycles + total_idle_cycles;
        if (capacity_cycles > 0) {
            s.avg_cpu_utilization = (static_cast<double>(total_busy_cycles) / capacity_cycles) * 100.0;
        } else if (elapsed_seconds > 0.0) {
            const double busy_seconds = static_cast<double>(total_pipeline_cycles) / CLOCK_FREQ_HZ;
            const double capacity_seconds = elapsed_seconds * num_cores;
            if (capacity_seconds > 0.0) {
                s.avg_cpu_utilization = (busy_seconds / capacity_seconds) * 100.0;
            }
        }
    }
    
    s.total_context_switches = context_switches;
    
    return s;
}

void FCFSScheduler::dump_state(const std::string& reason, uint64_t cycles, uint64_t max_cycles) const {
    namespace fs = std::filesystem;
    std::scoped_lock<std::mutex, std::mutex> lock(scheduler_mutex, ready_queue_mutex);
    fs::create_directories("logs");
    std::ofstream out("logs/scheduler_dumps.log", std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    out << "\n[FCFS] reason=" << reason
        << " cycles=" << cycles << "/" << max_cycles
        << " timestamp=" << std::put_time(std::localtime(&now_c), "%F %T") << "\n";
    out << "  finished=" << finished_count.load() << "/" << total_count.load()
        << " ready_queue=" << ready_queue.size()
        << " blocked=" << blocked_list.size()
        << " ready_count=" << ready_count.load()
        << " idle_cores=" << idle_cores_count.load()
        << " context_switches=" << context_switches << "\n";

    out << "  Ready queue:";
    if (ready_queue.empty()) {
        out << " <empty>";
    } else {
        for (const auto* pcb : ready_queue) {
            out << ' ' << pcb->pid << '(' << to_string_state(pcb->get_state()) << ')';
        }
    }
    out << "\n";

    out << "  Blocked list:";
    if (blocked_list.empty()) {
        out << " <empty>";
    } else {
        for (const auto* pcb : blocked_list) {
            out << ' ' << pcb->pid << '(' << to_string_state(pcb->get_state()) << ')';
        }
    }
    out << "\n";

    for (const auto& core : cores) {
        PCB* proc = core->get_current_process();
        out << "    core#" << core->get_id()
            << " idle=" << core->is_idle()
            << " thread_running=" << core->is_thread_running()
            << " busy_cycles=" << core->get_busy_cycles()
            << " idle_cycles=" << core->get_idle_cycles();
        if (proc) {
            out << " pid=" << proc->pid << '(' << to_string_state(proc->get_state()) << ')';
        } else {
            out << " pid=<none>";
        }
        out << "\n";
    }

    out.flush();
}
