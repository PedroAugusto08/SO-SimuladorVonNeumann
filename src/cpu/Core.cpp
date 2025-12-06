#include "Core.hpp"
#include "CONTROL_UNIT.hpp"
#include "../memory/MemoryManager.hpp"
#include "PCB.hpp"
#include "../IO/IOManager.hpp"
#include "TimeUtils.hpp"
#include <iostream>
#include <chrono>

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

Core::Core(int id, MemoryManager* mem_manager) 
    : core_id(id), memory_manager(mem_manager) 
{
    // Criar cache L1 privada (usa construtor padrão)
    // Cada núcleo tem sua própria cache para evitar contenção
    L1_cache = std::make_unique<Cache>();
    
    std::cout << "[Core " << core_id << "] Inicializado com cache L1 privada\n";
}

Core::~Core() {
    state.store(CoreState::STOPPING);
    
    // Aguarda término da thread se ainda estiver executando
    if (execution_thread.joinable()) {
        execution_thread.join();
    }
    
    std::cout << "[Core " << core_id << "] Finalizado\n";
}

void Core::execute_async(PCB* process) {
    std::lock_guard<std::mutex> lock(core_mutex);
    
    if (state.load() != CoreState::IDLE) {
        throw std::runtime_error("Core::execute_async - Núcleo não está idle!");
    }
    
    if (!process) {
        throw std::invalid_argument("Core::execute_async - Processo nulo!");
    }
    
    // Atribui processo ao núcleo
    current_process = process;
    process->assigned_core = core_id;
    process->set_state(State::Running);
    
    // Registra tempo de início se for a primeira execução
    if (process->start_time == 0) {
        process->start_time = cpu_time::now_ns();
    }
    
    // Marca núcleo como ocupado
    state.store(CoreState::BUSY);
    stop_requested.store(false, std::memory_order_release);
    
    std::cout << "[Core " << core_id << "] Iniciando execução do processo P" 
              << process->pid << " (quantum=" << process->quantum << ")\n";
    
    // CRÍTICO: Se há thread anterior, fazer join antes de criar nova
    if (execution_thread.joinable()) {
        execution_thread.join();
    }
    
    // Inicia thread de execução
    execution_thread = std::thread(&Core::run_process, this, process);
}

void Core::wait_completion() {
    join_thread();
}

void Core::run_process(PCB* process) {
    // 🔥 CRÍTICO: Registrar cache L1 privada desta thread
    if (L1_cache) {
        L1_cache->flush();
        MemoryManager::setThreadCache(L1_cache.get());
    } else {
        MemoryManager::setThreadCache(nullptr);
    }
    
    // Estruturas de controle
    Control_Unit control_unit;
    int counter = 0;
    int counterForEnd = 0;
    bool endProgram = false;
    bool endExecution = false;
    
    // Requisições de I/O (processadas posteriormente)
    std::vector<std::unique_ptr<IORequest>> ioRequests;
    bool printLock = true;
    
    // Contexto de execução
    ControlContext context = {
        .registers = process->regBank,
        .memManager = *memory_manager,
        .ioRequests = ioRequests,
        .printLock = printLock,
        .process = *process,
        .counter = counter,
        .counterForEnd = counterForEnd,
        .endProgram = endProgram,
        .endExecution = endExecution
    };
    
    std::cout << "[Core " << core_id << "] Processo P" << process->pid 
              << " executando (quantum=" << process->quantum << " ciclos)\n";
    
    // Loop de execução respeitando o quantum
    int cycles_in_quantum = 0;
    
        while (!context.endProgram && !context.endExecution && 
            cycles_in_quantum < process->quantum && !stop_requested.load(std::memory_order_acquire)) {
        
        Instruction_Data data;
        
        try {
            // Pipeline MIPS de 5 estágios
            control_unit.Fetch(context);
            if (context.endProgram) break;
            
            control_unit.Decode(context.registers, data);
            control_unit.Execute(data, context);
            control_unit.Memory_Acess(data, context);
            control_unit.Write_Back(data, context);
            
            // Contabiliza ciclo
            cycles_in_quantum++;
            process->pipeline_cycles++;
            
            // 🆕 RASTREAR CICLO BUSY
            busy_cycles++;

            
        } catch (const std::exception& e) {
            std::cerr << "[Core " << core_id << "] Erro na execução de P" 
                      << process->pid << ": " << e.what() << "\n";
            // Em caso de erro fatal na execução do processo, marcar como FAILED
            process->mark_failed(std::string(e.what()));
            process->finish_time = cpu_time::now_ns();
            // Sinaliza que o programa terminou para evitar re-entrância e re-agendamento
            context.endProgram = true;
            endExecution = true;
            break;
        }
    }
    
    // Determina o estado final do processo
    if (context.endProgram) {
        process->set_state(State::Finished);
        process->finish_time = cpu_time::now_ns();
        
        std::cout << "[Core " << core_id << "] P" << process->pid 
                  << " FINALIZADO (total: " << process->pipeline_cycles.load() 
                  << " ciclos)\n";
        
    } else if (!ioRequests.empty()) {
        process->set_state(State::Blocked);
        
        std::cout << "[Core " << core_id << "] P" << process->pid 
                  << " BLOQUEADO (aguardando I/O)\n";
        
    } else {
        // Quantum expirou
        // Só re-agendar (Ready) se o processo não foi marcado como Finished por um erro
        if (process->get_state() != State::Finished) {
            process->set_state(State::Ready);
        }
        // 🆕 NÃO incrementar aqui - será feito no scheduler!
        // process->context_switches++;  // ❌ REMOVIDO
        
        std::cout << "[Core " << core_id << "] P" << process->pid 
                  << " PREEMPTADO (quantum expirado após " 
                  << cycles_in_quantum << " ciclos)\n";
    }
    
    // NÃO liberar núcleo aqui - isso será feito após o collect no scheduler!
    // Apenas marcar como idle para que scheduler saiba que terminou
    state.store(CoreState::IDLE);
    
    std::cout << "[Core " << core_id << "] Finalizado (agora IDLE)\n";
}

void Core::request_stop() {
    {
        std::lock_guard<std::mutex> lock(core_mutex);
        if (current_process != nullptr) {
            std::cout << "[Core " << core_id << "] request_stop() while segurando P"
                      << current_process->pid << " (state="
                      << to_string_state(current_process->get_state())
                      << ", thread_running=" << (execution_thread.joinable() ? "yes" : "no")
                      << ")\n";
        }
    }
    stop_requested.store(true, std::memory_order_release);
}

void Core::join_thread() {
    if (execution_thread.joinable()) {
        execution_thread.join();
    }
}
