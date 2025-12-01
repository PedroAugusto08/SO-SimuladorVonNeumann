#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>

#include "memory/MemoryManager.hpp"
#include "cpu/PCB.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/CONTROL_UNIT.hpp"
#include "cpu/Core.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"
#include "memory/MemoryMetrics.hpp"

// Função para imprimir as métricas de um processo
void print_metrics(const PCB& pcb) {
    std::cout << "\n--- METRICAS FINAIS DO PROCESSO " << pcb.pid << " ---\n";
    std::cout << "Nome do Processo:       " << pcb.name << "\n";
    std::cout << "Estado Final:           " << (pcb.state == State::Finished ? "Finished" : "Incomplete") << "\n";
    std::cout << "Ciclos de Pipeline:     " << pcb.pipeline_cycles.load() << "\n";
    std::cout << "Total de Acessos a Mem: " << pcb.mem_accesses_total.load() << "\n";
    std::cout << "  - Leituras:             " << pcb.mem_reads.load() << "\n";
    std::cout << "  - Escritas:             " << pcb.mem_writes.load() << "\n";
    std::cout << "Acessos a Cache L1:     " << pcb.cache_mem_accesses.load() << "\n";
    std::cout << "Acessos a Mem Principal:" << pcb.primary_mem_accesses.load() << "\n";
    std::cout << "Acessos a Mem Secundaria:" << pcb.secondary_mem_accesses.load() << "\n";
    std::cout << "Ciclos Totais de Memoria: " << pcb.memory_cycles.load() << "\n";
    std::cout << "------------------------------------------\n";
    // cria pasta "output" se não existir
    std::filesystem::create_directory("output");

    // abre arquivos
    std::ofstream resultados("output/resultados.dat");
    std::ofstream output("output/output.dat");

    if (resultados.is_open()) {
        resultados << "=== Resultados de Execução ===\n";
        resultados << "PID: " << pcb.pid << "\n";
        resultados << "Nome: " << pcb.name << "\n";
        resultados << "Quantum: " << pcb.quantum << "\n";
        resultados << "Prioridade: " << pcb.priority << "\n";
        resultados << "Ciclos de Pipeline: " << pcb.pipeline_cycles << "\n";
        resultados << "Ciclos de Memória: " << pcb.memory_cycles << "\n";
        resultados << "Cache Hits: " << pcb.cache_hits << "\n";
        resultados << "Cache Misses: " << pcb.cache_misses << "\n";
        resultados << "Ciclos de IO: " << pcb.io_cycles << "\n";
    }


    if (output.is_open()) {
        output << "=== Saída Lógica do Programa ===\n";

        // Dump de registradores
        output << "Registradores principais:\n";
        output << pcb.regBank.get_registers_as_string() << "\n";

        // Inserir operações registradas
        output << "\n=== Operações Executadas ===\n";

        // Lê o arquivo temporário com operações
        std::string temp_filename = "output/temp_1.log";
        if (std::filesystem::exists(temp_filename)) {
            std::ifstream temp_file(temp_filename);
            if (temp_file.is_open()) {
                std::string line;
                while (std::getline(temp_file, line)) {
                    output << line << "\n";
                }
                temp_file.close();
            }

            // Remove arquivo temporário após consolidar
            std::filesystem::remove(temp_filename);
        } else {
            output << "(Nenhuma operação registrada)\n";
        }

        output << "\n=== Fim das Operações Registradas ===\n";
    }



    resultados.close();
    output.close();
}


