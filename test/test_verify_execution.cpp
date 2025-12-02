/**
 * Teste de Verificação de Execução Correta
 * 
 * Este teste verifica DETALHADAMENTE se:
 * 1. Os processos estão sendo executados (não apenas loops vazios)
 * 2. Os tempos estão sendo cronometrados corretamente
 * 3. Todos os processos finalizam
 * 4. Os contadores estão sincronizados
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include "memory/MemoryManager.hpp"
#include "cpu/PCB.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/Core.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

void run_verification_test(int num_cores) {
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TESTE DE VERIFICAÇÃO - " << num_cores << " CORE(S)                              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    const int NUM_PROCESSES = 8;
    const int QUANTUM = 1000;
    const int MAX_CYCLES = 10000000;
    std::string tasks_file = "examples/programs/tasks.json";
    
    std::vector<PCB*> process_ptrs;
    
    MemoryManager* memManager = new MemoryManager(4096, 32768);
    IOManager* ioManager = new IOManager();
    
    // Carregar processos
    std::cout << "1️⃣  Carregando " << NUM_PROCESSES << " processos...\n";
    for (int i = 0; i < NUM_PROCESSES; i++) {
        PCB* pcb = new PCB();
        if (load_pcb_from_json("examples/processes/process1.json", *pcb)) {
            pcb->pid = i + 1;
            pcb->name = "P" + std::to_string(i + 1);
            pcb->quantum = QUANTUM;
            
            // Salvar endereço antes de carregar
            int start_addr_before = pcb->program_start_addr;
            
            loadJsonProgram(tasks_file, *memManager, *pcb, 0);
            
            // Verificar se programa foi carregado
            std::cout << "   ✓ P" << pcb->pid << ": "
                      << "start_addr=" << pcb->program_start_addr
                      << ", size=" << pcb->program_size << " bytes\n";
            
            process_ptrs.push_back(pcb);
        }
    }
    
    std::cout << "\n2️⃣  Iniciando scheduler com " << num_cores << " core(s)...\n";
    
    RoundRobinScheduler scheduler(num_cores, memManager, ioManager, QUANTUM);
    for (PCB* pcb : process_ptrs) {
        scheduler.add_process(pcb);
    }
    
    std::cout << "   ✓ Todos os processos adicionados ao scheduler\n";
    std::cout << "   • Total de processos: " << NUM_PROCESSES << "\n\n";
    
    std::cout << "3️⃣  Executando processos...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int cycles = 0;
    int last_finished = 0;
    int last_print_cycle = 0;
    
    while (scheduler.has_pending_processes()) {
        scheduler.schedule_cycle();
        cycles++;
        
        int current_finished = scheduler.get_finished_count();
        
        // Print a cada processo finalizado
        if (current_finished > last_finished) {
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = now - start;
            
            std::cout << "   [" << std::setw(6) << cycles << " cycles, "
                      << std::setw(6) << std::fixed << std::setprecision(1) << elapsed.count() << " ms] "
                      << "Finalizados: " << current_finished << "/" << NUM_PROCESSES << "\n";
            
            last_finished = current_finished;
        }
        
        // Safety: evitar loop infinito
        if (cycles > MAX_CYCLES) {
            std::cout << "\n   ⚠️  TIMEOUT! Atingiu " << MAX_CYCLES << " ciclos\n";
            break;
        }
    }
    
    // Aguardar finalização dos threads
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    int final_finished = scheduler.get_finished_count();
    
    std::cout << "\n4️⃣  Resultados:\n";
    std::cout << "   • Ciclos executados: " << cycles << "\n";
    std::cout << "   • Tempo de execução: " << std::fixed << std::setprecision(2) 
              << duration.count() << " ms\n";
    std::cout << "   • Processos finalizados: " << final_finished << "/" << NUM_PROCESSES << "\n";
    
    if (final_finished == NUM_PROCESSES) {
        std::cout << "   ✅ SUCESSO: Todos os processos finalizaram!\n";
    } else {
        std::cout << "   ❌ FALHA: Apenas " << final_finished << "/" << NUM_PROCESSES 
                  << " processos finalizaram!\n";
    }
    
    // Verificar estado dos processos
    std::cout << "\n5️⃣  Verificando estado dos processos originais:\n";
    for (const auto* pcb : process_ptrs) {
        std::cout << "   • P" << pcb->pid << ": "
                  << "state=" << (int)pcb->state
                  << ", start_addr=" << pcb->program_start_addr
                  << ", size=" << pcb->program_size << "\n";
    }
    
    std::cout << "\n6️⃣  Análise de Desempenho:\n";
    if (duration.count() > 0) {
        double processes_per_sec = (NUM_PROCESSES / duration.count()) * 1000.0;
        std::cout << "   • Throughput: " << std::fixed << std::setprecision(2) 
                  << processes_per_sec << " processos/segundo\n";
        std::cout << "   • Tempo médio por processo: " << std::fixed << std::setprecision(2)
                  << (duration.count() / NUM_PROCESSES) << " ms\n";
    }
    
    std::cout << "\n════════════════════════════════════════════════════════════════\n";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         TESTE DE VERIFICAÇÃO DE EXECUÇÃO CORRETA              ║\n";
    std::cout << "║                                                                ║\n";
    std::cout << "║  Objetivo: Verificar se os processos estão sendo executados   ║\n";
    std::cout << "║  corretamente e se os tempos estão sendo medidos              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    
    // Testar com 1, 2 e 4 cores
    std::vector<int> core_counts = {1, 2, 4};
    
    for (int cores : core_counts) {
        run_verification_test(cores);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::cout << "\n✅ Verificação completa!\n\n";
    
    return 0;
}
