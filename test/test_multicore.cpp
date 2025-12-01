/**
 * Teste de Escalabilidade Multicore
 * Valida o desempenho do Round Robin com 1, 2, 4 e 8 núcleos
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <cmath>
#include "memory/MemoryManager.hpp"
#include "cpu/PCB.hpp"
#include "cpu/pcb_loader.hpp"
#include "cpu/Core.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

// Estrutura para armazenar resultados de cada teste
struct TestResult {
    int num_cores;
    int num_processes;
    int quantum;
    int total_cycles;
    double execution_time_ms;
    int total_context_switches;
    int processes_completed;
    double avg_wait_time;
    double avg_turnaround_time;
    double cpu_utilization;
    double speedup;
};

// Função para redirecionar stdout temporariamente
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
        if (old_cout) {
            std::cout.rdbuf(old_cout);
        }
        if (null_stream.is_open()) {
            null_stream.close();
        }
    }
};

// Função para executar teste com N núcleos
TestResult run_test(int num_cores, int num_processes, int quantum, int max_cycles) {
    TestResult result;
    result.num_cores = num_cores;
    result.num_processes = num_processes;
    result.quantum = quantum;
    result.total_cycles = 0;
    result.execution_time_ms = 0.0;
    result.total_context_switches = 0;
    result.processes_completed = 0;
    result.speedup = 0.0;
    
    try {
        // Reset estatísticas antes do teste
        MemoryManager::resetStats();
        
        // Criar componentes e processos
        MemoryManager memManager(1024, 8192);
        IOManager ioManager;
        std::vector<std::unique_ptr<PCB>> processes;
        
        // Carregar processos (SEM redeclaração de variáveis!)
        for (int i = 0; i < num_processes; i++) {
            auto pcb = std::make_unique<PCB>();
            
            if (load_pcb_from_json("process1.json", *pcb)) {
                pcb->pid = i + 1;
                pcb->name = "P" + std::to_string(i + 1);
                pcb->quantum = quantum;
                
                printf("[LOADING] Carregando programa para P%d\n", pcb->pid);
                fflush(stdout);
                loadJsonProgram("tasks.json", memManager, *pcb, 0);
                printf("[LOADING] P%d carregado\n", pcb->pid);
                fflush(stdout);
                processes.push_back(std::move(pcb));
            }
        }
        
        printf("[LOADING] TODOS os %d processos carregados! Agora vai iniciar scheduler...\n", num_processes);
        fflush(stdout);
        
        // CRITICAL: Use explicit scope to ensure scheduler is destroyed BEFORE processes
        {
            RoundRobinScheduler scheduler(num_cores, &memManager, &ioManager, quantum);
            
            // Add processes to scheduler
            for (auto& pcb : processes) {
                scheduler.add_process(pcb.get());
            }
            
            // Medir tempo de execução REAL (apenas processos rodando)
            auto start = std::chrono::high_resolution_clock::now();
            
            // Executar ATÉ TODOS processos terminarem
            int cycles = 0;
            int last_finished = 0;
            while (scheduler.has_pending_processes()) {
                scheduler.schedule_cycle();
                cycles++;
                
                // Debug: mostrar progresso a cada 1000 ciclos
                if (cycles % 1000 == 0) {
                    int finished_now = scheduler.get_finished_count();
                    printf("[PROGRESS] Ciclo %d: %d/%d processos finalizados\n", 
                           cycles, finished_now, num_processes);
                    fflush(stdout);
                    
                    // Se não houver progresso por 1000 ciclos, há um problema
                    if (finished_now == last_finished && cycles > 2000) {
                        printf("[ERROR] Sem progresso! Processos travados?\n");
                        break;
                    }
                    last_finished = finished_now;
                }
                
                // Safety: evitar loop infinito se houver bug
                if (cycles >= max_cycles) {
                    printf("[WARNING] Atingiu MAX_CYCLES (%d), forçando parada!\n", max_cycles);
                    printf("[DEBUG] Finalizados: %d/%d\n", scheduler.get_finished_count(), num_processes);
                    break;
                }
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            
            // Calcular tempo
            std::chrono::duration<double, std::milli> duration = end - start;
            result.execution_time_ms = duration.count();
            result.total_cycles = cycles;
            
            // Coletar métricas ANTES de sair do scope (destruir scheduler)
            auto stats = scheduler.get_statistics();
            result.total_context_switches = stats.total_context_switches;
            result.processes_completed = scheduler.get_finished_count();
            result.avg_wait_time = stats.avg_wait_time;
            result.avg_turnaround_time = stats.avg_turnaround_time;
            result.cpu_utilization = stats.avg_cpu_utilization;
            
            printf("[TEST] Teste concluído em %d ciclos (%d processos finalizados)\n", 
                   cycles, result.processes_completed);
            fflush(stdout);
            
            // Scheduler será destruído aqui ao sair do scope
        }
        
        // Coletar estatísticas de memória (FORA do SilentMode!)
        auto& mem_stats = MemoryManager::getStats();
        std::cout << "\n    📊 Estatísticas de Memória (" << num_cores << " core" << (num_cores > 1 ? "s" : "") << "):\n";
        std::cout << "       Cache Hits: " << mem_stats.cache_hits << "\n";
        std::cout << "       Cache Misses: " << mem_stats.cache_misses << "\n";
        std::cout << "       Cache Hit Rate: " << std::fixed << std::setprecision(1) 
                  << mem_stats.get_cache_hit_rate() << "%\n";
        std::cout << "       RAM Accesses: " << mem_stats.ram_accesses << "\n";
        std::cout << "       Disk Accesses: " << mem_stats.disk_accesses << "\n";
        std::cout << "       Lock Contentions: " << mem_stats.lock_contentions << "\n";
        std::cout << "       Avg Lock Wait: " << std::fixed << std::setprecision(2) 
                  << mem_stats.get_avg_lock_wait_us() << " μs\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Erro no teste com " << num_cores << " núcleos: " << e.what() << "\n";
    }
    
    return result;
}

// Função para imprimir linha da tabela
void print_table_line(const TestResult& result, double baseline_time) {
    // Calcular speedup
    double speedup = (baseline_time > 0) ? (baseline_time / result.execution_time_ms) : 1.0;
    double efficiency = (speedup / result.num_cores) * 100.0;
    
    std::cout << "│ " << std::setw(6) << result.num_cores
              << " │ " << std::setw(8) << result.total_cycles
              << " │ " << std::setw(10) << std::fixed << std::setprecision(2) << result.execution_time_ms
              << " │ " << std::setw(7) << std::fixed << std::setprecision(2) << speedup
              << " │ " << std::setw(10) << std::fixed << std::setprecision(1) << efficiency
              << " │ " << std::setw(9) << result.total_context_switches
              << " │ " << std::setw(9) << std::fixed << std::setprecision(1) << result.cpu_utilization
              << " │\n";
}

int main(int argc, char* argv[]) {
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     TESTE DE ESCALABILIDADE MULTICORE - ROUND ROBIN          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    // Configuração do teste - COM MÚLTIPLAS EXECUÇÕES PARA CONFIABILIDADE
    const int NUM_PROCESSES = 8;     // Original
    const int QUANTUM = 1000;        // Quantum grande (programa tem loops)
    const int MAX_CYCLES = 10000;    // Limite seguro
    const int NUM_RUNS = 10;         // Executar 10 vezes e calcular média
    
    // MemoryManager agora é thread-safe com recursive_mutex!
    std::vector<int> core_counts = {1, 2, 4, 8};
    std::vector<TestResult> results;
    
    std::cout << "Configuração do Teste:\n";
    std::cout << "  • Processos: " << NUM_PROCESSES << "\n";
    std::cout << "  • Workload: tasks.json (90 instruções + loops)\n";
    std::cout << "  • Quantum: " << QUANTUM << " ciclos\n";
    std::cout << "  • Máximo de ciclos: " << MAX_CYCLES << "\n";
    std::cout << "  • Execuções por configuração: " << NUM_RUNS << " (média ± desvio padrão)\n";
    std::cout << "  • Warm-up: 1 execução inicial descartada\n";
    std::cout << "  • Núcleos testados: 1, 2, 4, 8\n";
    std::cout << "  • MemoryManager: Thread-safe com shared_mutex (leituras paralelas)\n\n";
    
    std::cout << "⚠️  NOTA: Variabilidade é normal em sistemas multithread devido a:\n";
    std::cout << "    - Scheduler do SO (context switches não-determinísticos)\n";
    std::cout << "    - Cache effects (estados variam entre execuções)\n";
    std::cout << "    - Race conditions inerentes ao paralelismo\n";
    std::cout << "    CV < 20% é considerado aceitável para testes multicore.\n\n";
    
    std::cout << "Executando testes (isso pode levar alguns minutos)...\n\n";
    
    // Executar testes com múltiplas rodadas
    for (int cores : core_counts) {
        std::cout << "  ► Testando com " << cores << " núcleo(s)... " << std::flush;
        
        // WARM-UP: Executar 1 vez para estabilizar cache (descartado)
        std::cout << "[aquecimento...] " << std::flush;
        run_test(cores, NUM_PROCESSES, QUANTUM, MAX_CYCLES);
        
        // Executar NUM_RUNS vezes e calcular média
        std::vector<double> times;
        TestResult avg_result;
        
        for (int run = 0; run < NUM_RUNS; run++) {
            TestResult result = run_test(cores, NUM_PROCESSES, QUANTUM, MAX_CYCLES);
            times.push_back(result.execution_time_ms);
            
            // Acumular métricas
            if (run == 0) {
                avg_result = result;
            } else {
                avg_result.execution_time_ms += result.execution_time_ms;
                avg_result.total_cycles += result.total_cycles;
                avg_result.total_context_switches += result.total_context_switches;
            }
        }
        
        // Calcular média
        avg_result.execution_time_ms /= NUM_RUNS;
        avg_result.total_cycles /= NUM_RUNS;
        avg_result.total_context_switches /= NUM_RUNS;
        
        // Calcular desvio padrão
        double mean = avg_result.execution_time_ms;
        double variance = 0.0;
        for (double time : times) {
            variance += (time - mean) * (time - mean);
        }
        double stddev = std::sqrt(variance / NUM_RUNS);
        double cv = (stddev / mean) * 100.0; // Coeficiente de variação
        
        results.push_back(avg_result);
        
        std::cout << "✓ Média: " << std::fixed << std::setprecision(2) 
                  << avg_result.execution_time_ms << " ms (CV: " 
                  << std::setprecision(1) << cv << "%)\n";
    }
    
    std::cout << "\n";
    
    // Baseline para cálculo de speedup (single-core)
    double baseline_time = results.empty() ? 0.0 : results[0].execution_time_ms;
    
    // Imprimir tabela de resultados
    std::cout << "╔════════════════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                              RESULTADOS DO TESTE DE ESCALABILIDADE                            ║\n";
    std::cout << "╠════════╦══════════╦════════════╦═════════╦════════════╦═══════════╦═══════════╣\n";
    std::cout << "║ Núcleos│  Ciclos  │  Tempo(ms) │ Speedup │ Eficiência │  Ctx SW   │ CPU Util% ║\n";
    std::cout << "╠════════╬══════════╬════════════╬═════════╬════════════╬═══════════╬═══════════╣\n";
    
    for (const auto& result : results) {
        print_table_line(result, baseline_time);
    }
    
    std::cout << "╚════════╩══════════╩════════════╩═════════╩════════════╩═══════════╩═══════════╝\n\n";
    
    // Análise de resultados
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                     ANÁLISE DE DESEMPENHO                      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";
    
    if (results.size() >= 2) {
        double speedup_2cores = baseline_time / results[1].execution_time_ms;
        double speedup_4cores = (results.size() >= 3) ? (baseline_time / results[2].execution_time_ms) : 0.0;
        double speedup_8cores = (results.size() >= 4) ? (baseline_time / results[3].execution_time_ms) : 0.0;
        
        std::cout << "📊 Speedup Observado:\n";
        std::cout << "   • 2 núcleos: " << std::fixed << std::setprecision(2) << speedup_2cores << "x\n";
        if (results.size() >= 3)
            std::cout << "   • 4 núcleos: " << speedup_4cores << "x\n";
        if (results.size() >= 4)
            std::cout << "   • 8 núcleos: " << speedup_8cores << "x\n";
        
        std::cout << "\n📈 Interpretação:\n";
        if (speedup_2cores >= 1.8) {
            std::cout << "   ✅ Escalabilidade EXCELENTE para 2 núcleos!\n";
        } else if (speedup_2cores >= 1.5) {
            std::cout << "   ✅ Escalabilidade BOA para 2 núcleos.\n";
        } else if (speedup_2cores >= 1.2) {
            std::cout << "   ⚠️  Escalabilidade MODERADA para 2 núcleos.\n";
        } else {
            std::cout << "   ❌ Escalabilidade BAIXA - possível gargalo de sincronização.\n";
        }
        
        if (results.size() >= 4 && speedup_8cores > 0) {
            double efficiency_8 = (speedup_8cores / 8.0) * 100.0;
            if (efficiency_8 >= 70.0) {
                std::cout << "   ✅ Eficiência de " << std::fixed << std::setprecision(1) 
                          << efficiency_8 << "% com 8 núcleos é EXCELENTE!\n";
            } else if (efficiency_8 >= 50.0) {
                std::cout << "   ✅ Eficiência de " << efficiency_8 << "% com 8 núcleos é ACEITÁVEL.\n";
            } else {
                std::cout << "   ⚠️  Eficiência de " << efficiency_8 
                          << "% com 8 núcleos indica overhead de sincronização.\n";
            }
        }
    }
    
    std::cout << "\n💡 Observações:\n";
    std::cout << "   • Context switches indicam preempção por quantum\n";
    std::cout << "   • Utilização de CPU mostra balanceamento de carga\n";
    std::cout << "   • Speedup ideal = N (para N núcleos)\n";
    std::cout << "   • Eficiência = (Speedup / N) × 100%\n";
    
    // Salvar resultados em arquivo CSV
    std::ofstream csv_file("logs/multicore_results.csv");
    if (csv_file.is_open()) {
        csv_file << "Nucleos,Ciclos,Tempo_ms,Speedup,Eficiencia_%,Context_Switches,CPU_Util_%\n";
        for (const auto& result : results) {
            double speedup = baseline_time / result.execution_time_ms;
            double efficiency = (speedup / result.num_cores) * 100.0;
            csv_file << result.num_cores << ","
                    << result.total_cycles << ","
                    << std::fixed << std::setprecision(2) << result.execution_time_ms << ","
                    << speedup << ","
                    << efficiency << ","
                    << result.total_context_switches << ","
                    << result.cpu_utilization << "\n";
        }
        csv_file.close();
        std::cout << "\n✅ Resultados salvos em: logs/multicore_results.csv\n";
    }
    
    std::cout << "\n✓ Teste de escalabilidade concluído!\n\n";
    
    return 0;
}
