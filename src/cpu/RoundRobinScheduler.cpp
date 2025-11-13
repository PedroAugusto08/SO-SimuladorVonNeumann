#include "RoundRobinScheduler.hpp"
#include "Core.hpp"
#include <iostream>
#include <algorithm>

#include "../memory/MemoryManager.hpp"
#include "../IO/IOManager.hpp"

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
    }
}

RoundRobinScheduler::~RoundRobinScheduler() {
    std::cout << "[Scheduler] Encerrando...\n";
    // Aguarda término
    for (auto& core : cores) {
        if (!core->is_idle()) {
            core->wait_completion();
        }
    }
}

void RoundRobinScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    process->arrival_time = current_time;
    if (process->quantum == 0) process->quantum = default_quantum;
    process->state = State::Ready;
    ready_queue.push_back(process);
    total_count++;
    std::cout << "[Scheduler] Processo P" << process->pid
              << " adicionado (quantum=" << process->quantum << ")\n";
}

void RoundRobinScheduler::schedule_cycle() {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    current_time++;
    handle_blocked_processes();
    update_wait_times();

    for (auto& core : cores) {
        if (core->is_idle() && !ready_queue.empty()) {
            PCB* process = ready_queue.front();
            ready_queue.pop_front();
            assign_process_to_core(process, core.get());
        }
    }

    collect_finished_processes();
}

void RoundRobinScheduler::assign_process_to_core(PCB* process, Core* core) {
    std::cout << "[Scheduler] Atribuindo P" << process->pid
              << " ao Core " << core->get_id() << " (quantum=" << process->quantum << ")\n";

    process->assigned_core = core->get_id();

    if (process->last_core != -1 && process->last_core != core->get_id()) {
        process->context_switches++;
    }

    process->last_core = core->get_id();

    if (process->start_time == 0) {
        process->start_time = current_time;
    }

    process->state = State::Running;
    core->execute_async(process);
}

void RoundRobinScheduler::collect_finished_processes() {
    for (auto& core : cores) {
        if (core->is_idle()) continue;
        PCB* process = core->get_current_process();
        if (process == nullptr) continue;

        if (!core->is_thread_running()) {
            core->wait_completion();
            if (process->state == State::Finished) {
                process->finish_time = current_time;
                finished_list.push_back(process);
                finished_count++;
            } else if (process->state == State::Blocked) {
                // mover para fila de bloqueados
                blocked_queue.push_back(process);
            } else if (process->state == State::Ready) {
                // retornou ao final da fila (preempted)
                ready_queue.push_back(process);
            }
        }
    }
}

void RoundRobinScheduler::handle_blocked_processes() {
    // Simples: checar bloqueados e mover para prontos após I/O (placeholder)
    if (blocked_queue.empty()) return;

    // Aqui poderia haver lógica de tempo de I/O; para now movemos tudo de volta
    while (!blocked_queue.empty()) {
        PCB* p = blocked_queue.front();
        blocked_queue.pop_front();
        p->state = State::Ready;
        ready_queue.push_back(p);
    }
}

void RoundRobinScheduler::update_wait_times() {
    // incrementa tempo de espera para processos na ready_queue
    for (PCB* p : ready_queue) {
        p->total_wait_time++;
    }
}

bool RoundRobinScheduler::has_pending_processes() const {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    return finished_count < total_count;
}

RoundRobinScheduler::Statistics RoundRobinScheduler::get_statistics() const {
    Statistics s;
    if (finished_list.empty()) return s;

    double total_wait = 0.0;
    double total_turn = 0.0;
    for (const PCB* p : finished_list) {
        total_wait += (double)p->get_wait_time();
        total_turn += (double)p->get_turnaround_time();
        s.total_context_switches += (int)p->context_switches.load();
    }

    s.avg_wait_time = total_wait / finished_list.size();
    s.avg_turnaround_time = total_turn / finished_list.size();
    s.throughput = (double)finished_list.size() / (current_time ? (double)current_time : 1.0);
    // CPU util: proporção simplificada
    s.avg_cpu_utilization = 1.0 - (double)ready_queue.size() / ((double)num_cores + 1.0);
    return s;
}
