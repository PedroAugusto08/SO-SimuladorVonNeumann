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
#include "cpu/Core.hpp"  // NOVO: Classe Core para multicore
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


int main(int argc, char* argv[]) {
    // Configuração do sistema multicore
    int NUM_CORES = 2;  // Padrão: 2 núcleos
    int QUANTUM = 100;   // Padrão: 100 ciclos
    bool NON_PREEMPTIVE = false;  // Padrão: preemptivo
    
    // Lista de pares (programa.json, pcb.json) para carregar
    std::vector<std::pair<std::string, std::string>> process_files;
    
    // Parse de argumentos da linha de comando
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--non-preemptive" || arg == "-np") {
            NON_PREEMPTIVE = true;
        } else if (arg == "--cores" || arg == "-c") {
            if (i + 1 < argc) {
                NUM_CORES = std::atoi(argv[++i]);
            }
        } else if (arg == "--quantum" || arg == "-q") {
            if (i + 1 < argc) {
                QUANTUM = std::atoi(argv[++i]);
            }
        } else if (arg == "--process" || arg == "-p") {
            // Formato: --process programa.json pcb.json
            if (i + 2 < argc) {
                std::string program = argv[++i];
                std::string pcb = argv[++i];
                process_files.push_back({program, pcb});
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Uso: " << argv[0] << " [opções]\n";
            std::cout << "Opções:\n";
            std::cout << "  --cores, -c N              Número de núcleos (padrão: 2)\n";
            std::cout << "  --quantum, -q N            Quantum em ciclos (padrão: 100)\n";
            std::cout << "  --non-preemptive, -np      Modo não-preemptivo (sem quantum)\n";
            std::cout << "  --process, -p PROG PCB     Adiciona processo (pode repetir)\n";
            std::cout << "  --help, -h                 Exibe esta ajuda\n";
            std::cout << "\nExemplo:\n";
            std::cout << "  " << argv[0] << " -c 2 -p tasks.json process1.json\n";
            std::cout << "  " << argv[0] << " --non-preemptive -p prog1.json pcb1.json -p prog2.json pcb2.json\n";
            return 0;
        }
    }
    
    // Se nenhum processo foi especificado, usa configuração padrão
    if (process_files.empty()) {
        process_files.push_back({"tasks.json", "process1.json"});
    }
    
    std::cout << "===========================================";
    std::cout << "\n  SIMULADOR MULTICORE - ROUND ROBIN\n";
    std::cout << "===========================================";
    std::cout << "\nConfiguração:\n";
    std::cout << "  - Núcleos: " << NUM_CORES << "\n";
    std::cout << "  - Política: Round Robin\n";
    std::cout << "  - Modo: " << (NON_PREEMPTIVE ? "NÃO-PREEMPTIVO" : "PREEMPTIVO") << "\n";
    if (!NON_PREEMPTIVE) {
        std::cout << "  - Quantum: " << QUANTUM << " ciclos\n";
    }
    std::cout << "===========================================\n\n";
    
    // 1. Inicialização dos Módulos Principais
    std::cout << "Inicializando o simulador...\n";
    MemoryManager memManager(1024, 8192);
    IOManager ioManager;
    
    // 2. Criar núcleos de processamento
    std::vector<std::unique_ptr<Core>> cores;
    for (int i = 0; i < NUM_CORES; i++) {
        cores.push_back(std::make_unique<Core>(i, &memManager));
    }
    std::cout << "✓ " << NUM_CORES << " núcleos criados\n\n";

    // 3. Carregamento dos Processos
    std::vector<std::unique_ptr<PCB>> process_list;
    std::deque<PCB*> ready_queue;
    std::vector<PCB*> blocked_list;

    std::cout << "\n===========================================\n";
    std::cout << "  CARREGAMENTO DE PROCESSOS\n";
    std::cout << "===========================================\n";
    std::cout << "Total de processos a carregar: " << process_files.size() << "\n\n";
    
    // IMPORTANTE: Todos os processos devem ser carregados na memória ANTES da execução
    uint32_t next_base_address = 0;
    
    for (size_t i = 0; i < process_files.size(); i++) {
        const auto& [program_file, pcb_file] = process_files[i];
        
        auto pcb = std::make_unique<PCB>();
        
        // Carrega PCB do arquivo
        if (!load_pcb_from_json(pcb_file, *pcb)) {
            std::cerr << "❌ ERRO ao carregar PCB '" << pcb_file << "'\n";
            std::cerr << "   Verifique se o arquivo existe e está formatado corretamente.\n";
            return 1;
        }
        
        std::cout << "Processo " << (i+1) << "/" << process_files.size() << ":\n";
        std::cout << "  ├─ PID:       " << pcb->pid << "\n";
        std::cout << "  ├─ Nome:      " << pcb->name << "\n";
        std::cout << "  ├─ Quantum:   " << (NON_PREEMPTIVE ? 999999 : pcb->quantum) << "\n";
        std::cout << "  ├─ PCB:       " << pcb_file << "\n";
        std::cout << "  └─ Programa:  " << program_file << "\n";
        
        // Sobrescreve quantum se modo não-preemptivo
        if (NON_PREEMPTIVE) {
            pcb->quantum = 999999;
        }
        
        // Carrega programa na memória
        std::cout << "     └─> Carregando instruções na memória...\n";
        
        try {
            loadJsonProgram(program_file, memManager, *pcb, next_base_address);
            
            std::cout << "     └─> ✓ Carregado no endereço base 0x" 
                      << std::hex << next_base_address << std::dec << "\n";
            
            // Atualiza endereço base para próximo processo
            // Cada processo ocupa no mínimo 1KB (256 words de 4 bytes)
            next_base_address += 1024;
            
            // Define tempo de chegada (todos chegam no tempo 0 conforme especificação)
            pcb->arrival_time = 0;
            
            process_list.push_back(std::move(pcb));
            std::cout << "\n";
            
        } catch (const std::exception& e) {
            std::cerr << "     └─> ❌ ERRO ao carregar programa: " << e.what() << "\n";
            return 1;
        }
    }

    std::cout << "===========================================\n";
    std::cout << "✓ Todos os " << process_list.size() 
              << " processos foram carregados na memória\n";
    std::cout << "===========================================\n\n";

    // Adiciona os processos na fila de prontos
    for (const auto& process : process_list) {
        ready_queue.push_back(process.get());
    }

    int total_processes = process_list.size();
    int finished_processes = 0;
    uint64_t current_time = 0;

    // 4. Loop Principal do Escalonador Multicore
    std::cout << "\n===========================================\n";
    std::cout << "Iniciando escalonador Round-Robin Multicore...\n";
    std::cout << "===========================================\n\n";
    
    while (finished_processes < total_processes) {
        current_time++;
        
        // 4.1 Verifica processos desbloqueados pelo IOManager
        for (auto it = blocked_list.begin(); it != blocked_list.end(); ) {
            if ((*it)->state == State::Ready) {
                std::cout << "[Scheduler] Processo P" << (*it)->pid 
                          << " desbloqueado - movendo para fila de prontos\n";
                ready_queue.push_back(*it);
                it = blocked_list.erase(it);
            } else {
                ++it;
            }
        }

        // 4.2 Atribui processos a núcleos livres
        for (auto& core : cores) {
            if (core->is_idle() && !ready_queue.empty()) {
                PCB* process = ready_queue.front();
                ready_queue.pop_front();
                
                std::cout << "[Scheduler] Atribuindo P" << process->pid 
                          << " ao Core " << core->get_id() << "\n";
                
                // Detecta migração de núcleo
                if (process->last_core != -1 && 
                    process->last_core != core->get_id()) {
                    std::cout << "           └─> Migração: Core " 
                              << process->last_core << " → Core " 
                              << core->get_id() << "\n";
                }
                
                try {
                    core->execute_async(process);
                } catch (const std::exception& e) {
                    std::cerr << "[Scheduler] ERRO ao executar P" 
                              << process->pid << ": " << e.what() << "\n";
                    process->state = State::Finished;
                }
            }
        }

        // 4.3 Coleta processos que terminaram execução
        for (auto& core : cores) {
            if (!core->is_idle() && !core->is_thread_running()) {
                PCB* process = core->get_current_process();
                
                if (process) {
                    // Aguarda thread finalizar
                    core->wait_completion();
                    
                    // Trata estado do processo
                    switch (process->state) {
                        case State::Finished:
                            std::cout << "[Scheduler] P" << process->pid 
                                      << " FINALIZADO no Core " << core->get_id() 
                                      << "\n";
                            print_metrics(*process);
                            finished_processes++;
                            break;
                        
                        case State::Blocked:
                            std::cout << "[Scheduler] P" << process->pid 
                                      << " BLOQUEADO (I/O) - entregando ao IOManager\n";
                            ioManager.registerProcessWaitingForIO(process);
                            blocked_list.push_back(process);
                            break;
                        
                        default:  // Ready - quantum expirou
                            std::cout << "[Scheduler] P" << process->pid 
                                      << " quantum expirou - voltando para fila\n";
                            process->total_wait_time++;
                            ready_queue.push_back(process);
                            break;
                    }
                }
            }
        }
        
        // 4.4 Atualiza tempo de espera dos processos na fila
        for (PCB* process : ready_queue) {
            process->total_wait_time++;
        }

        // 4.5 Pequena pausa se não há trabalho
        if (ready_queue.empty() && 
            std::all_of(cores.begin(), cores.end(), 
                       [](const auto& c) { return c->is_idle(); })) {
            if (blocked_list.empty()) {
                break;  // Nada mais a fazer
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    std::cout << "\n===========================================\n";
    std::cout << "Todos os processos foram finalizados!\n";
    std::cout << "Tempo total de simulação: " << current_time << " ciclos\n";
    std::cout << "===========================================\n";

    return 0;
}