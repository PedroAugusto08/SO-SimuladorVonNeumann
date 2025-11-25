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
    
    // Inicializar contador de cores ociosos
    idle_cores.store(num_cores);
}

RoundRobinScheduler::~RoundRobinScheduler() {
    std::cout << "[Scheduler] Encerrando...\n";
    
    // 1. Aguarda término de todos os cores PRIMEIRO
    for (auto& core : cores) {
        if (!core->is_idle()) {
            core->wait_completion();
        }
    }
    
    // 2. CRITICAL: Limpar todas as filas de ponteiros  IMEDIATAMENTE
    // Fazemos isso ANTES de qualquer outra operação
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        ready_queue.clear();
        blocked_queue.clear();
        finished_list.clear();
    }
    
    std::cout << "[Scheduler] Filas limpas, finalizando...\n";
}

void RoundRobinScheduler::add_process(PCB* process) {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    
    // Setar arrival_time se ainda não foi setado
    if (process->arrival_time == 0) {
        process->arrival_time = std::chrono::steady_clock::now().time_since_epoch().count();
    }
    
    ready_queue.push_back(process);
    ready_count.fetch_add(1);
    total_count.fetch_add(1);
    process->state = State::Ready;
}

void RoundRobinScheduler::schedule_cycle() {
    current_time++;
    
    // **CRITICAL: COLETAR ANTES DE ATRIBUIR!**
    // Se core terminou processo e ficou IDLE, coletar ANTES de atribuir novo
    // Caso contrário, processo antigo é perdido quando novo é atribuído
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // OTIMIZAÇÃO: Verificação rápida lock-free antes de tentar lock
    bool should_schedule = (ready_count.load() > 0 && idle_cores.load() > 0);
    
    if (current_time <= 20 && current_time % 5 == 0) {
        std::cout << "[DEBUG] cycle=" << current_time 
                  << " ready=" << ready_count.load() 
                  << " idle=" << idle_cores.load() 
                  << " should_schedule=" << should_schedule << "\n";
    }
    
    // OTIMIZAÇÃO: Batch scheduling - só trava mutex a cada N ciclos OU quando necessário
    // Reduz locks de 10.000 para ~1.000 (10x menos contenção)
    if (current_time % batch_size == 0 || should_schedule) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        
        handle_blocked_processes();
        
        if (current_time <= 20 && !ready_queue.empty()) {
            std::cout << "[DEBUG] Tentando agendar, ready_queue.size=" << ready_queue.size() << "\n";
            for (size_t i = 0; i < cores.size(); i++) {
                std::cout << "  Core " << i << ": is_idle=" << cores[i]->is_idle() 
                          << " has_process=" << (cores[i]->get_current_process() != nullptr) << "\n";
            }
        }
        
        // Atribuir TODOS os processos possíveis de uma vez (maximiza paralelismo)
        // CRITICAL FIX: Continuar tentando até esvaziar ready_queue ou todos cores ocupados
        size_t max_attempts = ready_queue.size() * 2;  // Evita loop infinito
        size_t attempts = 0;
        
        while (!ready_queue.empty() && attempts < max_attempts) {
            bool assigned = false;
            attempts++;
            
            for (auto& core : cores) {
                if (core->is_idle() && !ready_queue.empty()) {
                    // CRITICAL: Coletar processo antigo ANTES de atribuir novo!
                    // Se core tem processo, coletar primeiro
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
                        if (old_process->state == State::Finished) {
                            old_process->finish_time = std::chrono::steady_clock::now().time_since_epoch().count();
                            finished_list.push_back(old_process);
                            finished_count.fetch_add(1);
                            std::cout << "[Scheduler] P" << old_process->pid << " FINALIZADO! (urgent collect)\n";
                        } else if (old_process->state == State::Ready) {
                            ready_queue.push_back(old_process);
                            ready_count.fetch_add(1);
                            std::cout << "[Scheduler] P" << old_process->pid << " RE-AGENDADO (urgent collect)\n";
                        }
                        
                        core->clear_current_process();
                        
                        // CRITICAL: Incrementar idle_cores após coletar!
                        // Antes estava apenas no collect_finished_processes()
                        idle_cores.fetch_add(1);
                    }
                    
                    // Agora sim, atribuir novo processo
                    PCB* process = ready_queue.front();
                    ready_queue.pop_front();
                    ready_count.fetch_sub(1);
                    assign_process_to_core(process, core.get());
                    assigned = true;
                }
            }
            
            // Se nenhum processo foi atribuído E todos cores estão ocupados, parar
            if (!assigned) {
                // Verificar se realmente todos cores estão ocupados
                bool all_busy = true;
                for (auto& core : cores) {
                    if (core->is_idle()) {
                        all_busy = false;
                        break;
                    }
                }
                
                if (all_busy) break;  // Todos cores ocupados, parar
                // Caso contrário, tentar novamente (race condition)
            }
        }
    }
    
    // Atualizar tempos de espera (lock-free, apenas incrementa contadores)
    if (current_time % batch_size == 0) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        update_wait_times();
    }
    
    // CRITICAL: Dar oportunidade para as threads executarem
    // std::this_thread::yield() permite que threads assíncronas rodem
    std::this_thread::yield();
    
    // CRITICAL: Segunda coleta após yield para pegar processos que terminaram agora
    // (resolve race condition onde thread termina mas não foi detectado na primeira coleta)
    {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // CRITICAL: Se todos cores IDLE mas ainda faltam processos, fazer coleta forçada
    // Isso resolve race conditions onde threads terminaram mas is_idle() ainda não atualizou
    int idle = idle_cores.load();
    int finished = finished_count.load();
    int total = total_count.load();
    
    if (idle >= num_cores && finished < total) {
        // Todos cores idle mas faltam processos - race condition!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // Tentar coletar novamente
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        collect_finished_processes();
    }
    
    // Extra: pequeno sleep a cada N ciclos para reduzir busy-wait
    if (current_time % 100 == 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
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
        process->start_time = std::chrono::steady_clock::now().time_since_epoch().count();
    }

    process->state = State::Running;
    
    // CRITICAL: Decrementar idle_cores quando atribui processo
    idle_cores.fetch_sub(1);
    
    core->execute_async(process);
}

