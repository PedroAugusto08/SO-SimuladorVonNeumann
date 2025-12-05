#include <iostream>
#include <memory>
#include <vector>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <map>
#include "memory/MemoryManager.hpp"
#include "memory/cache.hpp"
#include "IO/IOManager.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/PCB.hpp"
#include "cpu/CONTROL_UNIT.hpp"
#include "cpu/TimeUtils.hpp"
#include "parser_json/parser_json.hpp"

namespace fs = std::filesystem;

struct WorkloadDefinition {
    std::string key;
    std::string process_file;
    std::string tasks_file;
};

std::string extract_key(const std::string& stem, const std::string& prefix) {
    if (stem.rfind(prefix, 0) == 0 && stem.size() > prefix.size()) {
        return stem.substr(prefix.size());
    }
    return stem;
}

std::vector<WorkloadDefinition> load_single_core_workloads(const std::string& process_dir,
                                                          const std::string& tasks_dir) {
    std::map<std::string, std::string> process_map;
    std::map<std::string, std::string> tasks_map;

    if (fs::exists(process_dir)) {
        for (const auto& entry : fs::directory_iterator(process_dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            const auto stem = entry.path().stem().string();
            const auto key = extract_key(stem, "process_");
            process_map[key] = entry.path().string();
        }
    }

    if (fs::exists(tasks_dir)) {
        for (const auto& entry : fs::directory_iterator(tasks_dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            const auto stem = entry.path().stem().string();
            const auto key = extract_key(stem, "tasks_");
            tasks_map[key] = entry.path().string();
        }
    }

    std::vector<WorkloadDefinition> workloads;
    workloads.reserve(process_map.size());

    for (const auto& [key, process_path] : process_map) {
        auto task_it = tasks_map.find(key);
        if (task_it == tasks_map.end()) {
            std::cerr << "⚠️  Sem tasks correspondentes para o processo '" << key << "'.\n";
            continue;
        }
        workloads.push_back({key, process_path, task_it->second});
    }

    std::sort(workloads.begin(), workloads.end(),
              [](const WorkloadDefinition& a, const WorkloadDefinition& b) {
                  return a.key < b.key;
              });
    return workloads;
}

std::string workload_display_name(const WorkloadDefinition& workload) {
    if (!workload.key.empty()) {
        return workload.key;
    }
    return fs::path(workload.process_file).stem().string();
}

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
    bool print_lock = false; // Não bloqueia prints em modo single-core
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

    constexpr int MAX_CYCLES_PER_PROCESS = 1'000'000;
    const std::string PROCESS_DIR = "processes";
    const std::string TASKS_DIR = "tasks";
    const auto workloads = load_single_core_workloads(PROCESS_DIR, TASKS_DIR);
    if (workloads.empty()) {
        std::cerr << "❌ Nenhum workload encontrado em '" << PROCESS_DIR
                  << "' ou '" << TASKS_DIR << "'.\n";
        return 1;
    }
    const int NUM_PROCESSES = static_cast<int>(workloads.size());

    MemoryManager memory_manager(4096, 32768);
    IOManager io_manager;
    (void)io_manager; // Evita warning - mantido para futura extensão

    std::vector<std::unique_ptr<PCB>> processes;
    processes.reserve(NUM_PROCESSES);

    std::cout << "Carregando processos (" << NUM_PROCESSES << " workloads)...\n";
    for (int i = 0; i < NUM_PROCESSES; ++i) {
        const auto& workload = workloads[i];
        auto pcb = std::make_unique<PCB>();
        if (!load_pcb_from_json(workload.process_file.c_str(), *pcb)) {
            std::cerr << "⚠️  Falha ao carregar " << workload.process_file
                      << ". Abortando." << std::endl;
            return 1;
        }

        pcb->pid = i + 1;
        pcb->name = workload_display_name(workload);

        int start_addr = i * 4096;
        int end_addr = loadJsonProgram(workload.tasks_file, memory_manager, *pcb, start_addr);
        pcb->program_start_addr = start_addr;
        pcb->program_size = end_addr - start_addr;
        pcb->regBank.pc.write(start_addr);
        pcb->state = State::Ready;

        std::cout << "  • " << pcb->name << " (P" << pcb->pid << ") carregado: start=" << start_addr
                  << " size=" << pcb->program_size << " bytes" << std::endl;

        processes.push_back(std::move(pcb));
    }

    std::cout << "\nExecutando em um único core (sem threads)...\n";
    Cache single_core_cache;

    int finished = 0;
    int total_cycles = 0;

    for (const auto &pcb_ptr : processes) {
        PCB &process = *pcb_ptr;
        std::cout << "\n➡️  Iniciando " << process.name << " (P" << process.pid << ")..." << std::endl;
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
    std::cout << "  • Workloads executados:\n";
    for (const auto& workload : workloads) {
        std::cout << "    - " << workload_display_name(workload) << std::endl;
    }

    if (finished == NUM_PROCESSES) {
        std::cout << "  ✅ Todos os processos foram executados sem threads!" << std::endl;
    } else {
        std::cout << "  ⚠️  Nem todos os processos finalizaram. Verifique o log." << std::endl;
    }

    std::cout << "==============================================================\n\n";
    return (finished == NUM_PROCESSES) ? 0 : 2;
}
