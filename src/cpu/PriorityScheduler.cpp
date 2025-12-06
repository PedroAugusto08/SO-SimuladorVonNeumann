#include "PriorityScheduler.hpp"
#include <iostream>
#include <chrono>
#include <limits>
#include "TimeUtils.hpp"

PriorityScheduler::PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager) {
    
    std::cout << "[PRIORITY] Inicializando com " << num_cores << " núcleos (modo NÃO-PREEMPTIVO)\n";
    
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores[i]->reset_metrics();
    }
    
    // Inicializar contadores atômicos
    total_simulation_cycles.store(0);
    finished_count.store(0);
    total_count.store(0);
    total_execution_time.store(0);
    ready_count.store(0);
    idle_cores_count.store(num_cores);
    context_switches = 0;
    
    simulation_start_time = std::chrono::steady_clock::now();
}

PriorityScheduler::~PriorityScheduler() {
    std::cout << "[PRIORITY] Encerrando...\n";
    
    // Aguardar conclusão de todos os cores
    for (auto& core : cores) {
        if (!core->is_idle()) {
            core->wait_completion();
        }
    }
    
    // Limpar filas
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    ready_queue.clear();
    blocked_list.clear();
    finished_list.clear();
}

void PriorityScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
    }
    
    enqueue_ready_process(process);
    total_count.fetch_add(1);
    process->state = State::Ready;
}

void PriorityScheduler::enqueue_ready_process(PCB* process) {
    process->enter_ready_queue();
    ready_queue.push_back(process);
    ready_count.fetch_add(1);
    sort_by_priority();
}

void PriorityScheduler::sort_by_priority() {
    // Ordena por prioridade DECRESCENTE (maior prioridade primeiro)
    // Em caso de empate, mantém ordem de chegada (FCFS como tiebreaker)
    std::stable_sort(ready_queue.begin(), ready_queue.end(),
        [](PCB* a, PCB* b) {
            return a->priority > b->priority;
        });
}

void PriorityScheduler::collect_finished_processes() {
    for (auto& core : cores) {
        PCB* process = core->get_current_process();
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
        
        if (core->is_thread_running()) {
            core->wait_completion();
        }
        
        switch (process->state) {
            case State::Finished:
                process->finish_time = cpu_time::now_ns();
                finished_list.push_back(process);
                finished_count.fetch_add(1);
                if (process->failed.load()) {
                    failed_count.fetch_add(1);
                }
                std::cout << "[PRIORITY] P" << process->pid 
                          << " FINALIZADO (prioridade: " << process->priority << ")\n";
                break;
            case State::Blocked:
                ioManager->registerProcessWaitingForIO(process);
                blocked_list.push_back(process);
                std::cout << "[PRIORITY] P" << process->pid << " BLOQUEADO (I/O)\n";
                break;
            default:
                enqueue_ready_process(process);
                std::cout << "[PRIORITY] P" << process->pid << " RE-AGENDADO\n";
                break;
        }
        
        core->clear_current_process();
        idle_cores_count.fetch_add(1);
    }
}

void PriorityScheduler::drain_cores() {
    int safety = 0;
    while (has_pending_processes() && safety < 1000000) {
        schedule_cycle();
        ++safety;
    }
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    collect_finished_processes();
}

void PriorityScheduler::dump_state(const std::string &tag, int cycles, int cycle_budget) {
    std::cerr << "[PRIORITY DUMP] " << tag << " cycles=" << cycles << " budget=" << cycle_budget << "\n";
}

void PriorityScheduler::schedule_cycle() {
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
        
        // Desbloqueia processos que terminaram I/O
        for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
            if ((*it)->state == State::Ready) {
                enqueue_ready_process(*it);
                it = blocked_list.erase(it);
            } else {
                ++it;
            }
        }
        
        // Atribui processos aos núcleos livres (maior prioridade primeiro)
        size_t max_attempts = ready_queue.size() * 2;
        size_t attempts = 0;
        
        while (!ready_queue.empty() && attempts < max_attempts) {
            bool assigned = false;
            attempts++;
            
            for (auto& core : cores) {
                if (core->is_idle() && !ready_queue.empty()) {
                    // Urgent collect: verificar se há processo antigo não coletado
                    PCB* old_process = core->get_current_process();
                    if (old_process != nullptr) {
                        if (core->is_thread_running()) {
                            core->wait_completion();
                        }
                        
                        if (old_process->state == State::Finished) {
                            old_process->finish_time = cpu_time::now_ns();
                            finished_list.push_back(old_process);
                            finished_count.fetch_add(1);
                        } else if (old_process->state == State::Ready) {
                            enqueue_ready_process(old_process);
                        }
                        
                        core->clear_current_process();
                        idle_cores_count.fetch_add(1);
                    }
                    
                    // Atribuir novo processo (maior prioridade primeiro - já ordenado)
                    PCB* process = ready_queue.front();
                    ready_queue.pop_front();
                    ready_count.fetch_sub(1);
                    process->leave_ready_queue();
                    
                    if (process->start_time == 0) {
                        process->start_time = cpu_time::now_ns();
                    }
                    process->assigned_core = core->get_id();
                    
                    // Priority é não-preemptivo: quantum infinito
                    process->quantum = 999999;
                    
                    context_switches++;
                    process->context_switches++;
                    
                    std::cout << "[PRIORITY] Executando P" << process->pid 
                              << " (prioridade: " << process->priority << ") no core " 
                              << core->get_id() << "\n";
                    
                    idle_cores_count.fetch_sub(1);
                    core->execute_async(process);
                    assigned = true;
                }
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

bool PriorityScheduler::all_finished() const {
    int finished = finished_count.load(std::memory_order_acquire);
    int total = total_count.load(std::memory_order_acquire);
    
    if (finished >= total && total > 0) {
        // Verificar se há processos ainda em execução nos cores
        for (const auto& core : cores) {
            if (core->get_current_process() != nullptr) {
                return false;
            }
        }
        for (const auto& core : cores) {
            if (core->is_thread_running()) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool PriorityScheduler::has_pending_processes() const {
    int finished = finished_count.load(std::memory_order_acquire);
    int total = total_count.load(std::memory_order_acquire);
    
    if (finished >= total && total > 0) {
        return false;
    }
    
    int idle = idle_cores_count.load(std::memory_order_acquire);
    if (idle >= num_cores && finished < total) {
        std::this_thread::yield();
        finished = finished_count.load(std::memory_order_acquire);
        if (finished >= total) {
            return false;
        }
    }
    
    return finished < total;
}

std::vector<std::unique_ptr<Core>>& PriorityScheduler::get_cores() { return cores; }
std::deque<PCB*>& PriorityScheduler::get_ready_queue() { return ready_queue; }
std::vector<PCB*>& PriorityScheduler::get_blocked_list() { return blocked_list; }

PriorityScheduler::Statistics PriorityScheduler::get_statistics() const {
    Statistics s;
    
    if (finished_list.empty()) return s;
    
    uint64_t total_wait_ns = 0;
    uint64_t total_turnaround_ns = 0;
    uint64_t total_response_ns = 0;
    uint64_t earliest_arrival = std::numeric_limits<uint64_t>::max();
    uint64_t latest_finish = 0;
    uint64_t total_pipeline_cycles = 0;
    int total_cs = 0;

    for (const auto* pcb : finished_list) {
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
        total_cs += pcb->context_switches.load();
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

    s.total_context_switches = context_switches;
    
    return s;
}

