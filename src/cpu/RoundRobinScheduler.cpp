#include "RoundRobinScheduler.hpp"
#include "Core.hpp"
#include <iostream>
#include <algorithm>

#include <unordered_set>

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
<<<<<<< Updated upstream
=======
    total_simulation_cycles.store(0);
    global_context_switches.store(0);
    failed_count.store(0);
    
    // Inicializar contador de cores ociosos
    idle_cores.store(num_cores);
    
    start_ns = cpu_time::now_ns();

    // Inicializa registry
    process_registry.clear();
>>>>>>> Stashed changes
}

RoundRobinScheduler::~RoundRobinScheduler() {
    std::cout << "[Scheduler] Encerrando...\n";
<<<<<<< Updated upstream
    // Aguarda término
=======
    
    // Fase 1: Sinalizar para todas as threads pararem
>>>>>>> Stashed changes
    for (auto& core : cores) {
        core->request_stop();
    }
    
    // Fase 2: Aguardar a conclusão de todas as threads
    for (auto& core : cores) {
        core->join_thread();
    }
}

void RoundRobinScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
<<<<<<< Updated upstream
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
=======
    
    // Setar arrival_time (host) e simulated arrival_time se ainda não foi setado
    if (process->arrival_time == 0) {
        process->arrival_time = cpu_time::now_ns();
    }
    if (process->arrival_sim_time == 0) {
        process->arrival_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
    }
    
    enqueue_ready_process(process);
    total_count.fetch_add(1);
    process->set_state(State::Ready);
    // Registrar PCB
    process_registry[process->pid] = process;
}

