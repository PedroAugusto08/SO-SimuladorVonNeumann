#include "PriorityScheduler.hpp"
#include <iostream>
#include <chrono>

PriorityScheduler::PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager, int quantum)
    : num_cores(num_cores), quantum(quantum), memManager(memManager), ioManager(ioManager), 
      finished_count(0), total_count(0), context_switches(0), total_execution_time(0) {
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
    }
    simulation_start_time = std::chrono::steady_clock::now().time_since_epoch().count();
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
    
    // 1. Desbloqueia processos do IO
    for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
        if ((*it)->state == State::Ready) {
            ready_queue.push_back(*it);
            it = blocked_list.erase(it);
        } else {
            ++it;
        }
    }
    
    // 2. Reordena fila por prioridade
    sort_by_priority();
    
    // 3. PREEMPÇÃO POR PRIORIDADE: Verifica se processos rodando devem ser preemptados
    check_preemption();
    
    // 4. Atribui processos aos núcleos livres (maior prioridade primeiro)
    for (auto& core : cores) {
        if (core->is_idle() && !ready_queue.empty()) {
            PCB* process = ready_queue.front();
            ready_queue.pop_front();
            
            if (process->start_time == 0) {
                process->start_time = std::chrono::steady_clock::now().time_since_epoch().count();
            }
            process->assigned_core = core->get_id();
            
            // Define quantum do processo (usado pelo Core)
            process->quantum = quantum;
            
            std::cout << "[PRIORITY] Executando P" << process->pid 
                      << " (prioridade: " << process->priority << ") no core " 
                      << core->get_id() << "\n";
            
            core->execute_async(process);
        }
    }
    
    // Incrementa tempo de espera para processos na fila
    for (auto* process : ready_queue) {
        process->total_wait_time++;
    }
    
    // 5. Coleta processos finalizados ou bloqueados
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
}

bool PriorityScheduler::all_finished() const {
    for (const auto& core : cores) {
        if (!core->is_idle()) return false;
    }
    return ready_queue.empty() && blocked_list.empty();
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
    
    // Calcula utilização da CPU
    uint64_t total_busy_time = 0;
    for (size_t i = 0; i < cores.size(); ++i) {
        total_busy_time += total_execution_time;
    }
    double total_available_time = total_execution_time * num_cores;
    s.avg_cpu_utilization = (total_available_time > 0) ? 
        (total_busy_time / total_available_time) * 100.0 : 0.0;
    
    // Throughput: processos finalizados por unidade de tempo
    s.throughput = (total_execution_time > 0) ?
        (s.total_processes / (double)total_execution_time) * 1000.0 : 0.0;
    
    s.total_context_switches = context_switches;  // Priority preemptivo tem context switches
    
    return s;
}

