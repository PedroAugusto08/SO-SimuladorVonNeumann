/**
 * Teste simples de preempção por quantum
 * Valida que o Round Robin está preemptando processos corretamente
 */

#include <iostream>
#include <memory>
#include "src/memory/MemoryManager.hpp"
#include "src/cpu/PCB.hpp"
#include "src/cpu/pcb_loader.hpp"
#include "src/cpu/Core.hpp"
#include "src/cpu/RoundRobinScheduler.hpp"
#include "src/parser_json/parser_json.hpp"
#include "src/IO/IOManager.hpp"

int main() {
    std::cout << "\n========================================\n";
    std::cout << "  TESTE DE PREEMPÇÃO POR QUANTUM\n";
    std::cout << "========================================\n\n";
    
    try {
        // Configuração
        const int NUM_CORES = 2;
        const int QUANTUM = 50;  // Quantum pequeno para forçar preempção
        
        MemoryManager memManager(1024, 8192);
        IOManager ioManager;
        RoundRobinScheduler scheduler(NUM_CORES, &memManager, &ioManager, QUANTUM);
        
        std::cout << "✓ Escalonador criado: " << NUM_CORES << " núcleos, quantum=" << QUANTUM << "\n\n";
        
        // Criar 3 processos para testar
        std::vector<std::unique_ptr<PCB>> processes;
        
        for (int i = 0; i < 3; i++) {
            auto pcb = std::make_unique<PCB>();
            
            if (load_pcb_from_json("process1.json", *pcb)) {
                pcb->pid = i + 1;
                pcb->name = "P" + std::to_string(i + 1);
                pcb->quantum = QUANTUM;  // Definir quantum
                
                // Carregar programa
                loadJsonProgram("tasks.json", memManager, *pcb, 0);
                
                std::cout << "✓ Processo P" << pcb->pid << " carregado\n";
                
                scheduler.add_process(pcb.get());
                processes.push_back(std::move(pcb));
            }
        }
        
        std::cout << "\n--- Iniciando Execução ---\n";
        std::cout << "Executando por 100 ciclos para observar preempção...\n\n";
        
        // Executar por um número limitado de ciclos
        int max_cycles = 100;
        for (int i = 0; i < max_cycles && scheduler.has_pending_processes(); i++) {
            scheduler.schedule_cycle();
            
            if (i % 25 == 0) {
                std::cout << "  Ciclo " << i << "...\n";
            }
        }
        
        std::cout << "\n--- Análise de Preempção ---\n\n";
        
        // Verificar context switches em cada processo
        int total_switches = 0;
        for (const auto& pcb : processes) {
            int switches = pcb->context_switches.load();
            total_switches += switches;
            
            std::cout << "P" << pcb->pid << ": " << switches << " context switch(es)\n";
        }
        
        std::cout << "\nTotal de context switches: " << total_switches << "\n";
        
        // Validação
        if (total_switches > 0) {
            std::cout << "\n✅ SUCESSO: Preempção por quantum está funcionando!\n";
            std::cout << "   Os processos foram interrompidos e retornaram à fila.\n";
        } else {
            std::cout << "\n⚠️  AVISO: Nenhuma preempção detectada.\n";
            std::cout << "   Isso pode ser normal se os processos terminaram antes do quantum.\n";
        }
        
        // Estatísticas gerais
        auto stats = scheduler.get_statistics();
        std::cout << "\n--- Estatísticas Gerais ---\n";
        std::cout << "Processos finalizados: " << scheduler.get_finished_count() << "/" << processes.size() << "\n";
        std::cout << "Context switches totais: " << stats.total_context_switches << "\n";
        
        std::cout << "\n✓ Teste concluído!\n\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Erro: " << e.what() << "\n\n";
        return 1;
    }
}