void RoundRobinScheduler::schedule_cycle() {
    if (io_manager) {
        io_manager->do_work();
    }
    current_time++;
    total_simulation_cycles++;
    
    // Coletar processos finalizados antes de atribuir novos
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Verificação rápida antes de tentar lock
    bool should_schedule = (ready_count.load() > 0 && idle_cores.load() > 0);
    
    if (Log::debug_enabled() && current_time <= 20 && current_time % 5 == 0) {
        std::cout << "[DEBUG] cycle=" << current_time 
                  << " ready=" << ready_count.load() 
                  << " idle=" << idle_cores.load() 
                  << " should_schedule=" << should_schedule << "\n";
    }
    
    // Batch scheduling: só trava mutex a cada N ciclos ou quando necessário
    if (current_time % batch_size == 0 || should_schedule) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        
        handle_blocked_processes();
        
            if (Log::debug_enabled() && current_time <= 20 && !ready_queue.empty()) {
            std::cout << "[DEBUG] Tentando agendar, ready_queue.size=" << ready_queue.size() << "\n";
            for (size_t i = 0; i < cores.size(); i++) {
                std::cout << "  Core " << i << ": is_idle=" << cores[i]->is_idle() 
                          << " has_process=" << (cores[i]->get_current_process() != nullptr) << "\n";
            }
        }
        
        // Atribuir processos aos cores livres
        size_t max_attempts = ready_queue.size() * 2;
        size_t attempts = 0;
        
        while (!ready_queue.empty() && attempts < max_attempts) {
            bool assigned = false;
            attempts++;
            
            for (auto& core : cores) {
                if (core->is_idle() && !ready_queue.empty()) {
                    // Coletar processo antigo se existir
                    PCB* old_process = core->get_current_process();
                    if (old_process != nullptr) {
                        // Core está IDLE mas ainda tem processo antigo não coletado!
                        std::cout << "[URGENT-COLLECT] Core " << core->get_id() 
                                  << " tem P" << old_process->pid << " não coletado! Coletando agora...\n";
                        
                        // Fazer join se necessário
                        if (core->is_thread_running()) {
                            core->wait_completion();
                        }
                        
                        // Classificar e limpar
                        if (old_process->get_state() == State::Finished) {
                            old_process->finish_time = cpu_time::now_ns();
                            finished_list.push_back(old_process);
                            finished_count.fetch_add(1);
                            std::cout << "[Scheduler] P" << old_process->pid << " FINALIZADO! (urgent collect)\n";
                        } else if (old_process->get_state() == State::Ready) {
                            enqueue_ready_process(old_process);
                            std::cout << "[Scheduler] P" << old_process->pid << " RE-AGENDADO (urgent collect)\n";
                        } else if (old_process->get_state() == State::Blocked) {
                            // Processo bloqueado: registrar no IO manager e em bloqueados
                            blocked_queue.push_back(old_process);
                            if (io_manager) io_manager->registerProcessWaitingForIO(old_process);
                            std::cout << "[Scheduler] P" << old_process->pid << " BLOQUEADO (urgent collect) - esperando I/O\n";
                        }
                        
                        core->clear_current_process();
                        idle_cores.fetch_add(1);
                    }
                    
                    // Atribuir novo processo
                    PCB* process = ready_queue.front();
                    ready_queue.pop_front();
                    ready_count.fetch_sub(1);
                    process->leave_ready_queue_sim(total_simulation_cycles.load(std::memory_order_acquire));
                    assign_process_to_core(process, core.get());
                    assigned = true;
                }
            }
            
            // Se nenhum processo foi atribuído, verificar se todos cores ocupados
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
>>>>>>> Stashed changes
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
    if (process->start_sim_time == 0) {
        process->start_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
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
<<<<<<< Updated upstream
=======
        collected++;
        if (process->get_state() == State::Finished) {
            // Processo REALMENTE terminou (host + simulated timestamps)
            process->finish_time = cpu_time::now_ns();
            if (process->finish_sim_time == 0) {
                process->finish_sim_time = total_simulation_cycles.load(std::memory_order_acquire);
            }
            if (Log::debug_enabled()) {
                std::cout << "[RR DEBUG] P" << process->pid << " start=" << process->start_time.load()
                          << " finish=" << process->finish_time.load() << " turnaround_ns=" << process->get_turnaround_time() << "\n";
            }
            finished_list.push_back(process);
            finished_count.fetch_add(1);
            
            std::cout << "[Scheduler] P" << process->pid << " FINALIZADO!\n";
        } else if (process->get_state() == State::Blocked) {
            // Processo bloqueado (I/O)
            blocked_queue.push_back(process);
            
            std::cout << "[Scheduler] P" << process->pid << " BLOQUEADO (I/O)\n";
        } else if (process->get_state() == State::Ready) {
            // Processo preemptado (quantum expirou)
            enqueue_ready_process(process);
                    } else if (process->get_state() == State::Failed) {
                        // Processo falhou
                        process->finish_time = cpu_time::now_ns();
                        finished_list.push_back(process);
                        failed_count.fetch_add(1);
                        finished_count.fetch_add(1);
                        std::cout << "[Scheduler] P" << process->pid << " FALHOU! (collect)\n";
            
            std::cout << "[Scheduler] P" << process->pid << " RE-AGENDADO (preemptado)\n";
        }
        
        core->clear_current_process();
        idle_cores.fetch_add(1);
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
void RoundRobinScheduler::update_wait_times() {
    // incrementa tempo de espera para processos na ready_queue
    for (PCB* p : ready_queue) {
        p->total_wait_time++;
=======
void RoundRobinScheduler::enqueue_ready_process(PCB* process) {
    const uint64_t sim_ns = total_simulation_cycles.load(std::memory_order_acquire);
    if (!process->enter_ready_queue_sim(sim_ns)) {
        return;
>>>>>>> Stashed changes
    }
}

bool RoundRobinScheduler::has_pending_processes() const {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    return finished_count < total_count;
}

RoundRobinScheduler::Statistics RoundRobinScheduler::get_statistics() const {
<<<<<<< Updated upstream
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
=======
    // Delegate to the base class implementation using the member variable
    return calculate_statistics(finished_list, cores, start_ns, static_cast<int>(global_context_switches.load()));
>>>>>>> Stashed changes
}
