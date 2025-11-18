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
#include "cpu/RoundRobinScheduler.hpp"  // ✅ ADICIONAR ISSO!
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

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


int main() {
    // Configuração do sistema multicore
    const int NUM_CORES = 2;  // Começamos com 2 núcleos para teste
    const int DEFAULT_QUANTUM = 100;  // Quantum padrão
    
    std::cout << "===========================================\n";
    std::cout << "  SIMULADOR MULTICORE - ROUND ROBIN\n";
    std::cout << "===========================================\n";
    std::cout << "Configuração:\n";
    std::cout << "  - Núcleos: " << NUM_CORES << "\n";
    std::cout << "  - Política: Round Robin\n";
    std::cout << "  - Quantum: " << DEFAULT_QUANTUM << " ciclos\n";
    std::cout << "===========================================\n\n";
    
    // 1. Inicialização dos Módulos Principais
    std::cout << "Inicializando o simulador...\n";
    MemoryManager memManager(1024, 8192);
    IOManager ioManager;
    
    // 2. Criar Escalonador Round Robin (gerencia núcleos internamente)
    RoundRobinScheduler scheduler(NUM_CORES, &memManager, &ioManager, DEFAULT_QUANTUM);
    std::cout << "✓ Escalonador Round Robin criado com " << NUM_CORES << " núcleos\n\n";

    // 3. Carregamento dos Processos
    std::vector<std::unique_ptr<PCB>> process_list;

    // Carrega um processo a partir de um arquivo JSON
    auto p1 = std::make_unique<PCB>();
    if (load_pcb_from_json("process1.json", *p1)) {
        std::cout << "Carregando programa 'tasks.json' para o processo " << p1->pid << "...\n";
        loadJsonProgram("tasks.json", memManager, *p1, 0);
        
        // Define tempo de chegada
        p1->arrival_time = 0;
        
        // Adiciona ao escalonador
        scheduler.add_process(p1.get());
        process_list.push_back(std::move(p1));
    } else {
        std::cerr << "Erro ao carregar 'process1.json'. Certifique-se de que o arquivo está na pasta raiz do projeto.\n";
        return 1;
    }

    std::cout << "✓ " << process_list.size() << " processo(s) carregado(s)\n\n";

    // 4. Loop Principal do Escalonador Round Robin
    std::cout << "\n===========================================\n";
    std::cout << "Iniciando escalonador Round-Robin Multicore...\n";
    std::cout << "===========================================\n\n";
    
    // Executa até todos processos terminarem
    while (scheduler.has_pending_processes()) {
        scheduler.schedule_cycle();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\n===========================================\n";
    std::cout << "Todos os processos foram finalizados!\n";
    std::cout << "===========================================\n\n";
    
    // 5. Exibir estatísticas do escalonador
    auto stats = scheduler.get_statistics();
    std::cout << "--- ESTATÍSTICAS DO ESCALONADOR ---\n";
    std::cout << "Total de processos:       " << scheduler.get_total_count() << "\n";
    std::cout << "Processos finalizados:    " << scheduler.get_finished_count() << "\n";
    std::cout << "Tempo médio de espera:    " << stats.avg_wait_time << " ciclos\n";
    std::cout << "Tempo médio de retorno:   " << stats.avg_turnaround_time << " ciclos\n";
    std::cout << "Utilização média da CPU:  " << (stats.avg_cpu_utilization * 100) << "%\n";
    std::cout << "Throughput:               " << stats.throughput << " processos/ciclo\n";
    std::cout << "Context switches totais:  " << stats.total_context_switches << "\n";
    std::cout << "------------------------------------\n\n";
    
    // 6. Exibir métricas individuais de cada processo
    for (const auto& process : process_list) {
        if (process->state == State::Finished) {
            print_metrics(*process);
        }
    }

    return 0;
}