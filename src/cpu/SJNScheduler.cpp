#include "SJNScheduler.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>

SJNScheduler::SJNScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager),
      finished_count(0), total_count(0), total_execution_time(0) {
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
    }
    simulation_start_time = std::chrono::steady_clock::now().time_since_epoch().count();
}

void SJNScheduler::add_process(PCB* process) {
    if (process->arrival_time == 0) {
        process->arrival_time = std::chrono::steady_clock::now().time_since_epoch().count();
        total_count++;
    }
    // Insere na fila ordenada por estimated_job_size
    auto it = std::find_if(ready_queue.begin(), ready_queue.end(),
        [&](PCB* p) { return process->estimated_job_size < p->estimated_job_size; });
    ready_queue.insert(it, process);
}

void SJNScheduler::schedule_cycle() {
    total_execution_time++;
    
    // Desbloqueia processos do IO
    for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
        if ((*it)->state == State::Ready) {
            add_process(*it);
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
            
            // SJN é não-preemptivo: quantum infinito (processo executa até completar)
            process->quantum = 999999;
            
            core->execute_async(process);
        }
    }
    
    // Incrementa tempo de espera para processos na fila
    for (auto* process : ready_queue) {
        process->total_wait_time++;
    }
    
    // Coleta processos finalizados ou bloqueados
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
                add_process(process);
                core->clear_current_process();
                break;
        }
    }
}

bool SJNScheduler::all_finished() const {
    for (const auto& core : cores) {
        if (!core->is_idle()) return false;
    }
    return ready_queue.empty() && blocked_list.empty();
}

std::vector<std::unique_ptr<Core>>& SJNScheduler::get_cores() {
    return cores;
}

std::deque<PCB*>& SJNScheduler::get_ready_queue() {
    return ready_queue;
}

std::vector<PCB*>& SJNScheduler::get_blocked_list() {
    return blocked_list;
}

SJNScheduler::Statistics SJNScheduler::get_statistics() const {
    Statistics s;
    
    if (finished_list.empty()) return s;
    
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
    
    // Calcula utilização da CPU
    uint64_t total_busy_time = 0;
    for (const auto& core : cores) {
        total_busy_time += total_execution_time.load();
    }
    double total_available_time = total_execution_time.load() * num_cores;
    s.avg_cpu_utilization = (total_available_time > 0) ? 
        (total_busy_time / total_available_time) * 100.0 : 0.0;
    
    // Throughput: processos finalizados por unidade de tempo
    s.throughput = (total_execution_time.load() > 0) ?
        (s.total_processes / (double)total_execution_time.load()) * 1000.0 : 0.0;
    
    s.total_context_switches = 0;  // SJN é não-preemptivo
    
    return s;
}
