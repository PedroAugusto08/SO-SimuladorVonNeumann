#include "PriorityScheduler.hpp"
#include <iostream>
#include <chrono>
#include <limits>
#include "TimeUtils.hpp"
#include <unordered_set>
#include "../util/Log.hpp"

namespace {

const char* to_string_state(State state) {
    switch (state) {
        case State::Ready: return "Ready";
        case State::Running: return "Running";
        case State::Blocked: return "Blocked";
        case State::Finished: return "Finished";
        case State::Failed: return "Failed";
    }
    return "Unknown";
}

}

PriorityScheduler::PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager)
    : num_cores(num_cores), memManager(memManager), ioManager(ioManager) {
    
    std::cout << "[PRIORITY] Inicializando com " << num_cores << " núcleos (modo NÃO-PREEMPTIVO)\n";
    initialize_cores();
    
    // Inicializar contadores atômicos
    total_simulation_cycles.store(0);
    failed_count.store(0);
    ready_count.store(0);
    idle_cores_count.store(num_cores);
    context_switches = 0;
    
    start_ns = cpu_time::now_ns();
    process_registry.clear();
}

PriorityScheduler::~PriorityScheduler() {
    std::cout << "[PRIORITY] Encerrando...\n";
    shutdown_cores();
    
    // Limpar filas
    std::scoped_lock<std::mutex, std::mutex> lock(scheduler_mutex, ready_queue_mutex);
    ready_queue.clear();
    blocked_list.clear();
    finished_list.clear();
}

void PriorityScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
    }
    if (process->arrival_sim_time == 0) {
        process->arrival_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
    }
    
    enqueue_ready_process(process);
    total_count.fetch_add(1);
    process->set_state(State::Ready);
    process_registry[process->pid] = process;
}

void PriorityScheduler::enqueue_ready_process(PCB* process) {
    const uint64_t sim_ns = total_simulation_cycles.load(std::memory_order_acquire);
    if (!process->enter_ready_queue_sim(sim_ns)) {
        return;
    }
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    ready_queue.push_back(process);
    ready_count.fetch_add(1);
    std::stable_sort(ready_queue.begin(), ready_queue.end(),
        [](PCB* a, PCB* b) {
            return a->priority > b->priority;
        });
}

PCB* PriorityScheduler::dequeue_ready_process() {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    if (ready_queue.empty()) {
        return nullptr;
    }
    PCB* process = ready_queue.front();
    ready_queue.pop_front();
    ready_count.fetch_sub(1);
    process->leave_ready_queue_sim(total_simulation_cycles.load(std::memory_order_acquire));
    return process;
}

size_t PriorityScheduler::ready_queue_size() const {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    return ready_queue.size();
}

bool PriorityScheduler::ready_queue_empty() const {
    std::lock_guard<std::mutex> queue_lock(ready_queue_mutex);
    return ready_queue.empty();
}

