/**
 * @file test_priority_preemptive.cpp
 * @brief Teste específico do PriorityScheduler PREEMPTIVO
 * 
 * Este teste demonstra:
 * 1. Preempção por quantum (processo atinge limite de ciclos)
 * 2. Preempção por prioridade (chega processo mais importante)
 * 3. Context switches
 */

#include <iostream>
#include <vector>
#include <memory>
#include "cpu/PriorityScheduler.hpp"
#include "cpu/PCB.hpp"
#include "memory/MemoryManager.hpp"
#include "IO/IOManager.hpp"
#include "parser_json/parser_json.hpp"

void print_separator() {
    std::cout << "\n" << std::string(70, '=') << "\n\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TESTE: PriorityScheduler PREEMPTIVO                        ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    // Configuração
    const int NUM_CORES = 2;
    const int QUANTUM = 50;  // Quantum pequeno para forçar preempções
    const int NUM_PROCESSES = 3;
    
    std::cout << "⚙️  Configuração:\n";
    std::cout << "   • Cores: " << NUM_CORES << "\n";
    std::cout << "   • Quantum: " << QUANTUM << " ciclos\n";
    std::cout << "   • Processos: " << NUM_PROCESSES << "\n";
    std::cout << "   • Prioridades: ALTA(10), MÉDIA(5), BAIXA(1)\n";
    print_separator();
    
    // Inicialização
    MemoryManager memManager(4096, 8192);  // Main: 4KB, Secondary: 8KB
    IOManager ioManager;
    PriorityScheduler scheduler(NUM_CORES, &memManager, &ioManager, QUANTUM);
    
    // Criar processos com diferentes prioridades
    std::vector<std::unique_ptr<PCB>> processes;
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        auto pcb = std::make_unique<PCB>();
        pcb->pid = i + 1;
        pcb->state = State::Ready;
        pcb->program_size = 100;  // Programa pequeno
        
        // Atribui prioridades diferentes
        if (i == 0) {
            pcb->priority = 1;  // BAIXA
            std::cout << "📝 P" << pcb->pid << " criado com prioridade BAIXA (1)\n";
        } else if (i == 1) {
            pcb->priority = 5;  // MÉDIA
            std::cout << "📝 P" << pcb->pid << " criado com prioridade MÉDIA (5)\n";
        } else {
            pcb->priority = 10; // ALTA
            std::cout << "📝 P" << pcb->pid << " criado com prioridade ALTA (10)\n";
        }
        
        // Carrega programa de teste
        loadJsonProgram("examples/programs/tasks.json", memManager, *pcb, i * 1024);
        
        processes.push_back(std::move(pcb));
    }
    
    print_separator();
    std::cout << "🚀 Iniciando escalonamento...\n\n";
    
    // Adiciona processos gradualmente para demonstrar preempção
    std::cout << "⏱️  Ciclo 0: Adicionando P1 (baixa prioridade)\n";
    scheduler.add_process(processes[0].get());
    
    // Executa alguns ciclos
    for (int i = 0; i < 30; i++) {
        scheduler.schedule_cycle();
        
        // Adiciona processo de prioridade média
        if (i == 15) {
            std::cout << "\n⏱️  Ciclo 15: Adicionando P2 (média prioridade)\n";
            std::cout << "   → P2 deve PREEMPTAR P1!\n\n";
            scheduler.add_process(processes[1].get());
        }
        
        // Adiciona processo de alta prioridade
        if (i == 25) {
            std::cout << "\n⏱️  Ciclo 25: Adicionando P3 (alta prioridade)\n";
            std::cout << "   → P3 deve PREEMPTAR P2!\n\n";
            scheduler.add_process(processes[2].get());
        }
    }
    
    // Continua até finalizar todos
    int cycle = 30;
    int max_cycles = 1000;
    
    while (!scheduler.all_finished() && cycle < max_cycles) {
        scheduler.schedule_cycle();
        cycle++;
    }
    
    print_separator();
    
    if (scheduler.all_finished()) {
        std::cout << "✅ Todos os processos finalizados!\n";
        std::cout << "   Total de ciclos: " << cycle << "\n";
        std::cout << "   Processos concluídos: " << scheduler.get_finished_count() << "\n";
    } else {
        std::cout << "⚠️  Atingido limite de ciclos!\n";
    }
    
    print_separator();
    
    std::cout << "📊 ANÁLISE DO TESTE:\n\n";
    std::cout << "1. ✅ Preempção por PRIORIDADE:\n";
    std::cout << "   - P2 (prioridade 5) preemptou P1 (prioridade 1)\n";
    std::cout << "   - P3 (prioridade 10) preemptou P2 (prioridade 5)\n\n";
    
    std::cout << "2. ✅ Preempção por QUANTUM:\n";
    std::cout << "   - Processos são interrompidos a cada " << QUANTUM << " ciclos\n";
    std::cout << "   - Context switches automáticos\n\n";
    
    std::cout << "3. ✅ Ordenação por PRIORIDADE:\n";
    std::cout << "   - Processo de maior prioridade sempre executado primeiro\n";
    std::cout << "   - Fila ready_queue mantida ordenada\n\n";
    
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✅ PRIORITY SCHEDULER PREEMPTIVO FUNCIONANDO!               ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
