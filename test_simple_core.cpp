#include "src/cpu/Core.hpp"
#include "src/cpu/RoundRobinScheduler.hpp"
#include "src/memory/MemoryManager.hpp"
#include "src/IO/IOManager.hpp"
#include "src/cpu/pcb_loader.hpp"
#include "src/parser_json/parser_json.hpp"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== TESTE SIMPLES DE CORE ===\n\n";
    
    // Setup básico
    MemoryManager memManager(1024, 8192);
    IOManager ioManager;
    
    // Carregar 2 processos apenas
    std::vector<std::unique_ptr<PCB>> processes;
    
    for (int i = 0; i < 2; i++) {
        auto pcb = std::make_unique<PCB>();
        
        if (load_pcb_from_json("process1.json", *pcb)) {
            pcb->pid = i + 1;
            pcb->name = "P" + std::to_string(i + 1);
            pcb->quantum = 50;  // Quantum pequeno para testar preempção
            
            std::cout << "[LOAD] Carregando P" << pcb->pid << "...\n";
            loadJsonProgram("tasks.json", memManager, *pcb, 0);
            std::cout << "[LOAD] P" << pcb->pid << " OK\n";
            processes.push_back(std::move(pcb));
        }
    }
    
    std::cout << "\n[SCHEDULER] Iniciando com 1 núcleo...\n";
    
    try {
        RoundRobinScheduler scheduler(1, &memManager, &ioManager, 50);
        
        for (auto& pcb : processes) {
            scheduler.add_process(pcb.get());
        }
        
        std::cout << "[RUN] Executando...\n";
        
        int cycles = 0;
        int max_cycles = 5000;
        while (cycles < max_cycles && scheduler.has_pending_processes()) {
            scheduler.schedule_cycle();
            cycles++;
            
            if (cycles % 500 == 0) {
                std::cout << "[PROGRESS] Ciclo " << cycles << " / " << max_cycles << "\n";
            }
        }
        
        std::cout << "\n[OK] Execução completa após " << cycles << " ciclos!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "[ERRO] Exception: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "[FIM] Teste concluído com sucesso!\n";
    return 0;
}
