#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include "src/memory/MemoryManager.hpp"
#include "src/IO/IOManager.hpp"
#include "src/cpu/pcb_loader.hpp"
#include "src/parser_json/parser_json.hpp"
#include "src/cpu/RoundRobinScheduler.hpp"

// Variável global para monitorar
std::atomic<int> global_finished{0};

void monitor_thread(const std::vector<PCB*>& processes, bool* keep_running) {
    while (*keep_running) {
        int finished = 0;
        long total_cycles = 0;
        
        for (auto* p : processes) {
            if (p->state == State::Finished) finished++;
            total_cycles += p->pipeline_cycles.load();
        }
        
        global_finished.store(finished);
        
        std::cout << "[MONITOR] " << finished << "/8 finalizados, "
                  << "total_cycles=" << total_cycles << "\n";
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          INSPEÇÃO PROFUNDA DE EXECUÇÃO                     ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    
    const int NUM_PROCESSES = 8;
    const int QUANTUM = 1000;
    const int NUM_CORES = 4;  // Testar com 4 cores que tem CV estranho
    
    MemoryManager* memManager = new MemoryManager(4096, 32768);
    IOManager* ioManager = new IOManager();
    
    std::vector<PCB*> process_ptrs;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        PCB* pcb = new PCB();
        load_pcb_from_json("process1.json", *pcb);
        pcb->pid = i + 1;
        pcb->name = "P" + std::to_string(i + 1);
        pcb->quantum = QUANTUM;
        
        int start_addr = i * 1000;
        int end_addr = loadJsonProgram("tasks.json", *memManager, *pcb, start_addr);
        
        pcb->program_start_addr = start_addr;
        pcb->program_size = end_addr - start_addr;
        
        std::cout << "P" << pcb->pid << ": [" << start_addr << "-" << end_addr 
                  << ") size=" << pcb->program_size << " bytes\n";
        
        process_ptrs.push_back(pcb);
    }
    
    RoundRobinScheduler scheduler(NUM_CORES, memManager, ioManager, QUANTUM);
    for (PCB* pcb : process_ptrs) {
        scheduler.add_process(pcb);
    }
    
    std::cout << "\n━━━ EXECUTANDO COM " << NUM_CORES << " CORES ━━━\n";
    
    // Thread de monitoramento
    bool keep_monitoring = true;
    std::thread monitor(monitor_thread, std::ref(process_ptrs), &keep_monitoring);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    int cycles = 0;
    const int MAX_CYCLES = 1000000;  // Reduzido para não travar
    
    while (scheduler.has_pending_processes() && cycles < MAX_CYCLES) {
        scheduler.schedule_cycle();
        cycles++;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    keep_monitoring = false;
    monitor.join();
    
    std::cout << "\n━━━ ANÁLISE DETALHADA ━━━\n";
    std::cout << "Tempo total: " << duration.count() << " ms\n";
    std::cout << "Ciclos scheduler: " << cycles << "\n\n";
    
    int finished = 0;
    long total_pipeline_cycles = 0;
    
    std::cout << "Estado final de cada processo:\n";
    for (int i = 0; i < NUM_PROCESSES; i++) {
        PCB* p = process_ptrs[i];
        long pc_cycles = p->pipeline_cycles.load();
        total_pipeline_cycles += pc_cycles;
        
        std::string state_str = "?";
        if (p->state == State::Finished) { state_str = "Finished ✓"; finished++; }
        else if (p->state == State::Ready) state_str = "Ready    ✗";
        else if (p->state == State::Running) state_str = "Running  ⚠";
        
        std::cout << "  P" << p->pid << ": " << state_str 
                  << " | pipeline_cycles=" << pc_cycles
                  << " | switches=" << p->context_switches.load()
                  << " | PC=" << p->regBank.pc.value
                  << " | expected_end=" << (p->program_start_addr + p->program_size)
                  << " | ";
        
        if (p->regBank.pc.value >= (p->program_start_addr + p->program_size)) {
            std::cout << "PC FORA DO PROGRAMA!";
        } else if (p->state == State::Finished) {
            std::cout << "OK";
        } else {
            std::cout << "INCOMPLETO";
        }
        std::cout << "\n";
    }
    
    std::cout << "\n━━━ VERIFICAÇÕES ━━━\n";
    
    // 1. Todos terminaram?
    if (finished == NUM_PROCESSES) {
        std::cout << "✅ Todos " << NUM_PROCESSES << " processos finalizaram\n";
    } else {
        std::cout << "❌ Apenas " << finished << "/" << NUM_PROCESSES << " finalizaram\n";
    }
    
    // 2. Executaram trabalho real?
    double avg_cycles = (double)total_pipeline_cycles / NUM_PROCESSES;
    std::cout << "📊 Ciclos médios por processo: " << (long)avg_cycles << "\n";
    if (avg_cycles > 100) {
        std::cout << "✅ Processos executaram trabalho significativo\n";
    } else {
        std::cout << "❌ Processos executaram pouco trabalho (< 100 ciclos)\n";
    }
    
    // 3. Tempo real vs ciclos scheduler
    double scheduler_efficiency = (double)total_pipeline_cycles / cycles * 100.0;
    std::cout << "⚙️  Eficiência scheduler: " << scheduler_efficiency << "% "
              << "(ciclos úteis / ciclos totais)\n";
    
    if (scheduler_efficiency < 0.1) {
        std::cout << "⚠️  PROBLEMA: Scheduler roda " << (100.0/scheduler_efficiency) 
                  << "x mais ciclos que trabalho útil!\n";
        std::cout << "   Causa: Arquitetura async - scheduler polling enquanto threads executam\n";
    }
    
    // 4. Throughput real
    if (duration.count() > 0) {
        double throughput = (double)total_pipeline_cycles / duration.count() * 1000.0;
        std::cout << "🚀 Throughput: " << (long)throughput << " ciclos/segundo\n";
    }
    
    // 5. Timeout?
    if (cycles >= MAX_CYCLES) {
        std::cout << "⚠️  TIMEOUT: Scheduler atingiu MAX_CYCLES!\n";
        std::cout << "   Possível causa: has_pending_processes() retorna true mas nada executa\n";
    }
    
    std::cout << "\n━━━ TESTE COM tasks.json ━━━\n";
    std::cout << "Programa: ~100 instruções por processo\n";
    std::cout << "Quantum: " << QUANTUM << " ciclos\n";
    std::cout << "Esperado: Cada processo termina em 1 quantum (~200-500 ciclos)\n";
    std::cout << "Observado: " << (long)avg_cycles << " ciclos médios\n";
    
    if (avg_cycles >= 200 && avg_cycles <= 2000 && finished == NUM_PROCESSES) {
        std::cout << "\n✅ CONCLUSÃO: Processos ESTÃO executando corretamente!\n";
        std::cout << "   Tempo medido (" << duration.count() << "ms) reflete trabalho REAL\n";
    } else {
        std::cout << "\n❌ CONCLUSÃO: Há problemas na execução!\n";
    }
    
    return 0;
}
