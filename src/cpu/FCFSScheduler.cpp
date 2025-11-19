#include "FCFSScheduler.hpp"
#include <iostream>

FCFSScheduler::FCFSScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager) {
    for (int i = 0; i < num_cores; i++) {
        cores.push_back(std::make_unique<Core>(i, memManager));
    }
}

void FCFSScheduler::add_process(PCB* process) {
    ready_queue.push_back(process);
}

void FCFSScheduler::schedule_cycle() {
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
            core->execute_async(process);
        }
    }
    // Coleta processos finalizados ou bloqueados
    for (auto& core : cores) {
        if (!core->is_idle() && !core->is_thread_running()) {
            PCB* process = core->get_current_process();
            if (process) {
                core->wait_completion();
                switch (process->state) {
                    case State::Finished:
                        break;
                    case State::Blocked:
                        ioManager->registerProcessWaitingForIO(process);
                        blocked_list.push_back(process);
                        break;
                    default:
                        ready_queue.push_back(process);
                        break;
                }
            }
        }
    }
}

bool FCFSScheduler::all_finished() const {
    for (const auto& core : cores) {
        if (!core->is_idle()) return false;
    }
    return ready_queue.empty() && blocked_list.empty();
}

std::vector<std::unique_ptr<Core>>& FCFSScheduler::get_cores() { return cores; }
std::deque<PCB*>& FCFSScheduler::get_ready_queue() { return ready_queue; }
std::vector<PCB*>& FCFSScheduler::get_blocked_list() { return blocked_list; }