void PriorityScheduler::collect_finished_processes() {
    for (auto& core : cores) {
        PCB* process = core->get_current_process();
        if (process == nullptr) {
            core->increment_idle_cycles(1);
            continue;
        }
        
        const bool thread_running = core->is_thread_running();

        // Se a thread do núcleo ainda está rodando, o processo está ocupado. Pular.
        if (thread_running) {
            continue;
        }
        
        // Se a thread não está rodando, o processo terminou sua fatia de execução.
        core->wait_completion();
        
        core->with_process_lock([&](PCB*& guarded_process) {
            if (guarded_process == nullptr || guarded_process != process) {
                return;
            }

            const State proc_state = process->get_state();
            switch (process->get_state()) {
                case State::Finished:
                    process->finish_time = cpu_time::now_ns();
                    if (process->finish_sim_time == 0) {
                        process->finish_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
                    }
                    finished_list.push_back(process);
                    finished_count.fetch_add(1);
                    break;
                case State::Blocked:
                    ioManager->registerProcessWaitingForIO(process);
                    blocked_list.push_back(process);
                    break;
                case State::Failed:
                    process->finish_time = cpu_time::now_ns();
                    if (process->finish_sim_time == 0) {
                        process->finish_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
                    }
                    finished_list.push_back(process);
                    finished_count.fetch_add(1);
                    failed_count.fetch_add(1);
                    break;
                default:
                    enqueue_ready_process(process);
                    break;
            }

            guarded_process = nullptr;
            idle_cores_count.fetch_add(1);
        });
    }
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
            if ((*it)->get_state() == State::Ready) {
                enqueue_ready_process(*it);
                it = blocked_list.erase(it);
            } else {
                ++it;
            }
        }
        
        // Atribui processos aos núcleos livres (maior prioridade primeiro)
        const size_t max_attempts = ready_queue_size() * 2;
        size_t attempts = 0;
        
        while (attempts < max_attempts) {
            bool assigned = false;
            attempts++;
            
            for (auto& core : cores) {
                if (!core->is_idle()) {
                    continue;
                }
                PCB* process = dequeue_ready_process();
                if (process == nullptr) {
                    break;
                }
                    // Urgent collect: verificar se há processo antigo não coletado
                    PCB* old_process = core->get_current_process();
                    if (old_process != nullptr) {
                        if (core->is_thread_running()) {
                            core->wait_completion();
                        }
                        
                        if (old_process->get_state() == State::Finished) {
                            old_process->finish_time = cpu_time::now_ns();
                            if (old_process->finish_sim_time == 0) {
                                old_process->finish_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
                            }
                            finished_list.push_back(old_process);
                            finished_count.fetch_add(1);
                        } else if (old_process->get_state() == State::Ready) {
                            enqueue_ready_process(old_process);
                        } else if (old_process->get_state() == State::Blocked) {
                            ioManager->registerProcessWaitingForIO(old_process);
                            blocked_list.push_back(old_process);
                            std::cout << "[PRIORITY] P" << old_process->pid << " BLOQUEADO (urgent collect)" << std::endl;
                        }
                        
                        core->clear_current_process();
                        idle_cores_count.fetch_add(1);
                    }
                    
                    if (process->start_time == 0) {
                        process->start_time = cpu_time::now_ns();
                        if (process->start_sim_time == 0) {
                            process->start_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
                        }
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

            if (ready_queue_empty()) {
                break;
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
    const int total = total_count.load(std::memory_order_acquire);
    if (total == 0) {
        return true;
    }

    const int finished = finished_count.load(std::memory_order_acquire);
    if (finished < total) {
        return false;
    }

    if (!ready_queue_empty()) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        if (!blocked_list.empty()) {
            return false;
        }
    }

    for (const auto& core : cores) {
        if (core->get_current_process() != nullptr || core->is_thread_running()) {
            return false;
        }
    }

    if (ioManager && !ioManager->is_idle()) {
        return false;
    }

    return true;
}

bool PriorityScheduler::has_pending_processes() const {
    const int total = total_count.load(std::memory_order_acquire);
    if (total == 0) {
        return false;
    }

    const int finished = finished_count.load(std::memory_order_acquire);
    if (finished >= total) {
        return false;
    }

    // If there are ready processes, we have pending work
    if (!ready_queue_empty()) {
        return true;
    }

    for (const auto& core : cores) {
        if (core->get_current_process() != nullptr || core->is_thread_running()) {
            return true;
        }
    }

    // If there are blocked processes (waiting for I/O), we also have pending work
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        if (!blocked_list.empty()) {
            return true;
        }
    }

    if (ioManager && !ioManager->is_idle()) {
        return true;
    }

    // Otherwise nothing pending
    return false;
}

std::vector<std::unique_ptr<Core>>& PriorityScheduler::get_cores() { return cores; }

void PriorityScheduler::initialize_cores() {
    cores.clear();
    for (int i = 0; i < num_cores; ++i) {
        cores.push_back(std::make_unique<Core>(i, memManager));
        cores.back()->reset_metrics();
    }
    idle_cores_count.store(num_cores);
}

void PriorityScheduler::shutdown_cores() {
    for (auto& core : cores) {
        core->request_stop();
    }
    for (auto& core : cores) {
        core->join_thread();
    }
}

void PriorityScheduler::drain_cores() {
    shutdown_cores();
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    int pending = 0;
    for (const auto& core : cores) {
        const bool thread_running = core->is_thread_running();
        core->with_process_lock([&](PCB*& guarded_process) {
            if (guarded_process != nullptr) {
                pending++;
                std::cout << "[PRIORITY][drain] core#" << core->get_id()
                          << " segurando P" << guarded_process->pid
                          << " state=" << to_string_state(guarded_process->get_state())
                          << " thread_running=" << (thread_running ? "yes" : "no")
                          << "\n";
            }
        });
    }
    const int finished_before = finished_count.load(std::memory_order_relaxed);
    collect_finished_processes();
    const int finished_after = finished_count.load(std::memory_order_relaxed);
    std::cout << "[PRIORITY][drain] pending cores=" << pending
              << " newly_collected=" << (finished_after - finished_before)
              << " finished=" << finished_after << "/" << total_count.load()
              << " ready=" << ready_count.load()
              << " blocked=" << blocked_list.size() << "\n";
}

PriorityScheduler::Statistics PriorityScheduler::get_statistics() const {
    // Delegate to the base class implementation
    return calculate_statistics(finished_list, cores, start_ns, context_switches);
}
