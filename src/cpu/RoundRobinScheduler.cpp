#include "RoundRobinScheduler.hpp"
#include "Core.hpp"
#include <iostream>
#include <algorithm>
#include <limits>

#include "../memory/MemoryManager.hpp"
#include "../IO/IOManager.hpp"
#include "TimeUtils.hpp"
#include <string>

RoundRobinScheduler::RoundRobinScheduler(int num_cores,
                                         MemoryManager* mem_manager,
                                         IOManager* io_manager,
                                         int default_quantum)
    : num_cores(num_cores),
      default_quantum(default_quantum),
      memory_manager(mem_manager),
      io_manager(io_manager)
{
    std::cout << "[Scheduler] Inicializando com " << num_cores
              << " núcleos e quantum=" << default_quantum << "\n";

    for (int i = 0; i < num_cores; ++i) {
        cores.push_back(std::make_unique<Core>(i, memory_manager));
        cores[i]->reset_metrics();
    }
    total_simulation_cycles.store(0);
    global_context_switches.store(0);
    finished_count.store(0);
    total_count.store(0);
    
    // Inicializar contador de cores ociosos
    idle_cores.store(num_cores);
    
    simulation_start = std::chrono::steady_clock::now();
}

int RoundRobinScheduler::get_failed_count() const {
    return std::max(0, get_total_count() - get_finished_count());
}

void RoundRobinScheduler::drain_cores() {
    // Drain by scheduling until there are no pending processes
    int tries = 0;
    const int max_tries = 100000;
    while (has_pending_processes() && tries++ < max_tries) {
        schedule_cycle();
    }
}

void RoundRobinScheduler::dump_state(const std::string &label, int cycles, int cycle_budget) const {
    std::cerr << "[DUMP " << label << "] ready=" << ready_count.load()
              << " finished=" << get_finished_count() << " total=" << get_total_count()
              << " cycles=" << cycles << " budget=" << cycle_budget << "\n";
    for (const auto& core : cores) {
        auto p = core->get_current_process();
        if (p) {
            std::cerr << "  core=" << core->get_id() << " proc=P" << p->pid << " state=" << static_cast<int>(p->state) << "\n";
        } else {
            std::cerr << "  core=" << core->get_id() << " idle\n";
        }
    }
}

RoundRobinScheduler::~RoundRobinScheduler() {
    std::cout << "[Scheduler] Encerrando...\n";
    
    // Aguardar conclusão de todos os cores
    for (auto& core : cores) {
        if (!core->is_idle()) {
            core->wait_completion();
        }
    }
    
    // Limpar filas
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    ready_queue.clear();
    blocked_queue.clear();
    finished_list.clear();
}

void RoundRobinScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    
    // Setar arrival_time se ainda não foi setado
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
    }
    
    enqueue_ready_process(process);
    total_count.fetch_add(1);
    process->state = State::Ready;
}

void RoundRobinScheduler::schedule_cycle() {
    current_time++;
    total_simulation_cycles++;
    
    // Coletar processos finalizados antes de atribuir novos
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Verificação rápida antes de tentar lock
    bool should_schedule = (ready_count.load() > 0 && idle_cores.load() > 0);
    
    if (current_time <= 20 && current_time % 5 == 0) {
        std::cout << "[DEBUG] cycle=" << current_time 
                  << " ready=" << ready_count.load() 
                  << " idle=" << idle_cores.load() 
                  << " should_schedule=" << should_schedule << "\n";
    }
    
    // Batch scheduling: só trava mutex a cada N ciclos ou quando necessário
    if (current_time % batch_size == 0 || should_schedule) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        
        handle_blocked_processes();
        
        if (current_time <= 20 && !ready_queue.empty()) {
            std::cout << "[DEBUG] Tentando agendar, ready_queue.size=" << ready_queue.size() << "\n";
            for (size_t i = 0; i < cores.size(); i++) {
                std::cout << "  Core " << i << ": is_idle=" << cores[i]->is_idle() 
                          << " has_process=" << (cores[i]->get_current_process() != nullptr) << "\n";
            }
        }
        
        // Atribuir processos aos cores livres
        size_t max_attempts = ready_queue.size() * 2;
        size_t attempts = 0;
        
        while (!ready_queue.empty() && attempts < max_attempts) {
            bool assigned = false;
            attempts++;
            
            for (auto& core : cores) {
                if (core->is_idle() && !ready_queue.empty()) {
                    // Coletar processo antigo se existir
                    PCB* old_process = core->get_current_process();
                    if (old_process != nullptr) {
                        // Core está IDLE mas ainda tem processo antigo não coletado!
                        std::cout << "[URGENT-COLLECT] Core " << core->get_id() 
                                  << " tem P" << old_process->pid << " não coletado! Coletando agora...\n";
                        
                        // Fazer join se necessário
                        if (core->is_thread_running()) {
                            core->wait_completion();
                        }
                        
                        // Classificar e limpar
                        if (old_process->state == State::Finished) {
                            old_process->finish_time = cpu_time::now_ns();
                            finished_list.push_back(old_process);
                            finished_count.fetch_add(1);
                            std::cout << "[Scheduler] P" << old_process->pid << " FINALIZADO! (urgent collect)\n";
                        } else if (old_process->state == State::Ready) {
                            enqueue_ready_process(old_process);
                            std::cout << "[Scheduler] P" << old_process->pid << " RE-AGENDADO (urgent collect)\n";
                        }
                        
                        core->clear_current_process();
                        idle_cores.fetch_add(1);
                    }
                    
                    // Atribuir novo processo
                    PCB* process = ready_queue.front();
                    ready_queue.pop_front();
                    ready_count.fetch_sub(1);
                    process->leave_ready_queue();
                    assign_process_to_core(process, core.get());
                    assigned = true;
                }
            }
            
            // Se nenhum processo foi atribuído, verificar se todos cores ocupados
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
        }
    }
    
    std::this_thread::yield();
    
    // Segunda coleta após yield
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Coleta forçada se todos cores idle mas faltam processos
    int idle = idle_cores.load();
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