void print_help() {
    std::cout << "===========================================\n";
    std::cout << "  SIMULADOR VON NEUMANN MULTICORE\n";
    std::cout << "===========================================\n\n";
    std::cout << "USO:\n";
    std::cout << "  ./bin/simulador [OPÇÕES]\n\n";
    std::cout << "OPÇÕES:\n";
    std::cout << "  -h, --help              Exibe esta mensagem de ajuda\n\n";
    std::cout << "  -c, --cores NUM         Define o número de núcleos (padrão: 2)\n";
    std::cout << "                          Exemplo: --cores 4\n\n";
    std::cout << "  -q, --quantum NUM       Define o quantum para escalonamento RR e Priority\n";
    std::cout << "                          (padrão: 100 ciclos)\n";
    std::cout << "                          Exemplo: --quantum 50\n\n";
    std::cout << "  -s, --policy POLÍTICA   Define a política de escalonamento\n";
    std::cout << "                          Opções: RR, FCFS, SJN, PRIORITY\n";
    std::cout << "                          Padrão: RR (Round Robin)\n";
    std::cout << "                          Exemplo: --policy FCFS\n\n";
    std::cout << "  -p, --process PROG PCB  Adiciona um processo ao sistema\n";
    std::cout << "                          PROG: arquivo JSON com o programa\n";
    std::cout << "                          PCB: arquivo JSON com metadados do processo\n";
    std::cout << "                          Pode ser usado múltiplas vezes\n";
    std::cout << "                          Exemplo: --process tasks.json process1.json\n\n";
    std::cout << "POLÍTICAS DE ESCALONAMENTO:\n";
    std::cout << "  RR        - Round Robin (preemptivo com quantum)\n";
    std::cout << "  FCFS      - First Come First Served (não preemptivo)\n";
    std::cout << "  SJN       - Shortest Job Next (não preemptivo)\n";
    std::cout << "  PRIORITY  - Priority Scheduling (preemptivo com quantum)\n\n";
    std::cout << "ARQUIVOS DE SAÍDA:\n";
    std::cout << "  output/resultados.dat           - Métricas de execução\n";
    std::cout << "  output/output.dat               - Saída lógica do programa\n";
    std::cout << "  logs/memory_utilization.csv     - Utilização de memória\n";
    std::cout << "  logs/multicore_results.csv      - Resultados multicore (testes)\n";
    std::cout << "  logs/throughput_results.csv     - Métricas de throughput (testes)\n\n";
    std::cout << "EXEMPLOS:\n";
    std::cout << "  # Executar com configurações padrão\n";
    std::cout << "  ./bin/simulador\n\n";
    std::cout << "  # 4 núcleos com política FCFS\n";
    std::cout << "  ./bin/simulador --cores 4 --policy FCFS\n\n";
    std::cout << "  # Round Robin com quantum 50 em 8 núcleos\n";
    std::cout << "  ./bin/simulador -c 8 -q 50 -s RR\n\n";
    std::cout << "  # Múltiplos processos com diferentes prioridades\n";
    std::cout << "  ./bin/simulador --policy PRIORITY \\\n";
    std::cout << "    -p examples/programs/tasks.json examples/processes/process_high.json \\\n";
    std::cout << "    -p examples/programs/tasks.json examples/processes/process_low.json\n\n";
    std::cout << "MAIS INFORMAÇÕES:\n";
    std::cout << "  README.md               - Documentação completa do projeto\n";
    std::cout << "  MAKEFILE_COMMANDS.md    - Comandos de compilação e teste\n";
    std::cout << "===========================================\n";
}

