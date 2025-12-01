#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "memory/MemoryManager.hpp"
#include "IO/IOManager.hpp"
#include "cpu/pcb_loader.hpp"
#include "parser_json/parser_json.hpp"
#include "cpu/RoundRobinScheduler.hpp"

int main() {
    const int NUM_CORES = 4;
    const int QUANTUM = 1000;
    
    MemoryManager* memManager = new MemoryManager(4096, 32768);
    IOManager* ioManager = new IOManager();
    
    std::vector<PCB*> process_ptrs;
    for (int i = 0; i < 8; i++) {
        PCB* pcb = new PCB();
        load_pcb_from_json("process1.json", *pcb);
        pcb->pid = i + 1;
        pcb->name = "P" + std::to_string(i + 1);
        pcb->quantum = QUANTUM;
        
        int start_addr = i * 1000;
        int end_addr = loadJsonProgram("tasks.json", *memManager, *pcb, start_addr);
        
        pcb->program_start_addr = start_addr;
        pcb->program_size = end_addr - start_addr;
        
        process_ptrs.push_back(pcb);
    }
    
    RoundRobinScheduler scheduler(NUM_CORES, memManager, ioManager, QUANTUM);
    for (PCB* pcb : process_ptrs) {
        scheduler.add_process(pcb);
    }
    
    std::cout << "━━━ EXECUTANDO ━━━\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    int cycles = 0;
    const int MAX_CYCLES = 100000;
    
    while (scheduler.has_pending_processes() && cycles < MAX_CYCLES) {
        scheduler.schedule_cycle();
        cycles++;
        
        // Debug a cada 1000 ciclos
        if (cycles % 1000 == 0) {
            int f = scheduler.get_finished_count();
            int t = scheduler.get_total_count();
            std::cout << "[DEBUG " << cycles << "] finished=" << f << "/" << t 
                      << " has_pending=" << scheduler.has_pending_processes() << "\n";
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n━━━ RESULTADO ━━━\n";
    std::cout << "Tempo: " << duration.count() << " ms\n";
    std::cout << "Ciclos: " << cycles << "\n";
    std::cout << "Finalizados: " << scheduler.get_finished_count() << "/8\n";
    
    if (cycles < MAX_CYCLES) {
        std::cout << "✅ TERMINOU CORRETAMENTE\n";
    } else {
        std::cout << "⚠️  TIMEOUT (has_pending_processes ficou true indefinidamente)\n";
    }
    
    return 0;
}
