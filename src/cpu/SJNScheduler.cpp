#include "SJNScheduler.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <limits>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "TimeUtils.hpp"
#include "../util/Log.hpp"
#include <unordered_set>

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

SJNScheduler::SJNScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager) {
    
    std::cout << "[SJN] Inicializando com " << num_cores << " núcleos\n";
    initialize_cores();
    
    // Inicializar contadores atômicos
    total_simulation_cycles.store(0);
    failed_count.store(0);
    ready_count.store(0);
    idle_cores_count.store(num_cores);
    context_switches = 0;
    
    start_ns = cpu_time::now_ns();
    // registry init
    process_registry.clear();
}

SJNScheduler::~SJNScheduler() {
    std::cout << "[SJN] Encerrando...\n";
    shutdown_cores();
    
    // Limpar filas
    std::scoped_lock<std::mutex, std::mutex> lock(scheduler_mutex, ready_queue_mutex);
    ready_queue.clear();
    blocked_list.clear();
    finished_list.clear();
}

void SJNScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
    }
    if (process->arrival_sim_time == 0) {
        process->arrival_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
    }
    
    enqueue_ready_process(process);
    total_count.fetch_add(1);
    process->set_state(State::Ready);
    process_registry[process->pid] = process;
}

void SJNScheduler::enqueue_ready_process(PCB* process) {
    const uint64_t sim_ns = total_simulation_cycles.load(std::memory_order_acquire);
    if (!process->enter_ready_queue_sim(sim_ns)) {
        return;
    }
    // Insere na fila ordenada por estimated_job_size (menor primeiro)
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    auto it = std::find_if(ready_queue.begin(), ready_queue.end(),
        [&](PCB* p) { return process->estimated_job_size < p->estimated_job_size; });
    ready_queue.insert(it, process);
    ready_count.fetch_add(1);
}

PCB* SJNScheduler::dequeue_ready_process() {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    if (ready_queue.empty()) {
        return nullptr;
    }
    PCB* process = ready_queue.front();
    ready_queue.pop_front();
    ready_count.fetch_sub(1);
                    process->leave_ready_queue_sim(total_simulation_cycles.load(std::memory_order_acquire));
    return process;
}

size_t SJNScheduler::ready_queue_size() const {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    return ready_queue.size();
}

bool SJNScheduler::ready_queue_empty() const {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    return ready_queue.empty();
}

void SJNScheduler::collect_finished_processes() {
    for (auto& core : cores) {
        PCB* process = core->get_current_process();
        if (process == nullptr) {
            core->increment_idle_cycles(1);
            continue;
        }
        
        const bool thread_running = core->is_thread_running();

        // Se a thread do núcleo ainda está rodando, o processo está ocupado. Pular.
        if (thread_running) {
            continue;
        }

        // Se a thread não está rodando, o processo terminou sua fatia de execução.
        core->wait_completion();

        core->with_process_lock([&](PCB*& guarded_process) {
            if (guarded_process == nullptr || guarded_process != process) {
                return;
            }

            switch (process->get_state()) {
                case State::Finished:
                        process->finish_time = cpu_time::now_ns();
                        if (process->finish_sim_time == 0) {
                            process->finish_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
                        }
                    finished_list.push_back(process);
                    finished_count.fetch_add(1);
                    break;
                case State::Blocked:
                    ioManager->registerProcessWaitingForIO(process);
                    blocked_list.push_back(process);
                    break;
                case State::Failed:
                    process->finish_time = cpu_time::now_ns();
                    finished_list.push_back(process);
                    finished_count.fetch_add(1);
                    failed_count.fetch_add(1);
                    break;
                default:
                    enqueue_ready_process(process);
                    break;
            }

            guarded_process = nullptr;
            idle_cores_count.fetch_add(1);
        });
    }
}

