#include "Core.hpp"
#include "CONTROL_UNIT.hpp"
#include "../memory/MemoryManager.hpp"
#include "PCB.hpp"
#include "../IO/IOManager.hpp"
#include <iostream>
#include <chrono>

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
    process->state = State::Running;
    
    // Registra tempo de início se for a primeira execução
    if (process->start_time == 0) {
        process->start_time = std::chrono::steady_clock::now()
                                 .time_since_epoch().count();
    }
    
    // Marca núcleo como ocupado
    state.store(CoreState::BUSY);
    
    std::cout << "[Core " << core_id << "] Iniciando execução do processo P" 
              << process->pid << " (quantum=" << process->quantum << ")\n";
    
    // Inicia thread de execução
    execution_thread = std::thread(&Core::run_process, this, process);
}

void Core::wait_completion() {
    if (execution_thread.joinable()) {
        execution_thread.join();
    }
}

void Core::run_process(PCB* process) {
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
    
    while (!endProgram && !endExecution && 
           cycles_in_quantum < process->quantum) {
        
        Instruction_Data data;
        
        try {
            // Pipeline MIPS de 5 estágios
            control_unit.Fetch(context);
            if (endProgram) break;
            
            control_unit.Decode(context.registers, data);
            control_unit.Execute(data, context);
            control_unit.Memory_Acess(data, context);
            control_unit.Write_Back(data, context);
            
            // Contabiliza ciclo
            cycles_in_quantum++;
            process->pipeline_cycles++;
            
        } catch (const std::exception& e) {
            std::cerr << "[Core " << core_id << "] Erro na execução de P" 
                      << process->pid << ": " << e.what() << "\n";
            endExecution = true;
            break;
        }
    }
    
    // Determina o estado final do processo
    if (endProgram) {
        process->state = State::Finished;
        process->finish_time = std::chrono::steady_clock::now()
                                  .time_since_epoch().count();
        
        std::cout << "[Core " << core_id << "] P" << process->pid 
                  << " FINALIZADO (total: " << process->pipeline_cycles.load() 
                  << " ciclos)\n";
        
    } else if (!ioRequests.empty()) {
        process->state = State::Blocked;
        
        std::cout << "[Core " << core_id << "] P" << process->pid 
                  << " BLOQUEADO (aguardando I/O)\n";
        
    } else {
        // Quantum expirou
        process->state = State::Ready;
        process->context_switches++;
        
        std::cout << "[Core " << core_id << "] P" << process->pid 
                  << " PREEMPTADO (quantum expirado após " 
                  << cycles_in_quantum << " ciclos)\n";
    }
    
    // Libera núcleo
    {
        std::lock_guard<std::mutex> lock(core_mutex);
        current_process = nullptr;
        state.store(CoreState::IDLE);
    }
    
    std::cout << "[Core " << core_id << "] Liberado (agora IDLE)\n";
}
