#include "PriorityScheduler.hpp"
#include <iostream>
#include <chrono>

PriorityScheduler::PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager, int quantum)
    : num_cores(num_cores), quantum(quantum), memManager(memManager), ioManager(ioManager), 
      finished_count(0), total_count(0), context_switches(0), total_execution_time(0), total_simulation_cycles(0) {
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores[i]->reset_metrics();
    }
    
    simulation_start_time = std::chrono::steady_clock::now();
    std::cout << "[PRIORITY] Inicializado em modo PREEMPTIVO (quantum: " << quantum << " ciclos)\n";
}

void PriorityScheduler::add_process(PCB* process) {
    if (process->arrival_time == 0) {
        process->arrival_time = std::chrono::steady_clock::now().time_since_epoch().count();
        total_count++;
    }
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
                process->finish_time = std::chrono::steady_clock::now().time_since_epoch().count();
                finished_list.push_back(process);
                finished_count++;
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
                ready_queue.push_back(process);
                sort_by_priority();
                core->clear_current_process();
                break;
        }
    }
    
    // Desbloqueia processos que terminaram I/O
    for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
        if ((*it)->state == State::Ready) {
            ready_queue.push_back(*it);
            it = blocked_list.erase(it);
        } else {
            ++it;
        }
    }
    
    sort_by_priority();
    check_preemption();
    
    // Atribui processos aos núcleos livres (maior prioridade primeiro)
    for (auto& core : cores) {
        if (core->is_idle() && !ready_queue.empty()) {
            PCB* process = ready_queue.front();
            ready_queue.pop_front();
            
            if (process->start_time == 0) {
                process->start_time = std::chrono::steady_clock::now().time_since_epoch().count();
            }
            process->assigned_core = core->get_id();
            
            process->quantum = quantum;
            
            context_switches++;
            process->context_switches++;
            
            std::cout << "[PRIORITY] Executando P" << process->pid 
                      << " (prioridade: " << process->priority << ") no core " 
                      << core->get_id() << "\n";
            
            core->execute_async(process);
        }
    }
    
    // 🆕 Incrementa tempo de espera A CADA CICLO
    for (auto* process : ready_queue) {
        process->total_wait_time++;
    }
    for (auto* process : blocked_list) {
        process->total_wait_time++;
    }
}

bool PriorityScheduler::all_finished() const {
    if (total_count == 0) return false;
    if (!ready_queue.empty() || !blocked_list.empty()) return false;
    
    // Verificar se todos cores estão ociosos e sem processos pendentes
    for (const auto& core : cores) {
        if (!core->is_idle() || core->get_current_process() != nullptr) {
            return false;
        }
    }
    
    return finished_count >= total_count;
}

bool PriorityScheduler::has_pending_processes() const {
    return !all_finished();
}

int PriorityScheduler::get_finished_count() const {
    return finished_count;
}

std::vector<std::unique_ptr<Core>>& PriorityScheduler::get_cores() { return cores; }
std::deque<PCB*>& PriorityScheduler::get_ready_queue() { return ready_queue; }
std::vector<PCB*>& PriorityScheduler::get_blocked_list() { return blocked_list; }

// ====== FUNÇÕES DE PREEMPÇÃO ======

void PriorityScheduler::check_preemption() {
    if (ready_queue.empty()) return;
    
    // Pega o processo de maior prioridade na fila
    PCB* highest_priority = ready_queue.front();
    
    // Verifica cada core para ver se deve preemptar
    for (auto& core : cores) {
        if (!core->is_idle() && core->is_thread_running()) {
            PCB* running = core->get_current_process();
            if (running && should_preempt(running, highest_priority)) {
                std::cout << "[PRIORITY PREEMPT] P" << running->pid 
                          << " (prioridade " << running->priority 
                          << ") preemptado por P" << highest_priority->pid
                          << " (prioridade " << highest_priority->priority << ")\n";
                
                preempt_process(core.get(), running);
                
                // Remove processo de alta prioridade da fila e coloca no core
                ready_queue.pop_front();
                core->execute_async(highest_priority);
                
                // Reordena já que removemos um processo
                sort_by_priority();
                if (ready_queue.empty()) break;
                highest_priority = ready_queue.front();
            }
        }
    }
}

bool PriorityScheduler::should_preempt(PCB* running, PCB* waiting) {
    // Preempta se processo esperando tem prioridade ESTRITAMENTE MAIOR
    return waiting->priority > running->priority;
}

void PriorityScheduler::preempt_process(Core* core, PCB* process) {
    // Salva contexto do processo (registradores já salvos no PCB pelo Core)
    process->state = State::Ready;  // Marca como pronto para executar novamente
    
    // Aguarda thread do core finalizar execução atual
    core->wait_completion();
    
    // Coloca processo de volta na fila (será reordenado por prioridade)
    ready_queue.push_back(process);
    sort_by_priority();
    
    // Incrementa contador de context switches
    context_switches++;
    process->context_switches++;
}

PriorityScheduler::Statistics PriorityScheduler::get_statistics() const {
    Statistics s;
    
    if (finished_list.empty()) return s;
    
    // Somar métricas de todos os processos finalizados
    uint64_t total_wait = 0;
    uint64_t total_turnaround = 0;
    uint64_t total_response = 0;
    int total_cs = 0;
    
    for (const auto* pcb : finished_list) {
        total_wait += pcb->total_wait_time.load();
        total_turnaround += pcb->get_turnaround_time();
        total_response += (pcb->start_time.load() - pcb->arrival_time.load());
        total_cs += pcb->context_switches.load();
    }
    
    s.total_processes = finished_list.size();
    s.avg_wait_time = total_wait / (double)s.total_processes;
    s.avg_turnaround_time = total_turnaround / (double)s.total_processes;
    s.avg_response_time = total_response / (double)s.total_processes;
    
    // Calcular utilização da CPU (busy cycles / capacidade total)
    uint64_t total_busy = 0;
    for (const auto& core : cores) {
        total_busy += core->get_busy_cycles();
    }
    
    // Tempo real de simulação: máximo de (busy + idle) entre todos os cores
    uint64_t sim_cycles = 0;
    for (const auto& core : cores) {
        uint64_t core_cycles = core->get_busy_cycles() + core->get_idle_cycles();
        if (core_cycles > sim_cycles) {
            sim_cycles = core_cycles;
        }
    }
    
    if (sim_cycles > 0) {
        uint64_t total_capacity = sim_cycles * num_cores;
        s.avg_cpu_utilization = (total_busy * 100.0) / total_capacity;
    } else {
        s.avg_cpu_utilization = 0.0;
    }
    
    // Throughput = processos/segundo = 1 / (tempo médio por processo)
    double avg_turn_cycles = s.avg_turnaround_time;
    double seconds_per_proc = avg_turn_cycles / CLOCK_FREQ_HZ;
    double throughput_proc_per_s = (seconds_per_proc > 0.0) ? (1.0 / seconds_per_proc) : 0.0;
    s.throughput = throughput_proc_per_s;
    s.total_context_switches = context_switches;
    
    return s;
}