void SJNScheduler::schedule_cycle() {
    if (ioManager) {
        ioManager->do_work();
    }
    total_execution_time.fetch_add(1);
    total_simulation_cycles.fetch_add(1);
    uint64_t current_time = total_execution_time.load();
    
    // Coletar processos finalizados antes de atribuir novos
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Desbloqueia processos do IO
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
            if ((*it)->get_state() == State::Ready) {
                enqueue_ready_process(*it);
                it = blocked_list.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Verificação rápida antes de tentar lock (fast-path)
    bool should_schedule = (ready_count.load() > 0 && idle_cores_count.load() > 0);
    
    // Batch scheduling: só trava mutex a cada N ciclos ou quando necessário
    if (current_time % batch_size == 0 || should_schedule) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        
        // Atribui processos aos núcleos livres (menor job primeiro)
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
                        } else if (old_process->get_state() == State::Blocked) {
                            // If the old process was blocked (I/O) while running, ensure it's
                            // registered with IOManager and added to the blocked_list so it
                            // isn't lost (would become an orphan if not tracked).
                            if (ioManager) ioManager->registerProcessWaitingForIO(old_process);
                            if (std::find(blocked_list.begin(), blocked_list.end(), old_process) == blocked_list.end()) {
                                blocked_list.push_back(old_process);
                            }
                            std::cout << "[SJN] P" << old_process->pid << " BLOQUEADO (urgent collect) - esperando I/O\n";
                        }
                        
                        core->clear_current_process();
                        idle_cores_count.fetch_add(1);
                    }
                    
                    if (process->start_time == 0) {
                            process->start_time = cpu_time::now_ns();
                            if (process->start_sim_time == 0) {
                               process->start_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
                            }
                    }
                    process->assigned_core = core->get_id();
                    
                    // SJN é não-preemptivo: quantum infinito
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

bool SJNScheduler::all_finished() const {
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

bool SJNScheduler::has_pending_processes() const {
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

std::vector<std::unique_ptr<Core>>& SJNScheduler::get_cores() {
    return cores;
}

void SJNScheduler::initialize_cores() {
    cores.clear();
    for (int i = 0; i < num_cores; ++i) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores.back()->reset_metrics();
    }
    idle_cores_count.store(num_cores);
}

void SJNScheduler::shutdown_cores() {
    for (auto& core : cores) {
        core->request_stop();
    }
    for (auto& core : cores) {
        core->join_thread();
    }
}

void SJNScheduler::drain_cores() {
    shutdown_cores();
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    int pending = 0;
    for (const auto& core : cores) {
        const bool thread_running = core->is_thread_running();
        core->with_process_lock([&](PCB*& guarded_process) {
            if (guarded_process != nullptr) {
                pending++;
                std::cout << "[SJN][drain] core#" << core->get_id()
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
    std::cout << "[SJN][drain] pending cores=" << pending
              << " newly_collected=" << (finished_after - finished_before)
              << " finished=" << finished_after << "/" << total_count.load()
              << " ready=" << ready_count.load()
              << " blocked=" << blocked_list.size() << "\n";
}

SJNScheduler::Statistics SJNScheduler::get_statistics() const {
    // Delegate to the base class implementation
    auto stats = calculate_statistics(finished_list, cores, start_ns, context_switches);
    if (Log::debug_enabled()) {
        std::cerr << "[SJN] Debug: finished_list.size=" << finished_list.size() << "\n";
        for (const auto* pcb : finished_list) {
            std::cerr << "  PCB pid=" << pcb->pid
                      << " arrival_ns=" << pcb->arrival_time.load()
                      << " finish_ns=" << pcb->finish_time.load()
                      << " arrival_sim=" << pcb->arrival_sim_time.load()
                      << " finish_sim=" << pcb->finish_sim_time.load()
                      << " total_wait_ns=" << pcb->total_wait_time.load()
                      << " total_wait_sim=" << pcb->total_wait_sim_time.load()
                      << " turn_ns=" << pcb->get_turnaround_time()
                      << " turn_sim=" << pcb->get_turnaround_sim_time()
                      << "\n";
        }
    }
    return stats;
}

void SJNScheduler::dump_state(const std::string& reason, uint64_t cycles, uint64_t max_cycles) const {
    namespace fs = std::filesystem;
    std::scoped_lock<std::mutex, std::mutex> lock(scheduler_mutex, ready_queue_mutex);
    fs::create_directories("logs");
    std::ofstream out("logs/scheduler_dumps.log", std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    out << "\n[SJN] reason=" << reason
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
