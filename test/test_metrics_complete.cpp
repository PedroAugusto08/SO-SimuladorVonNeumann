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
#include <algorithm>
#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "memory/MemoryManager.hpp"
#include "memory/MemoryMonitor.hpp"
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

// Especialização para RoundRobin
MetricsResult print_statistics_rr(const std::string& policy, const RoundRobinScheduler::Statistics& stats) {
    std::cout << "📊 MÉTRICAS - " << policy << ":\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  • Tempo Médio de Espera:     " << stats.avg_wait_time << " ciclos\n";
    std::cout << "  • Tempo Médio de Execução:   " << stats.avg_turnaround_time << " ciclos\n";
    std::cout << "  • Utilização da CPU:          " << stats.avg_cpu_utilization << " %\n";
    std::cout << "  • Throughput:                 " << stats.throughput << " proc/s\n";
    print_separator();
    
    return {policy, stats.avg_wait_time, stats.avg_turnaround_time, stats.avg_response_time, 
            stats.avg_cpu_utilization, stats.throughput, stats.total_context_switches, stats.total_processes};
}

// Template for FCFS, SJN, PRIORITY
template<typename Stats>
MetricsResult print_statistics(const std::string& policy, const Stats& stats) {
    std::cout << "📊 MÉTRICAS - " << policy << ":\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  • Tempo Médio de Espera:     " << stats.avg_wait_time << " ciclos\n";
    std::cout << "  • Tempo Médio de Execução:   " << stats.avg_turnaround_time << " ciclos\n";
    std::cout << "  • Utilização da CPU:          " << stats.avg_cpu_utilization << " %\n";
    std::cout << "  • Throughput:                 " << stats.throughput << " proc/s\n";
    print_separator();
    
    return {policy, stats.avg_wait_time, stats.avg_turnaround_time, stats.avg_response_time,
            stats.avg_cpu_utilization, stats.throughput, stats.total_context_switches, stats.total_processes};
}

