#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <thread>

#include "../src/cpu/RoundRobinScheduler.hpp"
#include "../src/cpu/FCFSScheduler.hpp"
#include "../src/cpu/SJNScheduler.hpp"
#include "../src/memory/MemoryManager.hpp"
#include "../src/IO/IOManager.hpp"
#include "../src/cpu/pcb_loader.hpp"
#include "../src/parser_json/parser_json.hpp"

int main() {
    std::cout << "=== TESTE DIAGNÓSTICO DE ESCALONADORES ===\n\n";
    
    const int NUM_PROCESSES = 4;  // Usar poucos processos para debug
    const int QUANTUM = 1000;
    const int MAX_CYCLES = 100000;  // Limite baixo para diagnóstico
    
    std::vector<std::string> policies = {"RR", "FCFS"};
    
    for (const auto& policy : policies) {
        std::cout << "\n--- Testando " << policy << " ---\n";
        
        MemoryManager* memManager = new MemoryManager(4096, 32768);
        IOManager* ioManager = new IOManager();
        
        std::vector<PCB*> process_ptrs;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            PCB* pcb = new PCB();
            if (load_pcb_from_json("examples/processes/process1.json", *pcb)) {
                pcb->pid = i + 1;
                pcb->name = "P" + std::to_string(i + 1);
                pcb->quantum = QUANTUM;
                pcb->arrival_time = 0;
                loadJsonProgram("examples/programs/tasks.json", *memManager, *pcb, i * 1024);
                pcb->estimated_job_size = pcb->program_size;
                process_ptrs.push_back(pcb);
            }
        }
        
        std::cout << "Processos carregados: " << process_ptrs.size() << "\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        int cycles = 0;
        int last_report = 0;
        
        if (policy == "RR") {
            RoundRobinScheduler scheduler(2, memManager, ioManager, QUANTUM);
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            while (scheduler.has_pending_processes() && cycles < MAX_CYCLES) {
                scheduler.schedule_cycle();
                cycles++;
                
                // Report a cada 10000 ciclos
                if (cycles - last_report >= 10000) {
                    std::cout << "  [" << policy << "] Ciclos: " << cycles 
                              << ", Finalizados: " << scheduler.get_finished_count() << "\n";
                    last_report = cycles;
                }
            }
            std::cout << "  [" << policy << "] TOTAL: " << cycles << " ciclos, " 
                      << scheduler.get_finished_count() << " finalizados\n";
            
        } else if (policy == "FCFS") {
            FCFSScheduler scheduler(2, memManager, ioManager);
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            while (!scheduler.all_finished() && cycles < MAX_CYCLES) {
                scheduler.schedule_cycle();
                cycles++;
                
                if (cycles - last_report >= 10000) {
                    // Verificar estado dos cores
                    auto& cores = scheduler.get_cores();
                    auto& ready = scheduler.get_ready_queue();
                    auto& blocked = scheduler.get_blocked_list();
                    
                    std::cout << "  [" << policy << "] Ciclos: " << cycles 
                              << ", Ready: " << ready.size()
                              << ", Blocked: " << blocked.size()
                              << ", Cores: ";
                    for (auto& core : cores) {
                        std::cout << (core->is_idle() ? "I" : "B");
                        if (core->get_current_process()) {
                            std::cout << "(P" << core->get_current_process()->pid << ")";
                        }
                    }
                    std::cout << "\n";
                    last_report = cycles;
                }
            }
            std::cout << "  [" << policy << "] TOTAL: " << cycles << " ciclos\n";
            
            // Verificar estado final
            auto& cores = scheduler.get_cores();
            std::cout << "  Estado final dos cores: ";
            for (auto& core : cores) {
                std::cout << (core->is_idle() ? "IDLE" : "BUSY");
                if (core->get_current_process()) {
                    std::cout << "(P" << core->get_current_process()->pid << ")";
                }
                std::cout << " ";
            }
            std::cout << "\n";
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "  Tempo real: " << duration.count() << " ms\n";
        
        delete memManager;
        delete ioManager;
    }
    
    return 0;
}
