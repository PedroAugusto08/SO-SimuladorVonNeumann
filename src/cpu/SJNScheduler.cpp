#include "SJNScheduler.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <limits>
#include "TimeUtils.hpp"

SJNScheduler::SJNScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
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

void SJNScheduler::add_process(PCB* process) {
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
        total_count++;
    }
    // Insere na fila ordenada por estimated_job_size
    process->enter_ready_queue();
    auto it = std::find_if(ready_queue.begin(), ready_queue.end(),
        [&](PCB* p) { return process->estimated_job_size < p->estimated_job_size; });
    ready_queue.insert(it, process);
}

void SJNScheduler::schedule_cycle() {
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
            process->leave_ready_queue();
            
            if (process->start_time == 0) {
                process->start_time = cpu_time::now_ns();
            }
            process->assigned_core = core->get_id();
            
            // SJN é não-preemptivo: quantum infinito (processo executa até completar)
            process->quantum = 999999;
            
            // Context switch: processo sendo despachado para core
            context_switches++;
            process->context_switches++;
            
            core->execute_async(process);
        }
    }
    
}

bool SJNScheduler::all_finished() const {
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
    
    uint64_t total_wait_ns = 0;
    uint64_t total_turnaround_ns = 0;
    uint64_t total_response_ns = 0;
    uint64_t earliest_arrival = std::numeric_limits<uint64_t>::max();
    uint64_t latest_finish = 0;
    uint64_t total_pipeline_cycles = 0;
    
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