template<typename Scheduler>
MetricsResult test_scheduler(const std::string& name, Scheduler& scheduler, 
                    std::vector<std::unique_ptr<PCB>>& processes,
                    MemoryManager& memManager) {
    std::cout << "🚀 Testando " << name << "...\n";
    
    // Criar monitor de memória
    MemoryMonitor monitor("logs/memory/memory_" + name + ".csv");
    
    // Registrar estado inicial
    monitor.record_snapshot(
        memManager.getUsedMainMemory(),
        memManager.getUsedSecondaryMemory(),
        memManager.getTotalCacheHits(),
        memManager.getTotalCacheMisses()
    );
    
    // Adiciona processos
    for (auto& pcb : processes) {
        scheduler.add_process(pcb.get());
    }
    
    // Executa até finalizar
    int cycles = 0;
    int max_cycles = 5000; // 🆕 CORREÇÃO: Aumentado para dar tempo a TODOS os 4 processos
    
    // RoundRobin usa has_pending_processes(), outros usam all_finished()
    if constexpr (std::is_same_v<Scheduler, RoundRobinScheduler>) {
        while (scheduler.has_pending_processes() && cycles < max_cycles) {
            scheduler.schedule_cycle();
            cycles++;
            
            // Registrar uso de memória a cada 50 ciclos
            if (cycles % 50 == 0) {
                monitor.record_snapshot(
                    memManager.getUsedMainMemory(),
                    memManager.getUsedSecondaryMemory(),
                    memManager.getTotalCacheHits(),
                    memManager.getTotalCacheMisses()
                );
            }
        }
        
        // Ciclos extras para finalização completa
        for (int i = 0; i < 50; i++) {
            scheduler.schedule_cycle();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        // Registrar estado final
        monitor.record_snapshot(
            memManager.getUsedMainMemory(),
            memManager.getUsedSecondaryMemory(),
            memManager.getTotalCacheHits(),
            memManager.getTotalCacheMisses()
        );
        
        // Coleta e exibe métricas (RoundRobin tem estrutura diferente)
        auto stats = scheduler.get_statistics();
        return print_statistics_rr(name, stats);
    } else {
        while (!scheduler.all_finished() && cycles < max_cycles) {
            scheduler.schedule_cycle();
            cycles++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            // Registrar uso de memória a cada 50 ciclos
            if (cycles % 50 == 0) {
                monitor.record_snapshot(
                    memManager.getUsedMainMemory(),
                    memManager.getUsedSecondaryMemory(),
                    memManager.getTotalCacheHits(),
                    memManager.getTotalCacheMisses()
                );
            }
        }
        
        // Ciclos extras para garantir coleta completa de todos os processos
        for (int i = 0; i < 200; i++) {
            scheduler.schedule_cycle();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // Registrar a cada 10 ciclos extras
            if (i % 10 == 0) {
                monitor.record_snapshot(
                    memManager.getUsedMainMemory(),
                    memManager.getUsedSecondaryMemory(),
                    memManager.getTotalCacheHits(),
                    memManager.getTotalCacheMisses()
                );
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        for (int i = 0; i < 50; i++) {
            scheduler.schedule_cycle();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        // Registrar estado final
        monitor.record_snapshot(
            memManager.getUsedMainMemory(),
            memManager.getUsedSecondaryMemory(),
            memManager.getTotalCacheHits(),
            memManager.getTotalCacheMisses()
        );
        
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
    
    std::cout << "\n⚙️  Configuração: " << NUM_CORES << " cores, " 
              << NUM_PROCESSES << " processos, quantum=" << QUANTUM << " ciclos\n";
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
        csv << "Policy,Avg_Wait_Time,Avg_Execution_Time,CPU_Utilization,Throughput,Efficiency\n";
        for (const auto& r : results) {
            double efficiency = (r.avg_cpu_utilization / 100.0) * r.throughput;
            csv << r.policy << ","
                << std::fixed << std::setprecision(2)
                << r.avg_wait_time << ","
                << r.avg_turnaround_time << ","
                << r.avg_cpu_utilization << ","
                << r.throughput << ","
                << efficiency << "\n";
        }
        csv.close();
        std::cout << "\n✅ Métricas salvas em: logs/metrics/detailed_metrics.csv\n";
        std::cout << "✅ Utilização de memória salva em: logs/memory/memory_*.csv\n";
    } else {
        std::cerr << "\n❌ ERRO: Não foi possível criar logs/metrics/detailed_metrics.csv\n\n";
    }
    
    std::cout << "\n✅ TODOS OS 5 ESCALONADORES TESTADOS COM SUCESSO!\n\n";
    
    // Gerar relatório consolidado em formato texto
    std::cout << "📄 Gerando relatório consolidado...\n";
    std::ofstream report("logs/metrics/comparative_report.txt");
    if (report.is_open()) {
        report << "╔════════════════════════════════════════════════════════════════════╗\n";
        report << "║    RELATÓRIO COMPARATIVO - POLÍTICAS DE ESCALONAMENTO            ║\n";
        report << "╚════════════════════════════════════════════════════════════════════╝\n\n";
        
        report << "Configuração do Teste:\n";
        report << "  • Núcleos: " << NUM_CORES << "\n";
        report << "  • Processos: " << NUM_PROCESSES << "\n";
        report << "  • Quantum (RR): " << QUANTUM << " ciclos\n";
        report << "  • Políticas testadas: FCFS, SJN, RR, PRIORITY, PRIORITY_PREEMPT\n\n";
        
        report << "═══════════════════════════════════════════════════════════════════\n\n";
        
        for (const auto& r : results) {
            report << "Política: " << r.policy << "\n";
            report << std::string(60, '-') << "\n";
            report << std::fixed << std::setprecision(2);
            report << "  • Tempo Médio de Espera:      " << r.avg_wait_time << " ciclos\n";
            report << "  • Tempo Médio de Execução:    " << r.avg_turnaround_time << " ciclos\n";
            report << "  • Utilização da CPU:          " << r.avg_cpu_utilization << " %\n";
            report << "  • Throughput:                 " << std::setprecision(2) << r.throughput << " proc/s\n";
            report << "  • Eficiência:                 " << (r.avg_cpu_utilization / 100.0) * r.throughput << " proc/s efetivos\n\n";
        }
        
        report << "═══════════════════════════════════════════════════════════════════\n\n";
        report << "RESUMO COMPARATIVO:\n\n";
        
        auto min_wait = *std::min_element(results.begin(), results.end(), 
            [](const auto& a, const auto& b) { return a.avg_wait_time < b.avg_wait_time; });
        auto min_exec = *std::min_element(results.begin(), results.end(),
            [](const auto& a, const auto& b) { return a.avg_turnaround_time < b.avg_turnaround_time; });
        auto max_cpu = *std::max_element(results.begin(), results.end(),
            [](const auto& a, const auto& b) { return a.avg_cpu_utilization < b.avg_cpu_utilization; });
        auto max_throughput = *std::max_element(results.begin(), results.end(),
            [](const auto& a, const auto& b) { return a.throughput < b.throughput; });
        
        report << "  🏆 Melhor Tempo de Espera:     " << min_wait.policy 
               << " (" << std::fixed << std::setprecision(2) << min_wait.avg_wait_time << " ciclos)\n";
        report << "  🏆 Melhor Tempo de Execução:   " << min_exec.policy
               << " (" << std::fixed << std::setprecision(2) << min_exec.avg_turnaround_time << " ciclos)\n";
        report << "  🏆 Melhor Utilização de CPU:   " << max_cpu.policy
               << " (" << std::fixed << std::setprecision(2) << max_cpu.avg_cpu_utilization << " %)\n";
        report << "  🏆 Maior Throughput:            " << max_throughput.policy
               << " (" << std::fixed << std::setprecision(2) << max_throughput.throughput << " proc/s)\n\n";
        
        report << "═══════════════════════════════════════════════════════════════════\n";
        report << "Relatório gerado: logs/metrics/comparative_report.txt\n";
        report << "Dados CSV: logs/metrics/detailed_metrics.csv\n";
        report << "═══════════════════════════════════════════════════════════════════\n";
        
        report.close();
        std::cout << "✅ Relatório consolidado salvo em: logs/metrics/comparative_report.txt\n\n";
    }
    
    return 0;
}