int main(int argc, char* argv[]) {
    // Verificar se --help foi solicitado
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        }
    }
    
    // Configuração do sistema multicore
    int NUM_CORES = 2;
    int DEFAULT_QUANTUM = 100;
    std::string SCHED_POLICY = "RR";
    // Parse de argumentos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--cores" || arg == "-c") {
            if (i + 1 < argc) NUM_CORES = std::atoi(argv[++i]);
        } else if (arg == "--quantum" || arg == "-q") {
            if (i + 1 < argc) DEFAULT_QUANTUM = std::atoi(argv[++i]);
        } else if (arg == "--policy" || arg == "-s") {
            if (i + 1 < argc) SCHED_POLICY = argv[++i];
        }
    }
    std::cout << "===========================================\n";
    std::cout << "  SIMULADOR MULTICORE\n";
    std::cout << "===========================================\n";
    std::cout << "Configuração:\n";
    std::cout << "  - Núcleos: " << NUM_CORES << "\n";
    std::cout << "  - Política: ";
    if (SCHED_POLICY == "FCFS") std::cout << "FCFS";
    else if (SCHED_POLICY == "SJN") std::cout << "SJN";
    else if (SCHED_POLICY == "PRIORITY") std::cout << "Priority (Preemptivo)";
    else std::cout << "Round Robin";
    std::cout << "\n";
    if (SCHED_POLICY == "RR") std::cout << "  - Quantum: " << DEFAULT_QUANTUM << " ciclos\n";
    std::cout << "===========================================\n\n";
    // Inicialização dos módulos
    MemoryManager memManager(1024, 8192);
    IOManager ioManager;
    MemoryMetrics memMetrics("logs/memory_utilization.csv");
    // Escolha do escalonador
    std::unique_ptr<RoundRobinScheduler> rr_sched;
    std::unique_ptr<FCFSScheduler> fcfs_sched;
    std::unique_ptr<SJNScheduler> sjn_sched;
    std::unique_ptr<PriorityScheduler> priority_sched;
    
    if (SCHED_POLICY == "FCFS") {
        fcfs_sched = std::make_unique<FCFSScheduler>(NUM_CORES, &memManager, &ioManager);
    } else if (SCHED_POLICY == "SJN") {
        sjn_sched = std::make_unique<SJNScheduler>(NUM_CORES, &memManager, &ioManager);
    } else if (SCHED_POLICY == "PRIORITY") {
        priority_sched = std::make_unique<PriorityScheduler>(NUM_CORES, &memManager, &ioManager, DEFAULT_QUANTUM);
    } else {
        rr_sched = std::make_unique<RoundRobinScheduler>(NUM_CORES, &memManager, &ioManager, DEFAULT_QUANTUM);
    }
    // Carregamento dos processos
    std::vector<std::unique_ptr<PCB>> process_list;
    std::vector<std::pair<std::string, std::string>> process_files;
    // Parse de argumentos para múltiplos processos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--process" || arg == "-p") {
            if (i + 2 < argc) {
                std::string prog = argv[++i];
                std::string pcb = argv[++i];
                process_files.push_back({prog, pcb});
            }
        }
    }
    if (process_files.empty()) {
        process_files.push_back({"examples/programs/tasks.json", "examples/processes/process1.json"});
    }
    uint32_t next_base_address = 0;
    for (size_t i = 0; i < process_files.size(); i++) {
        const auto& [program_file, pcb_file] = process_files[i];
        auto pcb = std::make_unique<PCB>();
        if (!load_pcb_from_json(pcb_file, *pcb)) {
            std::cerr << "Erro ao carregar '" << pcb_file << "'.\n";
            return 1;
        }
        loadJsonProgram(program_file, memManager, *pcb, next_base_address);
        pcb->arrival_time = 0;
            // Estimativa: usar tamanho do programa como proxy de job size
            pcb->estimated_job_size = pcb->program_size;
        next_base_address += 1024;
        if (SCHED_POLICY == "FCFS") fcfs_sched->add_process(pcb.get());
        else if (SCHED_POLICY == "SJN") sjn_sched->add_process(pcb.get());
        else if (SCHED_POLICY == "PRIORITY") priority_sched->add_process(pcb.get());
        else rr_sched->add_process(pcb.get());
        process_list.push_back(std::move(pcb));
    }
    std::cout << "✓ " << process_list.size() << " processo(s) carregado(s)\n\n";
    // Loop principal
    std::cout << "\n===========================================\n";
    std::cout << "Iniciando escalonador...\n";
    std::cout << "===========================================\n\n";
    auto record_mem = [&]() {
        memMetrics.record(
            memManager.getUsedMainMemory(),
            memManager.getUsedSecondaryMemory(),
            memManager.getTotalCacheHits(),
            memManager.getTotalCacheMisses()
        );
    };
    if (SCHED_POLICY == "FCFS") {
        while (!fcfs_sched->all_finished()) {
            fcfs_sched->schedule_cycle();
            record_mem();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } else if (SCHED_POLICY == "SJN") {
        while (!sjn_sched->all_finished()) {
            sjn_sched->schedule_cycle();
            record_mem();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } else if (SCHED_POLICY == "PRIORITY") {
        while (priority_sched->has_pending_processes()) {
            priority_sched->schedule_cycle();
            record_mem();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } else {
        while (rr_sched->has_pending_processes()) {
            rr_sched->schedule_cycle();
            record_mem();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    memMetrics.flush();
    std::cout << "\n===========================================\n";
    std::cout << "Todos os processos foram finalizados!\n";
    std::cout << "===========================================\n\n";
    // Exibir métricas individuais
    for (const auto& process : process_list) {
        if (process->state == State::Finished) {
            print_metrics(*process);
        }
    }
    return 0;
}