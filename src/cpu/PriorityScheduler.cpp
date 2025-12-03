#include "PriorityScheduler.hpp"
#include <iostream>
#include <chrono>
#include <limits>
#include "TimeUtils.hpp"

PriorityScheduler::PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager), 
      context_switches(0), total_execution_time(0), total_simulation_cycles(0) {
    finished_count.store(0);
    total_count.store(0);
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores[i]->reset_metrics();
    }
    
    simulation_start_time = std::chrono::steady_clock::now();
    std::cout << "[PRIORITY] Inicializado em modo NÃO-PREEMPTIVO (por prioridade)\n";
}

void PriorityScheduler::add_process(PCB* process) {
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
        total_count.fetch_add(1);
    }
    process->enter_ready_queue();
    ready_queue.push_back(process);
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

void PriorityScheduler::schedule_cycle() {
    total_execution_time++;
    total_simulation_cycles++;
    
    // Incrementar ciclos ociosos para cores sem processo
    for (auto& core : cores) {
        if (core->is_idle() && core->get_current_process() == nullptr) {
            core->increment_idle_cycles(1);
        }
    }
    
    // Coletar processos finalizados ou bloqueados antes de agendar novos
    for (auto& core : cores) {
        PCB* process = core->get_current_process();
        if (process == nullptr) continue;
        
        // Coletar se core está IDLE OU thread não está mais rodando
        bool is_idle = core->is_idle();
        bool thread_done = !core->is_thread_running();
        
        if (!is_idle && !thread_done) {
            continue;  // Ainda executando
        }
        
        // Thread terminou OU core está idle - coletar processo
        if (core->is_thread_running()) {
            core->wait_completion();
        }
        
        switch (process->state) {
            case State::Finished:
                process->finish_time = cpu_time::now_ns();
                finished_list.push_back(process);
                finished_count.fetch_add(1);  // Incremento atômico seguro
                std::cout << "[PRIORITY] P" << process->pid 
                          << " finalizado (prioridade: " << process->priority 
                          << ", context switches: " << context_switches << ")\n";
                core->clear_current_process(); // Evita coleta duplicada
                break;
            case State::Blocked:
                ioManager->registerProcessWaitingForIO(process);
                blocked_list.push_back(process);
                core->clear_current_process();
                break;
            default:
                process->enter_ready_queue();
                ready_queue.push_back(process);
                sort_by_priority();
                core->clear_current_process();
                break;
        }
    }
    
    // Desbloqueia processos que terminaram I/O
    for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
        if ((*it)->state == State::Ready) {
            (*it)->enter_ready_queue();
            ready_queue.push_back(*it);
            it = blocked_list.erase(it);
        } else {
            ++it;
        }
    }
    
    sort_by_priority();
    
    // Atribui processos aos núcleos livres (maior prioridade primeiro)
    // NÃO-PREEMPTIVO: processo roda até terminar, sem interrupção
    for (auto& core : cores) {
        if (core->is_available_for_new_process() && !ready_queue.empty()) {
            PCB* process = ready_queue.front();
            ready_queue.pop_front();
            process->leave_ready_queue();
            
            if (process->start_time == 0) {
                process->start_time = cpu_time::now_ns();
            }
            process->assigned_core = core->get_id();
            process->quantum = 999999;  // Não-preemptivo: quantum alto para rodar até terminar
            
            context_switches++;
            process->context_switches++;
            
            std::cout << "[PRIORITY] Executando P" << process->pid 
                      << " (prioridade: " << process->priority << ") no core " 
                      << core->get_id() << "\n";
            
            core->execute_async(process);
        }
    }
    
    // Se todos os cores estão ocupados, aguardar um pouco para evitar busy-wait
    bool all_busy = true;
    for (auto& core : cores) {
        if (core->is_idle() || core->get_current_process() == nullptr) {
            all_busy = false;
            break;
        }
    }
    
    if (all_busy && !ready_queue.empty()) {
        // Cores ocupados mas ainda há processos na fila - aguardar threads
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    } else {
        // Yield para reduzir busy-wait
        std::this_thread::yield();
    }
    
    // Segunda passagem de coleta após yield (para capturar processos que terminaram durante o yield)
    for (auto& core : cores) {
        PCB* process = core->get_current_process();
        if (process == nullptr) continue;
        
        if (core->is_idle() || !core->is_thread_running()) {
            if (core->is_thread_running()) {
                core->wait_completion();
            }
            
            switch (process->state) {
                case State::Finished:
                    process->finish_time = cpu_time::now_ns();
                    finished_list.push_back(process);
                    finished_count.fetch_add(1);
                    core->clear_current_process();
                    break;
                case State::Blocked:
                    ioManager->registerProcessWaitingForIO(process);
                    blocked_list.push_back(process);
                    core->clear_current_process();
                    break;
                default:
                    // Ready ou outro estado - volta para a fila
                    process->enter_ready_queue();
                    ready_queue.push_back(process);
                    sort_by_priority();
                    core->clear_current_process();
                    break;
            }
        }
    }
}

bool PriorityScheduler::all_finished() const {
    int finished = finished_count.load();
    int total = total_count.load();
    
    if (finished >= total && total > 0) {
        // Verificar se há processos ainda em execução nos cores
        for (const auto& core : cores) {
            if (core->get_current_process() != nullptr) {
                return false;  // Ainda há processo no core, aguardar coleta
            }
        }
        // Também verificar se há threads ainda rodando
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
    return !all_finished();
}

int PriorityScheduler::get_finished_count() const {
    return finished_count.load();
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

