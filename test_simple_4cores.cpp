#include <iostream>
#include <memory>
#include <vector>
#include "src/memory/MemoryManager.hpp"
#include "src/cpu/PCB.hpp"
#include "src/cpu/pcb_loader.hpp"
#include "src/cpu/Core.hpp"
#include "src/cpu/RoundRobinScheduler.hpp"
#include "src/parser_json/parser_json.hpp"
#include "src/IO/IOManager.hpp"

int main() {
    std::cout << "Teste simples com 4 núcleos\n";
    
    MemoryManager memManager(1024, 8192);
    IOManager ioManager;
    RoundRobinScheduler scheduler(4, &memManager, &ioManager, 100);
    
    std::vector<std::unique_ptr<PCB>> processes;
    
    for (int i = 0; i < 3; i++) {
        auto pcb = std::make_unique<PCB>();
        if (load_pcb_from_json("process1.json", *pcb)) {
            pcb->pid = i + 1;
            pcb->name = "P" + std::to_string(i + 1);
            loadJsonProgram("tasks.json", memManager, *pcb, 0);
            scheduler.add_process(pcb.get());
            processes.push_back(std::move(pcb));
        }
    }
    
    std::cout << "Executando scheduling...\n";
    for (int i = 0; i < 100; i++) {
        scheduler.schedule_cycle();
    }
    
    std::cout << "Aguardando threads...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Finalizando...\n";
    return 0;
}
