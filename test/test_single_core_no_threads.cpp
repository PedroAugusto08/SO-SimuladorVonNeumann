#include <iostream>
#include <memory>
#include <vector>
#include <iomanip>
#include <chrono>

#include "memory/MemoryManager.hpp"
#include "memory/cache.hpp"
#include "IO/IOManager.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/PCB.hpp"
#include "cpu/CONTROL_UNIT.hpp"
#include "cpu/TimeUtils.hpp"
#include "parser_json/parser_json.hpp"

namespace {

struct ExecutionStats {
    int cycles = 0;
    bool finished = false;
};

ExecutionStats run_process_single_core(PCB &process,
                                       MemoryManager &memory_manager,
                                       Cache &single_core_cache,
                                       int max_cycles) {
    ExecutionStats stats;

    // Usa cache dedicada para simular um único core sem threads
    MemoryManager::setThreadCache(&single_core_cache);

    Control_Unit control_unit;
    std::vector<std::unique_ptr<IORequest>> io_requests;
    bool print_lock = true;
    int counter = 0;
    int counter_for_end = 0;
    bool end_program = false;
    bool end_execution = false;

    ControlContext context = {
        .registers = process.regBank,
        .memManager = memory_manager,
        .ioRequests = io_requests,
        .printLock = print_lock,
        .process = process,
        .counter = counter,
        .counterForEnd = counter_for_end,
        .endProgram = end_program,
        .endExecution = end_execution
    };

    process.state = State::Running;
    process.assigned_core = 0;
    process.start_time = cpu_time::now_ns();

    while (!context.endProgram && !context.endExecution && stats.cycles < max_cycles) {
        try {
            control_unit.Fetch(context);
            if (context.endProgram) {
                break;
            }

            Instruction_Data data;
            control_unit.Decode(context.registers, data);
            control_unit.Execute(data, context);
            control_unit.Memory_Acess(data, context);
            control_unit.Write_Back(data, context);

            stats.cycles++;
            process.pipeline_cycles++;
        } catch (const std::exception &ex) {
            std::cerr << "[SingleCore] Erro ao executar P" << process.pid
                      << ": " << ex.what() << "\n";
            context.endExecution = true;
            break;
        }
    }

    if (context.endProgram) {
        process.state = State::Finished;
        process.finish_time = cpu_time::now_ns();
        stats.finished = true;
    } else if (stats.cycles >= max_cycles) {
        process.state = State::Ready;
        std::cerr << "[SingleCore] P" << process.pid
                  << " atingiu o limite de ciclos sem finalizar." << std::endl;
    } else {
        process.state = State::Blocked;
    }

    return stats;
}

} // namespace

int main() {
    std::cout << "\n==============================================================\n";
    std::cout << "  TESTE: Execução sequencial sem threads (1 core)\n";
    std::cout << "==============================================================\n\n";

    constexpr int NUM_PROCESSES = 4;
    constexpr int MAX_CYCLES_PER_PROCESS = 1'000'000;
    const std::string process_file = "examples/processes/process1.json";
    const std::string program_file = "examples/programs/tasks.json";

    MemoryManager memory_manager(4096, 32768);
    IOManager io_manager;
    (void)io_manager; // Evita warning - mantido para futura extensão

    std::vector<std::unique_ptr<PCB>> processes;
    processes.reserve(NUM_PROCESSES);

    std::cout << "Carregando processos...\n";
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        auto pcb = std::make_unique<PCB>();
        if (!load_pcb_from_json(process_file, *pcb)) {
            std::cerr << "⚠️  Falha ao carregar " << process_file
                      << ". Abortando." << std::endl;
            return 1;
        }

        int start_addr = i * 2048;
        int end_addr = loadJsonProgram(program_file, memory_manager, *pcb, start_addr);
        pcb->program_start_addr = start_addr;
        pcb->program_size = end_addr - start_addr;
        pcb->regBank.pc.write(start_addr);
        pcb->state = State::Ready;

        std::cout << "  • P" << pcb->pid << " carregado: start=" << start_addr
                  << " size=" << pcb->program_size << " bytes" << std::endl;

        processes.push_back(std::move(pcb));
    }

    std::cout << "\nExecutando em um único core (sem threads)...\n";
    Cache single_core_cache;

    int finished = 0;
    int total_cycles = 0;

    for (const auto &pcb_ptr : processes) {
        PCB &process = *pcb_ptr;
        std::cout << "\n➡️  Iniciando P" << process.pid << "..." << std::endl;
        auto stats = run_process_single_core(process, memory_manager,
                                             single_core_cache, MAX_CYCLES_PER_PROCESS);

        total_cycles += stats.cycles;
        if (stats.finished) {
            finished++;
            std::cout << "   ✅ P" << process.pid << " finalizado em "
                      << stats.cycles << " ciclos" << std::endl;
        } else {
            std::cout << "   ❌ P" << process.pid
                      << " não finalizou (" << stats.cycles << " ciclos)" << std::endl;
        }
    }

    std::cout << "\n==============================================================\n";
    std::cout << "Resumo final" << std::endl;
    std::cout << "  • Processos finalizados: " << finished << "/" << NUM_PROCESSES << std::endl;
    std::cout << "  • Total de ciclos executados: " << total_cycles << std::endl;

    if (finished == NUM_PROCESSES) {
        std::cout << "  ✅ Todos os processos foram executados sem threads!" << std::endl;
    } else {
        std::cout << "  ⚠️  Nem todos os processos finalizaram. Verifique o log." << std::endl;
    }

    std::cout << "==============================================================\n\n";
    return (finished == NUM_PROCESSES) ? 0 : 2;
}