void RoundRobinScheduler::collect_finished_processes() {
    int collected = 0;
    
    if (current_time <= 200 && current_time % 10 == 0) {
        std::cout << "[DEBUG collect] cycle=" << current_time << "\n";
    }
    
    for (auto& core : cores) {
        // Verificar se há processo para coletar
        PCB* process = core->get_current_process();
        
        if (current_time <= 200 && current_time % 10 == 0) {
            std::cout << "  Core " << core->get_id() 
                      << ": process=" << (process ? process->pid : -1)
                      << " thread_running=" << core->is_thread_running()
                      << " is_idle=" << core->is_idle() << "\n";
        }
        
        // Se não há processo, pular
        if (process == nullptr) {
            continue;
        }
        
        // CRITICAL: Coletar se core está IDLE **OU** thread não está mais rodando
        // Isso pega ambos os casos:
        // 1. Core marcou IDLE (caso normal)
        // 2. Thread terminou mas is_idle() ainda não atualizou (race condition)
        bool is_idle = core->is_idle();
        bool thread_done = !core->is_thread_running();
        
        // Se core ainda está executando E thread ainda está rodando, pular
        if (!is_idle && !thread_done) {
            continue;  // Ainda executando
        }
        
        // Thread terminou OU core está idle - coletar!
        std::cout << "[COLLECT] Core " << core->get_id() << " coletando P" << process->pid 
                  << " (state=" << (int)process->state 
                  << " is_idle=" << is_idle 
                  << " thread_done=" << thread_done << ")\n";
        
        // Fazer wait_completion() se thread ainda é joinable
        if (core->is_thread_running()) {
            core->wait_completion();
        }
        collected++;
        
        // Classificar processo baseado no estado final
        if (process->state == State::Finished) {
            // Processo REALMENTE terminou
            process->finish_time = std::chrono::steady_clock::now().time_since_epoch().count();
            finished_list.push_back(process);
            finished_count.fetch_add(1);
            
            std::cout << "[Scheduler] P" << process->pid << " FINALIZADO!\n";
        } else if (process->state == State::Blocked) {
            // Processo bloqueado (I/O)
            blocked_queue.push_back(process);
            
            std::cout << "[Scheduler] P" << process->pid << " BLOQUEADO (I/O)\n";
        } else if (process->state == State::Ready) {
            // Processo preemptado (quantum expirou)
            ready_queue.push_back(process);
            ready_count.fetch_add(1);
            
            std::cout << "[Scheduler] P" << process->pid << " RE-AGENDADO (preemptado)\n";
        }
        
        // Limpar o core após coletar o processo
        core->clear_current_process();
        
        // CRITICAL: Incrementar idle_cores quando core fica livre
        // (independente do estado do processo)
        idle_cores.fetch_add(1);
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
    // OTIMIZAÇÃO: Verificação lock-free usando atomics
    // Evita deadlock não travando mutex em método const
    
    int finished = finished_count.load(std::memory_order_acquire);
    int total = total_count.load(std::memory_order_acquire);
    int idle = idle_cores.load(std::memory_order_acquire);
    
    // Verificação rápida: se todos cores IDLE e todos processos finalizados
    if (idle >= num_cores && finished >= total) {
        return false;  // TERMINOU!
    }
    
    // CRITICAL: Se todos cores IDLE mas ainda faltam processos
    // Há race condition - processos finalizaram mas não foram coletados
    // Como agora fazemos urgent-collect, isso não deveria acontecer
    // Mas dar uma chance com delay mínimo
    if (idle >= num_cores && finished < total) {
        // Delay muito curto - urgent collect já deve ter resolvido
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // Re-verificar
        finished = finished_count.load(std::memory_order_acquire);
        
        if (finished >= total) {
            return false;  // Agora terminou!
        }
    }
    
    // Ainda há trabalho pendente
    return finished < total || idle < num_cores;
}

RoundRobinScheduler::Statistics RoundRobinScheduler::get_statistics() const {
    Statistics s;
    if (finished_list.empty()) return s;

    uint64_t total_wait = 0;
    uint64_t total_turnaround = 0;
    uint64_t total_response = 0;
    
    for (const PCB* p : finished_list) {
        total_wait += p->total_wait_time.load();
        total_turnaround += p->get_turnaround_time();
        total_response += (p->start_time.load() - p->arrival_time.load());
        s.total_context_switches += (int)p->context_switches.load();
    }

    s.total_processes = finished_list.size();
    s.avg_wait_time = total_wait / (double)s.total_processes;
    s.avg_turnaround_time = total_turnaround / (double)s.total_processes;
    s.avg_response_time = total_response / (double)s.total_processes;
    
    // Utilização da CPU (estimativa baseada em processos finalizados vs tempo total)
    uint64_t total_busy_time = 0;
    for (const auto& core : cores) {
        total_busy_time += current_time;
    }
    double total_available_time = current_time * num_cores;
    s.avg_cpu_utilization = (total_available_time > 0) ? 
        (total_busy_time / total_available_time) * 100.0 : 0.0;
    
    // Throughput: processos por milissegundo
    s.throughput = (current_time > 0) ?
        (s.total_processes / (double)current_time) * 1000.0 : 0.0;
    
    return s;
}
