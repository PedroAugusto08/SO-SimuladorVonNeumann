#include "FCFSScheduler.hpp"
#include <iostream>
#include <chrono>

FCFSScheduler::FCFSScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager),
      finished_count(0), total_count(0), total_execution_time(0) {
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores[i]->reset_metrics();
    }
    total_simulation_cycles.store(0);
    context_switches = 0;
    
    simulation_start_time = std::chrono::steady_clock::now();
}

void FCFSScheduler::add_process(PCB* process) {
    if (process->arrival_time == 0) {
        process->arrival_time = std::chrono::steady_clock::now().time_since_epoch().count();
    }
    total_count++;
    ready_queue.push_back(process);
}

void FCFSScheduler::schedule_cycle() {
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
                core->clear_current_process(); // Evita coleta duplicada
                break;
            case State::Blocked:
                ioManager->registerProcessWaitingForIO(process);
                blocked_list.push_back(process);
                core->clear_current_process();
                break;
            default:
                ready_queue.push_back(process);
                core->clear_current_process();
                break;
        }
    }
    
    // Desbloqueia processos do IO
    for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
        if ((*it)->state == State::Ready) {
            ready_queue.push_back(*it);
            it = blocked_list.erase(it);
        } else {
            ++it;
        }
    }
    
    // Atribui processos aos núcleos livres
    for (auto& core : cores) {
        if (core->is_idle() && !ready_queue.empty()) {
            PCB* process = ready_queue.front();
            ready_queue.pop_front();
            
            if (process->start_time == 0) {
                process->start_time = std::chrono::steady_clock::now().time_since_epoch().count();
            }
            process->assigned_core = core->get_id();
            
            // FCFS é não-preemptivo: quantum infinito (processo executa até completar)
            process->quantum = 999999;
            
            // Context switch: processo sendo despachado para core
            context_switches++;
            process->context_switches++;
            
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

bool FCFSScheduler::all_finished() const {
    int finished = finished_count.load();
    int total = total_count.load();
    
    if (total == 0) return false;
    if (!ready_queue.empty() || !blocked_list.empty()) return false;
    
    // Verificar se todos cores estão ociosos e sem processos pendentes
    for (const auto& core : cores) {
        if (!core->is_idle() || core->get_current_process() != nullptr) {
            return false;
        }
    }
    
    return finished >= total;
}

std::vector<std::unique_ptr<Core>>& FCFSScheduler::get_cores() { return cores; }
std::deque<PCB*>& FCFSScheduler::get_ready_queue() { return ready_queue; }
std::vector<PCB*>& FCFSScheduler::get_blocked_list() { return blocked_list; }

FCFSScheduler::Statistics FCFSScheduler::get_statistics() const {
    Statistics s;
    
    if (finished_list.empty()) return s;
    
    // Somar métricas de todos os processos finalizados
    uint64_t total_wait = 0;
    uint64_t total_turnaround = 0;
    uint64_t total_response = 0;
    
    for (const auto* pcb : finished_list) {
        total_wait += pcb->total_wait_time.load();
        total_turnaround += pcb->get_turnaround_time();
        total_response += (pcb->start_time.load() - pcb->arrival_time.load());
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