void RoundRobinScheduler::assign_process_to_core(PCB* process, Core* core) {
    global_context_switches++;
    
    std::cout << "[Scheduler] Atribuindo P" << process->pid
              << " ao Core " << core->get_id() << " (quantum=" << process->quantum << ")\n";

    process->assigned_core = core->get_id();

    if (process->last_core != -1 && process->last_core != core->get_id()) {
        process->context_switches++;
    }

    process->last_core = core->get_id();

    if (process->start_time == 0) {
        process->start_time = cpu_time::now_ns();
    }

    process->state = State::Running;
    idle_cores.fetch_sub(1);
    
    core->execute_async(process);
}

void RoundRobinScheduler::collect_finished_processes() {
    int collected = 0;
    
    if (current_time <= 200 && current_time % 10 == 0) {
        std::cout << "[DEBUG collect] cycle=" << current_time << "\n";
    }
    
    for (auto& core : cores) {
        // Verificar se há processo para coletar
        PCB* process = core->get_current_process();
        
        if (current_time <= 200 && current_time % 10 == 0) {
            std::cout << "  Core " << core->get_id() 
                      << ": process=" << (process ? process->pid : -1)
                      << " thread_running=" << core->is_thread_running()
                      << " is_idle=" << core->is_idle() << "\n";
        }
        
        if (process == nullptr) {
            core->increment_idle_cycles(1);
            continue;
        }
        
        // Coletar se core está idle ou thread terminou
        bool is_idle = core->is_idle();
        bool thread_done = !core->is_thread_running();
        
        if (!is_idle && !thread_done) {
            continue;
        }
        
        std::cout << "[COLLECT] Core " << core->get_id() << " coletando P" << process->pid 
                  << " (state=" << (int)process->state 
                  << " is_idle=" << is_idle 
                  << " thread_done=" << thread_done << ")\n";
        
        if (core->is_thread_running()) {
            core->wait_completion();
        }
        collected++;
        if (process->state == State::Finished) {
            // Processo REALMENTE terminou
            process->finish_time = cpu_time::now_ns();
            finished_list.push_back(process);
            finished_count.fetch_add(1);
            const double tat_ms = cpu_time::ns_to_ms(process->get_turnaround_time());
            const double wait_ms = cpu_time::ns_to_ms(process->get_wait_time());
            std::cout << "[Scheduler] P" << process->pid << " FINALIZADO! turnaround=" << tat_ms << "ms wait=" << wait_ms << "ms\n";
        } else if (process->state == State::Blocked) {
            // Processo bloqueado (I/O)
            blocked_queue.push_back(process);
            
            std::cout << "[Scheduler] P" << process->pid << " BLOQUEADO (I/O)\n";
        } else if (process->state == State::Ready) {
            // Processo preemptado (quantum expirou)
            enqueue_ready_process(process);
            
            std::cout << "[Scheduler] P" << process->pid << " RE-AGENDADO (preemptado)\n";
        }
        
        core->clear_current_process();
        idle_cores.fetch_add(1);
    }
}

void RoundRobinScheduler::handle_blocked_processes() {
    if (blocked_queue.empty()) return;
    while (!blocked_queue.empty()) {
        PCB* p = blocked_queue.front();
        blocked_queue.pop_front();
        p->state = State::Ready;
        enqueue_ready_process(p);
    }
}

void RoundRobinScheduler::enqueue_ready_process(PCB* process) {
    process->enter_ready_queue();
    ready_queue.push_back(process);
    ready_count.fetch_add(1);
}

bool RoundRobinScheduler::has_pending_processes() const {
    int finished = finished_count.load(std::memory_order_acquire);
    int total = total_count.load(std::memory_order_acquire);
    
    if (finished >= total && total > 0) {
        return false;
    }
    
    int idle = idle_cores.load(std::memory_order_acquire);
    if (idle >= num_cores && finished < total) {
        std::this_thread::yield();
        finished = finished_count.load(std::memory_order_acquire);
        if (finished >= total) {
            return false;
        }
    }
    
    return finished < total;
}

RoundRobinScheduler::Statistics RoundRobinScheduler::get_statistics() const {
    Statistics s;
    if (finished_list.empty()) return s;

    uint64_t total_wait_ns = 0;
    uint64_t total_turnaround_ns = 0;
    uint64_t total_response_ns = 0;
    uint64_t earliest_arrival = std::numeric_limits<uint64_t>::max();
    uint64_t latest_finish = 0;
    uint64_t total_pipeline_cycles = 0;
    
    for (const PCB* p : finished_list) {
        total_wait_ns += p->total_wait_time.load();
        const uint64_t tat = p->get_turnaround_time();
        total_turnaround_ns += tat;
        const uint64_t start_time = p->start_time.load();
        const uint64_t arrival = p->arrival_time.load();
        if (start_time > 0 && start_time >= arrival) {
            total_response_ns += (start_time - arrival);
        }
        earliest_arrival = std::min(earliest_arrival, arrival);
        latest_finish = std::max(latest_finish, p->finish_time.load());
        total_pipeline_cycles += p->pipeline_cycles.load();
    }

    if (earliest_arrival == std::numeric_limits<uint64_t>::max()) {
        earliest_arrival = latest_finish;
    }

    s.total_processes = finished_list.size();
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
    
    s.total_context_switches = static_cast<int>(global_context_switches.load());
    
    return s;
}
