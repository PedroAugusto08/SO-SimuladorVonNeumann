/**
 * Teste Comparativo de Eficiência Multicore - TODAS AS POLÍTICAS
 * 
 * Este teste compara o desempenho de 5 políticas de escalonamento:
 *   - RR (Round Robin): Preemptivo com quantum
 *   - FCFS (First Come First Served): Não-preemptivo
 *   - SJN (Shortest Job Next): Não-preemptivo, ordenado por tamanho
 *   - PRIORITY: Não-preemptivo, ordenado por prioridade
 *   - PRIORITY_PREEMPT: Preemptivo por prioridade (sem quantum)
 * 
 * METODOLOGIA:
 *   - Workload idêntico para todas as políticas
 *   - 3 iterações + 1 warm-up para estabilidade estatística
 *   - Remoção de outliers >1.5σ (desvio padrão)
 *   - CV < 15%: Excelente confiabilidade
 *   - Speedup = baseline_time / current_time
 * 
 * @author Grupo Peripherals
 * @date 2024
 * @version 5.0 - Teste comparativo com 5 políticas incluindo PRIORITY_PREEMPT
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
#include "memory/MemoryManager.hpp"
#include "cpu/PCB.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/Core.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

struct TestResult {
    std::string policy;            // RR, FCFS, SJN, PRIORITY, PRIORITY_PREEMPT
    int num_cores;
    double execution_time_ms;
    double speedup;
    double efficiency;
    double cv_percent;
    int processes_finished;
};

class SilentMode {
private:
    std::streambuf* old_cout;
    std::ofstream null_stream;
public:
    SilentMode() : old_cout(nullptr) {
        null_stream.open("/dev/null");
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

TestResult run_test(const std::string& policy, int num_cores, int num_processes, int quantum, int max_cycles, const std::string& tasks_file) {
    TestResult result;
    result.policy = policy;
    result.num_cores = num_cores;
    result.processes_finished = 0;
    
    std::vector<PCB*> process_ptrs;
    
    try {
        SilentMode silent;
        MemoryManager::resetStats();
        
        MemoryManager* memManager = new MemoryManager(4096, 32768);
        IOManager* ioManager = new IOManager();
        
        // Carregar processos
        for (int i = 0; i < num_processes; i++) {
            PCB* pcb = new PCB();
            if (load_pcb_from_json("process1.json", *pcb)) {
                pcb->pid = i + 1;
                pcb->name = "P" + std::to_string(i + 1);
                pcb->quantum = quantum;
                pcb->arrival_time = 0;
                loadJsonProgram(tasks_file, *memManager, *pcb, i * 1024);
                pcb->estimated_job_size = pcb->program_size; // Para SJN
                process_ptrs.push_back(pcb);
            } else {
                delete pcb;
            }
        }
        
        double execution_time_ms = 0.0;
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
            result.processes_finished = scheduler.get_finished_count();
            
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
            result.processes_finished = process_ptrs.size();
            
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
            result.processes_finished = process_ptrs.size();
            
        } else if (policy == "PRIORITY" || policy == "PRIORITY_PREEMPT") {
            PriorityScheduler scheduler(num_cores, memManager, ioManager, quantum);
            // Atribuir prioridades diferentes para teste (maior número = maior prioridade)
            for (size_t i = 0; i < process_ptrs.size(); i++) {
                process_ptrs[i]->priority = 10 - (i % 3) * 3;  // Prioridades: 10, 7, 4
            }
            for (PCB* pcb : process_ptrs) {
                scheduler.add_process(pcb);
            }
            
            int cycles = 0;
            while (!scheduler.all_finished() && cycles < max_cycles) {
                scheduler.schedule_cycle();
                cycles++;
            }
            result.processes_finished = scheduler.get_finished_count();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> duration = end - start;
        execution_time_ms = duration.count();
        result.execution_time_ms = execution_time_ms;
        
    } catch (const std::exception& e) {
        result.execution_time_ms = 0.0;
    } catch (...) {
        result.execution_time_ms = 0.0;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return result;
}

int main() {
    const int NUM_PROCESSES = 8;
    const int QUANTUM = 1000;
    const int MAX_CYCLES = 10000000;
    const std::string TASKS_FILE = "tasks.json";
    const int ITERATIONS = 3;
    const int WARMUP_ITERATIONS = 1;
    
    std::vector<std::string> policies = {"RR", "FCFS", "SJN", "PRIORITY", "PRIORITY_PREEMPT"};
    std::vector<int> core_configs = {1, 2, 4, 6};
    std::map<std::string, std::vector<TestResult>> results_by_policy;
    
    std::cout << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TESTE COMPARATIVO DE POLÍTICAS DE ESCALONAMENTO MULTICORE      ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Configuração:\n";
    std::cout << "  • Políticas: RR, FCFS, SJN, PRIORITY, PRIORITY_PREEMPT\n";
    std::cout << "  • Processos: " << NUM_PROCESSES << "\n";
    std::cout << "  • Quantum (RR): " << QUANTUM << " ciclos\n";
    std::cout << "  • Workload: " << TASKS_FILE << "\n";
    std::cout << "  • Iterações: " << ITERATIONS << " (após " << WARMUP_ITERATIONS << " warm-ups)\n";
    std::cout << "  • Métrica: TEMPO DE EXECUÇÃO (ms) - menor é MELHOR\n\n";
    
    // Executar testes para cada política
    for (const auto& policy : policies) {
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  Testando política: " << policy;
        if (policy == "RR") std::cout << " (Round Robin - Preemptivo)";
        else if (policy == "FCFS") std::cout << " (First Come First Served)";
        else if (policy == "SJN") std::cout << " (Shortest Job Next)";
        else if (policy == "PRIORITY") std::cout << " (Priority - Por Prioridade)";
        else if (policy == "PRIORITY_PREEMPT") std::cout << " (Priority Preemptivo)";
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        for (int num_cores : core_configs) {
            std::cout << "  ► " << num_cores << " core(s): ";
            std::vector<double> times;
            
            // Warm-up
            for (int w = 0; w < WARMUP_ITERATIONS; w++) {
                std::cout << "[warm-up.] " << std::flush;
                run_test(policy, num_cores, NUM_PROCESSES, QUANTUM, MAX_CYCLES, TASKS_FILE);
            }
            
            // Iterações válidas
            std::cout << "[medindo] " << std::flush;
            for (int i = 0; i < ITERATIONS; i++) {
                TestResult result = run_test(policy, num_cores, NUM_PROCESSES, QUANTUM, MAX_CYCLES, TASKS_FILE);
                if (result.execution_time_ms > 0) {
                    times.push_back(result.execution_time_ms);
                }
            }
            
            if (times.empty()) {
                std::cout << "✗ Falhou\n";
                continue;
            }
            
            // Calcular estatísticas
            double sum = 0.0;
            for (double t : times) sum += t;
            double mean = sum / times.size();
            
            double variance = 0.0;
            for (double t : times) {
                variance += (t - mean) * (t - mean);
            }
            double stddev = std::sqrt(variance / times.size());
            double cv = (mean > 0) ? (stddev / mean * 100.0) : 0.0;
            
            // Remover outliers
            std::vector<double> filtered;
            for (double t : times) {
                if (std::abs(t - mean) <= 1.5 * stddev) {
                    filtered.push_back(t);
                }
            }
            
            // Recalcular com dados filtrados
            double final_time = mean;
            if (!filtered.empty()) {
                sum = 0.0;
                for (double t : filtered) sum += t;
                final_time = sum / filtered.size();
                
                variance = 0.0;
                for (double t : filtered) {
                    variance += (t - final_time) * (t - final_time);
                }
                stddev = std::sqrt(variance / filtered.size());
                cv = (final_time > 0) ? (stddev / final_time * 100.0) : 0.0;
            }
            
            TestResult final_result;
            final_result.policy = policy;
            final_result.num_cores = num_cores;
            final_result.execution_time_ms = final_time;
            final_result.cv_percent = cv;
            final_result.processes_finished = NUM_PROCESSES;
            
            results_by_policy[policy].push_back(final_result);
            
            std::cout << "✓ " << std::fixed << std::setprecision(1) 
                      << final_time << " ms (CV: " << cv << "%)\n";
        }
    }
    
    // Calcular speedup e eficiência para cada política
    for (auto& [policy, results] : results_by_policy) {
        double baseline_time = results[0].execution_time_ms;
        for (auto& result : results) {
            result.speedup = baseline_time / result.execution_time_ms;
            result.efficiency = (result.speedup / result.num_cores) * 100.0;
        }
    }
    
    // Exibir resultados comparativos
    std::cout << "\n\n╔════════════════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                         RESULTADOS COMPARATIVOS - TODAS AS POLÍTICAS                              ║\n";
    std::cout << "╠═══════════╦════════╦═══════════╦═════════╦════════════╦═══════════╦═════════════════════════════╣\n";
    std::cout << "║ Política  │ Cores  │  Tempo(ms)│ Speedup │ Eficiência │    CV%    │         Status              ║\n";
    std::cout << "║           │        │           │         │     %      │           │                             ║\n";
    std::cout << "╠═══════════╬════════╬═══════════╬═════════╬════════════╬═══════════╬═════════════════════════════╣\n";
    
    for (const auto& policy : policies) {
        const auto& results = results_by_policy[policy];
        std::string policy_name = policy;
        if (policy == "RR") policy_name = "RR       ";
        else if (policy == "FCFS") policy_name = "FCFS     ";
        else if (policy == "SJN") policy_name = "SJN      ";
        
        for (size_t i = 0; i < results.size(); i++) {
            const auto& result = results[i];
            std::string status;
            if (result.speedup >= 0.95) status = "✓ Linear               ";
            else if (result.speedup >= 0.7) status = "⚠ Sublinear            ";
            else status = "✗ Degradado            ";
            
            if (i == 0) {
                std::cout << "║ " << policy_name;
            } else {
                std::cout << "║           ";
            }
            
            std::cout << " │ " << std::setw(6) << result.num_cores 
                      << " │ " << std::setw(9) << std::fixed << std::setprecision(2) << result.execution_time_ms
                      << " │ " << std::setw(7) << std::fixed << std::setprecision(2) << result.speedup
                      << " │ " << std::setw(10) << std::fixed << std::setprecision(1) << result.efficiency
                      << " │ " << std::setw(9) << std::fixed << std::setprecision(2) << result.cv_percent
                      << " │ " << status << " ║\n";
        }
        if (policy != policies.back()) {
            std::cout << "╠═══════════╬════════╬═══════════╬═════════╬════════════╬═══════════╬═════════════════════════════╣\n";
        }
    }
    
    std::cout << "╚═══════════╩════════╩═══════════╩═════════╩════════════╩═══════════╩═════════════════════════════╝\n";
    
    // Análise comparativa entre políticas
    std::cout << "\n╔════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    ANÁLISE COMPARATIVA ENTRE POLÍTICAS                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "⏱️  Tempo de Execução por Política (menor é MELHOR):\n\n";
    for (int num_cores : core_configs) {
        std::cout << "  🔹 " << num_cores << " core(s):\n";
        std::vector<std::pair<std::string, double>> policy_times;
        for (const auto& policy : policies) {
            for (const auto& result : results_by_policy[policy]) {
                if (result.num_cores == num_cores) {
                    policy_times.push_back({policy, result.execution_time_ms});
                    break;
                }
            }
        }
        
        // Ordenar por tempo (melhor primeiro)
        std::sort(policy_times.begin(), policy_times.end(), 
                  [](const auto& a, const auto& b) { return a.second < b.second; });
        
        for (size_t i = 0; i < policy_times.size(); i++) {
            std::string medal = (i == 0) ? "🥇" : (i == 1) ? "🥈" : "🥉";
            std::cout << "     " << medal << " " << std::setw(6) << std::left << policy_times[i].first 
                      << ": " << std::fixed << std::setprecision(2) << policy_times[i].second << " ms";
            if (i == 0) {
                std::cout << " (MELHOR)";
            } else {
                double diff = ((policy_times[i].second - policy_times[0].second) / policy_times[0].second) * 100.0;
                std::cout << " (+" << std::fixed << std::setprecision(1) << diff << "%)";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    
    // Salvar resultados em CSV
    std::ofstream csv_file("logs/multicore_comparative_results.csv");
    csv_file << "Politica,Cores,Tempo_ms,Speedup,Eficiencia_%,CV_%\n";
    for (const auto& policy : policies) {
        for (const auto& result : results_by_policy[policy]) {
            csv_file << policy << ","
                     << result.num_cores << ","
                     << std::fixed << std::setprecision(2) << result.execution_time_ms << ","
                     << std::fixed << std::setprecision(2) << result.speedup << ","
                     << std::fixed << std::setprecision(2) << result.efficiency << ","
                     << std::fixed << std::setprecision(2) << result.cv_percent << "\n";
        }
    }
    csv_file.close();
    
    std::cout << "\n✅ Resultados salvos em: logs/multicore_comparative_results.csv\n";
    
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                           CONCLUSÕES                           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    // Determinar melhor política geral
    std::map<std::string, double> avg_time_by_policy;
    for (const auto& policy : policies) {
        double sum = 0.0;
        for (const auto& result : results_by_policy[policy]) {
            sum += result.execution_time_ms;
        }
        avg_time_by_policy[policy] = sum / results_by_policy[policy].size();
    }
    
    std::string best_policy;
    double best_time = std::numeric_limits<double>::max();
    for (const auto& [policy, time] : avg_time_by_policy) {
        if (time < best_time) {
            best_time = time;
            best_policy = policy;
        }
    }
    
    std::cout << "🏆 Melhor política geral: " << best_policy 
              << " (tempo médio: " << std::fixed << std::setprecision(1) << best_time << " ms)\n\n";
    
    std::cout << "📊 Características observadas:\n";
    std::cout << "  • RR (Round Robin): Preemptivo, justo, mais overhead de troca de contexto\n";
    std::cout << "  • FCFS: Simples, sem preempção, pode ter espera longa\n";
    std::cout << "  • SJN: Otimiza tempo médio, mas pode causar starvation\n\n";
    
    return 0;
}
