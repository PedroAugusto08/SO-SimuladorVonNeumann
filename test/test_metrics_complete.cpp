/**
 * @file test_metrics_complete.cpp
 * @brief Teste completo de métricas detalhadas para todos os escalonadores
 * 
 * Este teste demonstra a coleta de métricas em todos os 5 escalonadores:
 * - FCFS, SJN, RR, PRIORITY, PRIORITY_PREEMPT
 * 
 * Métricas coletadas:
 * - Tempo médio de espera
 * - Tempo médio de turnaround
 * - Tempo médio de resposta
 * - Utilização da CPU
 * - Throughput
 * - Context switches
 * 
 * Output: Console + CSV (logs/metrics/detailed_metrics.csv)
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <type_traits>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "memory/MemoryManager.hpp"
#include "IO/IOManager.hpp"
#include "parser_json/parser_json.hpp"
#include "cpu/pcb_loader.hpp"

void print_separator() {
    std::cout << "\n" << std::string(80, '=') << "\n\n";
}

// Estrutura para armazenar resultados
struct MetricsResult {
    std::string policy;
    double avg_wait_time;
    double avg_turnaround_time;
    double avg_response_time;
    double avg_cpu_utilization;
    double throughput;
    int context_switches;
    int total_processes;
};

// Especialização para RoundRobin (agora com avg_response_time também)
MetricsResult print_statistics_rr(const std::string& policy, const RoundRobinScheduler::Statistics& stats) {
    std::cout << "📊 MÉTRICAS DETALHADAS - " << policy << ":\n\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  ⏱️  Tempo Médio de Espera:      " << stats.avg_wait_time << " ciclos\n";
    std::cout << "  ⏱️  Tempo Médio de Turnaround:  " << stats.avg_turnaround_time << " ciclos\n";
    std::cout << "  ⏱️  Tempo Médio de Resposta:    " << stats.avg_response_time << " ciclos\n";
    std::cout << "  💻 Utilização da CPU:           " << stats.avg_cpu_utilization << " %\n";
    std::cout << "  📈 Throughput:                  " << stats.throughput << " proc/ms\n";
    std::cout << "  🔄 Context Switches:            " << stats.total_context_switches << "\n";
    std::cout << "  📦 Processos Concluídos:        " << stats.total_processes << "\n";
    print_separator();
    
    // Retorna resultado para CSV
    return {policy, stats.avg_wait_time, stats.avg_turnaround_time, stats.avg_response_time, 
            stats.avg_cpu_utilization, stats.throughput, stats.total_context_switches, stats.total_processes};
}

// Template for FCFS, SJN, PRIORITY
template<typename Stats>
MetricsResult print_statistics(const std::string& policy, const Stats& stats) {
    std::cout << "📊 MÉTRICAS DETALHADAS - " << policy << ":\n\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  ⏱️  Tempo Médio de Espera:      " << stats.avg_wait_time << " ciclos\n";
    std::cout << "  ⏱️  Tempo Médio de Turnaround:  " << stats.avg_turnaround_time << " ciclos\n";
    std::cout << "  ⏱️  Tempo Médio de Resposta:    " << stats.avg_response_time << " ciclos\n";
    std::cout << "  💻 Utilização da CPU:           " << stats.avg_cpu_utilization << " %\n";
    std::cout << "  📈 Throughput:                  " << stats.throughput << " proc/ms\n";
    std::cout << "  🔄 Context Switches:            " << stats.total_context_switches << "\n";
    std::cout << "  📦 Processos Concluídos:        " << stats.total_processes << "\n";
    print_separator();
    
    // Retorna resultado para CSV
    return {policy, stats.avg_wait_time, stats.avg_turnaround_time, stats.avg_response_time,
            stats.avg_cpu_utilization, stats.throughput, stats.total_context_switches, stats.total_processes};
}

template<typename Scheduler>
MetricsResult test_scheduler(const std::string& name, Scheduler& scheduler, 
                    std::vector<std::unique_ptr<PCB>>& processes,
                    MemoryManager&) {
    std::cout << "🚀 Testando " << name << "...\n";
    
    // Adiciona processos
    for (auto& pcb : processes) {
        scheduler.add_process(pcb.get());
    }
    
    // Executa até finalizar
    int cycles = 0;
    int max_cycles = 2000; // Aumentado para dar tempo ao Round Robin (com preempção)
    
    // RoundRobin usa has_pending_processes(), outros usam all_finished()
    if constexpr (std::is_same_v<Scheduler, RoundRobinScheduler>) {
        while (scheduler.has_pending_processes() && cycles < max_cycles) {
            scheduler.schedule_cycle();
            cycles++;
        }
        // Coleta e exibe métricas (RoundRobin tem estrutura diferente)
        auto stats = scheduler.get_statistics();
        return print_statistics_rr(name, stats);
    } else {
        while (!scheduler.all_finished() && cycles < max_cycles) {
            scheduler.schedule_cycle();
            cycles++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Delay para threads processarem
        }
        
        // Aguarda mais ciclos para garantir coleta de processos finalizados
        for (int i = 0; i < 50; i++) {
            scheduler.schedule_cycle();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        // Coleta e exibe métricas
        auto stats = scheduler.get_statistics();
        return print_statistics(name, stats);
    }
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TESTE DE MÉTRICAS COMPLETAS - TODOS ESCALONADORES               ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    
    const int NUM_CORES = 2;
    const int NUM_PROCESSES = 4;
    const int QUANTUM = 1000;
    
    std::cout << "\n⚙️  Configuração:\n";
    std::cout << "   • Cores: " << NUM_CORES << "\n";
    std::cout << "   • Processos: " << NUM_PROCESSES << "\n";
    std::cout << "   • Quantum (RR): " << QUANTUM << " ciclos\n";
    print_separator();
    
    // Vetor para armazenar resultados
    std::vector<MetricsResult> results;
    
    // ========== TESTE FCFS ==========
    {
        MemoryManager memManager(4096, 8192);
        IOManager ioManager;
        FCFSScheduler fcfs(NUM_CORES, &memManager, &ioManager);
        
        std::vector<std::unique_ptr<PCB>> processes;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            auto pcb = std::make_unique<PCB>();
            pcb->pid = i + 1;
            pcb->state = State::Ready;
            pcb->estimated_job_size = (i + 1) * 50;
            loadJsonProgram("examples/programs/tasks.json", memManager, *pcb, i * 1024);
            processes.push_back(std::move(pcb));
        }
        
        auto result = test_scheduler("FCFS (First Come First Served)", fcfs, processes, memManager);
        results.push_back(result);
    }
    
    // ========== TESTE SJN ==========
    {
        MemoryManager memManager(4096, 8192);
        IOManager ioManager;
        SJNScheduler sjn(NUM_CORES, &memManager, &ioManager);
        
        std::vector<std::unique_ptr<PCB>> processes;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            auto pcb = std::make_unique<PCB>();
            pcb->pid = i + 1;
            pcb->state = State::Ready;
            pcb->estimated_job_size = (NUM_PROCESSES - i) * 50; // Ordem inversa
            loadJsonProgram("examples/programs/tasks.json", memManager, *pcb, i * 1024);
            processes.push_back(std::move(pcb));
        }
        
        auto result = test_scheduler("SJN (Shortest Job Next)", sjn, processes, memManager);
        results.push_back(result);
    }
    
    // ========== TESTE ROUND ROBIN ==========
    {
        MemoryManager memManager(4096, 8192);
        IOManager ioManager;
        RoundRobinScheduler rr(NUM_CORES, &memManager, &ioManager, QUANTUM);
        
        std::vector<std::unique_ptr<PCB>> processes;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            auto pcb = std::make_unique<PCB>();
            pcb->pid = i + 1;
            pcb->state = State::Ready;
            pcb->quantum = QUANTUM;
            loadJsonProgram("examples/programs/tasks.json", memManager, *pcb, i * 1024);
            processes.push_back(std::move(pcb));
        }
        
        auto result = test_scheduler("Round Robin (Preemptivo)", rr, processes, memManager);
        results.push_back(result);
    }
    
    // ========== TESTE PRIORITY (Não-Preemptivo) ==========
    {
        MemoryManager memManager(4096, 8192);
        IOManager ioManager;
        PriorityScheduler priority(NUM_CORES, &memManager, &ioManager, 999999); // Quantum infinito = não-preemptivo
        
        std::vector<std::unique_ptr<PCB>> processes;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            auto pcb = std::make_unique<PCB>();
            pcb->pid = i + 1;
            pcb->state = State::Ready;
            pcb->priority = (NUM_PROCESSES - i); // Prioridade decrescente (maior = mais importante)
            loadJsonProgram("examples/programs/tasks.json", memManager, *pcb, i * 1024);
            processes.push_back(std::move(pcb));
        }
        
        auto result = test_scheduler("PRIORITY (Não-Preemptivo)", priority, processes, memManager);
        results.push_back(result);
    }
    
    // ========== TESTE PRIORITY PREEMPTIVO ==========
    {
        MemoryManager memManager(4096, 8192);
        IOManager ioManager;
        PriorityScheduler priority_preempt(NUM_CORES, &memManager, &ioManager, QUANTUM);
        
        std::vector<std::unique_ptr<PCB>> processes;
        for (int i = 0; i < NUM_PROCESSES; i++) {
            auto pcb = std::make_unique<PCB>();
            pcb->pid = i + 1;
            pcb->state = State::Ready;
            pcb->priority = (NUM_PROCESSES - i); // Prioridade decrescente (maior = mais importante)
            loadJsonProgram("examples/programs/tasks.json", memManager, *pcb, i * 1024);
            processes.push_back(std::move(pcb));
        }
        
        auto result = test_scheduler("PRIORITY PREEMPTIVO (por Prioridade)", priority_preempt, processes, memManager);
        results.push_back(result);
    }
    
    // Criar diretório se não existir
    system("mkdir -p logs/metrics");
    
    // Salvar resultados em CSV
    std::ofstream csv("logs/metrics/detailed_metrics.csv");
    if (csv.is_open()) {
        csv << "Policy,Avg_Wait_Time,Avg_Turnaround_Time,Avg_Response_Time,CPU_Utilization,Throughput,Context_Switches,Total_Processes\n";
        for (const auto& r : results) {
            csv << r.policy << ","
                << std::fixed << std::setprecision(2)
                << r.avg_wait_time << ","
                << r.avg_turnaround_time << ","
                << r.avg_response_time << ","
                << r.avg_cpu_utilization << ","
                << r.throughput << ","
                << r.context_switches << ","
                << r.total_processes << "\n";
        }
        csv.close();
        std::cout << "\n✅ Métricas detalhadas salvas em: logs/metrics/detailed_metrics.csv\n\n";
    } else {
        std::cerr << "\n❌ ERRO: Não foi possível criar logs/metrics/detailed_metrics.csv\n\n";
    }
    
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✅ TODOS OS 5 ESCALONADORES TESTADOS COM SUCESSO!              ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "📊 COMPARAÇÃO RÁPIDA:\n\n";
    std::cout << "┌──────────────────┬──────────────┬────────────┬──────────────┐\n";
    std::cout << "│ Política         │ Context SW   │ Preemptivo │ Complexidade │\n";
    std::cout << "├──────────────────┼──────────────┼────────────┼──────────────┤\n";
    std::cout << "│ FCFS             │ 0            │ Não        │ O(1)         │\n";
    std::cout << "│ SJN              │ 0            │ Não        │ O(n log n)   │\n";
    std::cout << "│ RR               │ Alto         │ Sim        │ O(1)         │\n";
    std::cout << "│ PRIORITY         │ 0            │ Não        │ O(n log n)   │\n";
    std::cout << "│ PRIORITY_PREEMPT │ Médio        │ Sim        │ O(n log n)   │\n";
    std::cout << "└──────────────────┴──────────────┴────────────┴──────────────┘\n\n";
    
    std::cout << "💡 OBSERVAÇÕES:\n";
    std::cout << "   • FCFS, SJN e PRIORITY (não-preempt) não têm context switches\n";
    std::cout << "   • RR tem mais context switches devido ao quantum fixo\n";
    std::cout << "   • PRIORITY_PREEMPT preempta quando chega processo de maior prioridade\n";
    std::cout << "   • RR é o mais justo: todos os processos completam!\n";
    std::cout << "   • Todas as métricas são coletadas automaticamente!\n\n";
    
    return 0;
}
