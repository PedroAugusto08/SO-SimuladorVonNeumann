/**
 * @file test_complete_unified.cpp
 * @brief Teste UNIFICADO e COMPLETO - Multicore + Métricas + Cache por configuração
 * 
 * Este teste combina test_multicore_comparative e test_metrics_complete para gerar
 * dados completos que permitem comparar QUALQUER métrica vs QUALQUER outra na GUI.
 * 
 * Gera dados de:
 * - Tempo de execução por cores (1, 2, 4, 6)
 * - Speedup e Eficiência
 * - Cache Hits/Misses por cores
 * - Tempo de Espera, Turnaround, CPU Utilization, Throughput por cores
 * 
 * Output:
 * - dados_graficos/unified_complete.csv (TODOS os dados em um único arquivo)
 * - dados_graficos/escalonadores_multicore.csv (compatibilidade)
 * - dados_graficos/metricas_escalonadores.csv (compatibilidade)
 * - dados_graficos/memoria_*.csv (por política e cores)
 * 
 * @author Grupo Peripherals
 * @date 2024
 * @version 1.0 - Teste unificado completo
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <map>
#include <sstream>
#include "memory/MemoryManager.hpp"
#include "memory/MemoryMonitor.hpp"
#include "cpu/PCB.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/Core.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

// Estrutura completa de resultado
struct UnifiedResult {
    std::string policy;
    int num_cores;
    
    // Tempo e escalabilidade
    double execution_time_ms;
    double speedup;
    double efficiency_pct;
    double cv_percent;
    
    // Métricas de escalonamento
    double avg_wait_time;
    double avg_turnaround_time;
    double cpu_utilization_pct;
    double throughput;
    
    // Métricas de memória/cache
    long cache_hits;
    long cache_misses;
    double hit_rate_pct;
    
    int processes_finished;
};

class SilentMode {
private:
    std::streambuf* old_cout;
    std::ofstream null_stream;
public:
    SilentMode() : old_cout(nullptr) {
#ifdef _WIN32
        null_stream.open("NUL");
#else
        null_stream.open("/dev/null");
#endif
        if (null_stream.is_open()) {
            old_cout = std::cout.rdbuf();
            std::cout.rdbuf(null_stream.rdbuf());
        }
    }
    ~SilentMode() {
        if (old_cout) std::cout.rdbuf(old_cout);
        if (null_stream.is_open()) null_stream.close();
    }
};

UnifiedResult run_unified_test(const std::string& policy, int num_cores, int num_processes, 
                                int quantum, int max_cycles, const std::string& tasks_file,
                                bool collect_detailed_metrics = true) {
    UnifiedResult result;
    result.policy = policy;
    result.num_cores = num_cores;
    result.processes_finished = 0;
    result.avg_wait_time = 0;
    result.avg_turnaround_time = 0;
    result.cpu_utilization_pct = 0;
    result.throughput = 0;
    result.cache_hits = 0;
    result.cache_misses = 0;
    result.hit_rate_pct = 0;
    
    std::vector<PCB*> process_ptrs;
    
    try {
        SilentMode silent;
        
        // Reset stats de memória
        MemoryManager::resetStats();
        
        MemoryManager* memManager = new MemoryManager(4096, 32768);
        IOManager* ioManager = new IOManager();
        
        // Carregar processos
        for (int i = 0; i < num_processes; i++) {
            PCB* pcb = new PCB();
            if (load_pcb_from_json("examples/processes/process1.json", *pcb)) {
                pcb->pid = i + 1;
                pcb->name = "P" + std::to_string(i + 1);
                pcb->quantum = quantum;
                pcb->arrival_time = 0;
                pcb->state = State::Ready;
                loadJsonProgram(tasks_file, *memManager, *pcb, i * 1024);
                pcb->estimated_job_size = pcb->program_size;
                
                // Prioridades variadas para PRIORITY
                pcb->priority = 10 - (i % 3) * 3;  // 10, 7, 4, 10, ...
                
                process_ptrs.push_back(pcb);
            } else {
                delete pcb;
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        if (policy == "RR") {
            RoundRobinScheduler scheduler(num_cores, memManager, ioManager, quantum);
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            int cycles = 0;
            while (scheduler.has_pending_processes() && cycles < max_cycles) {
                scheduler.schedule_cycle();
                cycles++;
            }
            
            // Ciclos extras para finalização
            for (int i = 0; i < 100; i++) {
                scheduler.schedule_cycle();
            }
            
            result.processes_finished = scheduler.get_finished_count();
            
            // Coletar métricas detalhadas
            if (collect_detailed_metrics) {
                auto stats = scheduler.get_statistics();
                result.avg_wait_time = stats.avg_wait_time;
                result.avg_turnaround_time = stats.avg_turnaround_time;
                result.cpu_utilization_pct = stats.avg_cpu_utilization;
                result.throughput = stats.throughput;
            }
            
        } else if (policy == "FCFS") {
            FCFSScheduler scheduler(num_cores, memManager, ioManager);
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            int cycles = 0;
            while (!scheduler.all_finished() && cycles < max_cycles) {
                scheduler.schedule_cycle();
                cycles++;
            }
            
            for (int i = 0; i < 200; i++) {
                scheduler.schedule_cycle();
            }
            
            result.processes_finished = process_ptrs.size();
            
            if (collect_detailed_metrics) {
                auto stats = scheduler.get_statistics();
                result.avg_wait_time = stats.avg_wait_time;
                result.avg_turnaround_time = stats.avg_turnaround_time;
                result.cpu_utilization_pct = stats.avg_cpu_utilization;
                result.throughput = stats.throughput;
            }
            
        } else if (policy == "SJN") {
            SJNScheduler scheduler(num_cores, memManager, ioManager);
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            int cycles = 0;
            while (!scheduler.all_finished() && cycles < max_cycles) {
                scheduler.schedule_cycle();
                cycles++;
            }
            
            for (int i = 0; i < 200; i++) {
                scheduler.schedule_cycle();
            }
            
            result.processes_finished = process_ptrs.size();
            
            if (collect_detailed_metrics) {
                auto stats = scheduler.get_statistics();
                result.avg_wait_time = stats.avg_wait_time;
                result.avg_turnaround_time = stats.avg_turnaround_time;
                result.cpu_utilization_pct = stats.avg_cpu_utilization;
                result.throughput = stats.throughput;
            }
            
        } else if (policy == "PRIORITY") {
            PriorityScheduler scheduler(num_cores, memManager, ioManager);
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            int cycles = 0;
            while (!scheduler.all_finished() && cycles < max_cycles) {
                scheduler.schedule_cycle();
                cycles++;
            }
            
            for (int i = 0; i < 200; i++) {
                scheduler.schedule_cycle();
            }
            
            result.processes_finished = scheduler.get_finished_count();
            
            if (collect_detailed_metrics) {
                auto stats = scheduler.get_statistics();
                result.avg_wait_time = stats.avg_wait_time;
                result.avg_turnaround_time = stats.avg_turnaround_time;
                result.cpu_utilization_pct = stats.avg_cpu_utilization;
                result.throughput = stats.throughput;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        result.execution_time_ms = duration.count();
        
        // Sleep APÓS a medição para estabilidade entre iterações
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Coletar métricas de cache
        result.cache_hits = memManager->getTotalCacheHits();
        result.cache_misses = memManager->getTotalCacheMisses();
        long total_accesses = result.cache_hits + result.cache_misses;
        result.hit_rate_pct = (total_accesses > 0) ? 
            (result.cache_hits * 100.0 / total_accesses) : 0.0;
        
        // Cleanup
        delete memManager;
        delete ioManager;
        for (PCB* pcb : process_ptrs) {
            delete pcb;
        }
        
    } catch (const std::exception& e) {
        result.execution_time_ms = 0.0;
    } catch (...) {
        result.execution_time_ms = 0.0;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    return result;
}

int main() {
    const int NUM_PROCESSES = 8;
    const int QUANTUM = 1000;
    const int MAX_CYCLES = 10000000;
    const std::string TASKS_FILE = "examples/programs/tasks.json";
    const int ITERATIONS = 3;
    const int WARMUP_ITERATIONS = 1;
    
    std::vector<std::string> policies = {"RR", "FCFS", "SJN", "PRIORITY"};
    std::vector<int> core_configs = {1, 2, 4, 6};
    
    // Mapa: policy -> cores -> resultado
    std::map<std::string, std::map<int, UnifiedResult>> all_results;
    
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     TESTE UNIFICADO COMPLETO - MULTICORE + MÉTRICAS + CACHE             ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Configuração:\n";
    std::cout << "  • Políticas: RR, FCFS, SJN, PRIORITY\n";
    std::cout << "  • Cores: 1, 2, 4, 6\n";
    std::cout << "  • Processos: " << NUM_PROCESSES << "\n";
    std::cout << "  • Quantum (RR): " << QUANTUM << " ciclos\n";
    std::cout << "  • Iterações: " << ITERATIONS << " (após " << WARMUP_ITERATIONS << " warm-ups)\n";
    std::cout << "  • Workload: " << TASKS_FILE << "\n\n";
    
    std::cout << "Este teste gera dados completos para a GUI permitindo comparar:\n";
    std::cout << "  ✓ Número de Cores vs Cache Hits\n";
    std::cout << "  ✓ Número de Cores vs Throughput\n";
    std::cout << "  ✓ Tempo de Execução vs CPU Utilization\n";
    std::cout << "  ✓ Qualquer combinação de métricas!\n\n";
    
    // Executar testes para cada política e configuração de cores
    for (const auto& policy : policies) {
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  Testando política: " << policy;
        if (policy == "RR") std::cout << " (Round Robin - Preemptivo)";
        else if (policy == "FCFS") std::cout << " (First Come First Served)";
        else if (policy == "SJN") std::cout << " (Shortest Job Next)";
        else if (policy == "PRIORITY") std::cout << " (Priority - Não Preemptivo)";
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        for (int num_cores : core_configs) {
            std::cout << "  ► " << num_cores << " core(s): ";
            std::cout.flush();
            
            std::vector<UnifiedResult> iteration_results;
            
            // Warm-up
            for (int w = 0; w < WARMUP_ITERATIONS; w++) {
                std::cout << "W" << std::flush;
                run_unified_test(policy, num_cores, NUM_PROCESSES, QUANTUM, MAX_CYCLES, TASKS_FILE, false);
            }
            std::cout << " ";
            
            // Iterações válidas
            for (int i = 0; i < ITERATIONS; i++) {
                std::cout << "." << std::flush;
                UnifiedResult result = run_unified_test(policy, num_cores, NUM_PROCESSES, 
                                                        QUANTUM, MAX_CYCLES, TASKS_FILE, true);
                if (result.execution_time_ms > 0) {
                    iteration_results.push_back(result);
                }
            }
            
            if (iteration_results.empty()) {
                std::cout << " ✗ Falhou\n";
                continue;
            }
            
            // Calcular médias
            UnifiedResult final_result;
            final_result.policy = policy;
            final_result.num_cores = num_cores;
            final_result.processes_finished = NUM_PROCESSES;
            
            double sum_time = 0, sum_wait = 0, sum_turn = 0, sum_cpu = 0, sum_thr = 0;
            long sum_hits = 0, sum_misses = 0;
            
            for (const auto& r : iteration_results) {
                sum_time += r.execution_time_ms;
                sum_wait += r.avg_wait_time;
                sum_turn += r.avg_turnaround_time;
                sum_cpu += r.cpu_utilization_pct;
                sum_thr += r.throughput;
                sum_hits += r.cache_hits;
                sum_misses += r.cache_misses;
            }
            
            int n = iteration_results.size();
            final_result.execution_time_ms = sum_time / n;
            final_result.avg_wait_time = sum_wait / n;
            final_result.avg_turnaround_time = sum_turn / n;
            final_result.cpu_utilization_pct = sum_cpu / n;
            final_result.throughput = sum_thr / n;
            final_result.cache_hits = sum_hits / n;
            final_result.cache_misses = sum_misses / n;
            
            long total_access = final_result.cache_hits + final_result.cache_misses;
            final_result.hit_rate_pct = (total_access > 0) ? 
                (final_result.cache_hits * 100.0 / total_access) : 0.0;
            
            // Calcular CV
            double variance = 0.0;
            for (const auto& r : iteration_results) {
                variance += (r.execution_time_ms - final_result.execution_time_ms) * 
                            (r.execution_time_ms - final_result.execution_time_ms);
            }
            double stddev = std::sqrt(variance / n);
            final_result.cv_percent = (final_result.execution_time_ms > 0) ? 
                (stddev / final_result.execution_time_ms * 100.0) : 0.0;
            
            all_results[policy][num_cores] = final_result;
            
            std::cout << " ✓ " << std::fixed << std::setprecision(1) 
                      << final_result.execution_time_ms << "ms"
                      << " | Cache: " << final_result.cache_hits << "/" << final_result.cache_misses
                      << " (" << std::setprecision(1) << final_result.hit_rate_pct << "%)"
                      << " | CPU: " << std::setprecision(1) << final_result.cpu_utilization_pct << "%"
                      << "\n";
        }
    }
    
    // Calcular speedup e eficiência
    for (auto& [policy, core_results] : all_results) {
        if (core_results.count(1) > 0) {
            double baseline_time = core_results[1].execution_time_ms;
            for (auto& [cores, result] : core_results) {
                result.speedup = baseline_time / result.execution_time_ms;
                result.efficiency_pct = (result.speedup / cores) * 100.0;
            }
        }
    }
    
    // ========== EXIBIR TABELA CONSOLIDADA ==========
    std::cout << "\n\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                    RESULTADOS CONSOLIDADOS - TODAS AS MÉTRICAS                                      ║\n";
    std::cout << "╠═════════╦══════╦══════════╦═════════╦═══════════╦═══════════╦═══════════╦════════════╦═══════════╦═══════════╦═══════╣\n";
    std::cout << "║ Política│Cores │Tempo(ms) │ Speedup │Eficiência │CacheHits  │CacheMisses│  HitRate   │ CPU Util  │Throughput │  CV%  ║\n";
    std::cout << "╠═════════╬══════╬══════════╬═════════╬═══════════╬═══════════╬═══════════╬════════════╬═══════════╬═══════════╬═══════╣\n";
    
    for (const auto& policy : policies) {
        bool first = true;
        for (int cores : core_configs) {
            if (all_results[policy].count(cores) == 0) continue;
            const auto& r = all_results[policy][cores];
            
            if (first) {
                std::cout << "║ " << std::setw(7) << std::left << policy << " ";
                first = false;
            } else {
                std::cout << "║         ";
            }
            
            std::cout << "│ " << std::setw(4) << std::right << cores << " "
                      << "│ " << std::setw(8) << std::fixed << std::setprecision(1) << r.execution_time_ms << " "
                      << "│ " << std::setw(7) << std::fixed << std::setprecision(2) << r.speedup << " "
                      << "│ " << std::setw(8) << std::fixed << std::setprecision(1) << r.efficiency_pct << "% "
                      << "│ " << std::setw(9) << r.cache_hits << " "
                      << "│ " << std::setw(9) << r.cache_misses << " "
                      << "│ " << std::setw(9) << std::fixed << std::setprecision(1) << r.hit_rate_pct << "% "
                      << "│ " << std::setw(8) << std::fixed << std::setprecision(1) << r.cpu_utilization_pct << "% "
                      << "│ " << std::setw(9) << std::fixed << std::setprecision(2) << r.throughput << " "
                      << "│ " << std::setw(5) << std::fixed << std::setprecision(1) << r.cv_percent << " ║\n";
        }
        if (policy != policies.back()) {
            std::cout << "╠═════════╬══════╬══════════╬═════════╬═══════════╬═══════════╬═══════════╬════════════╬═══════════╬═══════════╬═══════╣\n";
        }
    }
    std::cout << "╚═════════╩══════╩══════════╩═════════╩═══════════╩═══════════╩═══════════╩════════════╩═══════════╩═══════════╩═══════╝\n";
    
    // ========== CRIAR DIRETÓRIO ==========
#ifdef _WIN32
    system("if not exist dados_graficos mkdir dados_graficos");
#else
    system("mkdir -p dados_graficos");
#endif
    
    // ========== SALVAR CSV UNIFICADO COMPLETO ==========
    std::ofstream csv_unified("dados_graficos/unified_complete.csv");
    csv_unified << "Politica,Cores,Tempo_ms,Speedup,Eficiencia_Pct,CV_Pct,"
                << "Cache_Hits,Cache_Misses,Hit_Rate_Pct,"
                << "Tempo_Espera_ms,Tempo_Turnaround_ms,CPU_Utilizacao_Pct,Throughput_proc_s\n";
    
    for (const auto& policy : policies) {
        for (int cores : core_configs) {
            if (all_results[policy].count(cores) == 0) continue;
            const auto& r = all_results[policy][cores];
            
            csv_unified << policy << ","
                       << cores << ","
                       << std::fixed << std::setprecision(2) << r.execution_time_ms << ","
                       << std::fixed << std::setprecision(3) << r.speedup << ","
                       << std::fixed << std::setprecision(2) << r.efficiency_pct << ","
                       << std::fixed << std::setprecision(2) << r.cv_percent << ","
                       << r.cache_hits << ","
                       << r.cache_misses << ","
                       << std::fixed << std::setprecision(2) << r.hit_rate_pct << ","
                       << std::fixed << std::setprecision(2) << r.avg_wait_time << ","
                       << std::fixed << std::setprecision(2) << r.avg_turnaround_time << ","
                       << std::fixed << std::setprecision(2) << r.cpu_utilization_pct << ","
                       << std::fixed << std::setprecision(4) << r.throughput << "\n";
        }
    }
    csv_unified.close();
    
    // ========== SALVAR FORMATO COMPATÍVEL: escalonadores_multicore.csv ==========
    std::ofstream csv_multicore("dados_graficos/escalonadores_multicore.csv");
    csv_multicore << "Politica,Cores,Tempo_ms,Speedup,Eficiencia_Pct,CV_Pct\n";
    
    for (const auto& policy : policies) {
        for (int cores : core_configs) {
            if (all_results[policy].count(cores) == 0) continue;
            const auto& r = all_results[policy][cores];
            
            csv_multicore << policy << ","
                         << cores << ","
                         << std::fixed << std::setprecision(2) << r.execution_time_ms << ","
                         << std::fixed << std::setprecision(2) << r.speedup << ","
                         << std::fixed << std::setprecision(2) << r.efficiency_pct << ","
                         << std::fixed << std::setprecision(2) << r.cv_percent << "\n";
        }
    }
    csv_multicore.close();
    
    // ========== SALVAR FORMATO COMPATÍVEL: metricas_escalonadores.csv ==========
    // Usar dados de 2 cores para compatibilidade
    std::ofstream csv_metricas("dados_graficos/metricas_escalonadores.csv");
    csv_metricas << "Politica,Tempo_Espera_ms,Tempo_Execucao_ms,CPU_Utilizacao_Pct,Throughput_proc_s,Eficiencia\n";
    
    for (const auto& policy : policies) {
        // Usar 2 cores como referência (padrão do test_metrics_complete)
        int ref_cores = 2;
        if (all_results[policy].count(ref_cores) > 0) {
            const auto& r = all_results[policy][ref_cores];
            double efficiency = (r.cpu_utilization_pct / 100.0) * r.throughput;
            
            csv_metricas << policy << ","
                        << std::fixed << std::setprecision(2) << r.avg_wait_time << ","
                        << std::fixed << std::setprecision(2) << r.avg_turnaround_time << ","
                        << std::fixed << std::setprecision(2) << r.cpu_utilization_pct << ","
                        << std::fixed << std::setprecision(4) << r.throughput << ","
                        << std::fixed << std::setprecision(4) << efficiency << "\n";
        }
    }
    csv_metricas.close();
    
    // ========== SALVAR DADOS DE MEMÓRIA POR POLÍTICA E CORES ==========
    for (const auto& policy : policies) {
        std::ofstream csv_mem("dados_graficos/memoria_" + policy + ".csv");
        csv_mem << "cores,cache_hits,cache_misses,hit_rate\n";
        
        for (int cores : core_configs) {
            if (all_results[policy].count(cores) > 0) {
                const auto& r = all_results[policy][cores];
                csv_mem << cores << ","
                       << r.cache_hits << ","
                       << r.cache_misses << ","
                       << std::fixed << std::setprecision(2) << r.hit_rate_pct << "\n";
            }
        }
        csv_mem.close();
    }
    
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                         ARQUIVOS GERADOS                                 ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  ✅ dados_graficos/unified_complete.csv        (COMPLETO - todas métricas)\n";
    std::cout << "  ✅ dados_graficos/escalonadores_multicore.csv (compatibilidade)\n";
    std::cout << "  ✅ dados_graficos/metricas_escalonadores.csv  (compatibilidade)\n";
    std::cout << "  ✅ dados_graficos/memoria_RR.csv              (cache por cores)\n";
    std::cout << "  ✅ dados_graficos/memoria_FCFS.csv            (cache por cores)\n";
    std::cout << "  ✅ dados_graficos/memoria_SJN.csv             (cache por cores)\n";
    std::cout << "  ✅ dados_graficos/memoria_PRIORITY.csv        (cache por cores)\n\n";
    
    std::cout << "🎉 Agora você pode usar a GUI para comparar:\n";
    std::cout << "   • Número de Cores (X) vs Cache Hits (Y)\n";
    std::cout << "   • Número de Cores (X) vs Throughput (Y)\n";
    std::cout << "   • Qualquer combinação de métricas!\n\n";
    
    return 0;
}
