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
#include "src/memory/MemoryManager.hpp"
#include "src/cpu/PCB.hpp"
#include "src/cpu/pcb_loader.hpp"
#include "src/cpu/Core.hpp"
#include "src/cpu/RoundRobinScheduler.hpp"
#include "src/parser_json/parser_json.hpp"
#include "src/IO/IOManager.hpp"

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
        
        // SCOPE para SilentMode - destruído antes das stats
        {
            // Suprimir saída verbosa - DESABILITADO PARA DEBUG
            // SilentMode silent;
        IOManager ioManager;
        std::vector<std::unique_ptr<PCB>> processes;
        
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
            
            // Medir tempo de execução
            auto start = std::chrono::high_resolution_clock::now();
            
            // Executar
            int cycles = 0;
            while (cycles < max_cycles && scheduler.has_pending_processes()) {
                scheduler.schedule_cycle();
                cycles++;
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
            
            // Scheduler será destruído aqui ao sair do scope
        }
        
        } // FIM DO SCOPE DO SilentMode - agora std::cout está restaurado!
        
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
    
    // Configuração do teste - VOLTANDO AO ORIGINAL (estava funcionando)
    const int NUM_PROCESSES = 8;     // Original
    const int QUANTUM = 100;         // Quantum padrão
    const int MAX_CYCLES = 10000;    // Original
    
    // MemoryManager agora é thread-safe com recursive_mutex!
    std::vector<int> core_counts = {1, 2, 4, 8};
    std::vector<TestResult> results;
    
    std::cout << "Configuração do Teste:\n";
    std::cout << "  • Processos: " << NUM_PROCESSES << "\n";
    std::cout << "  • Quantum: " << QUANTUM << " ciclos\n";
    std::cout << "  • Máximo de ciclos: " << MAX_CYCLES << "\n";
    std::cout << "  • Núcleos testados: 1, 2, 4, 8\n";
    std::cout << "  • MemoryManager: Thread-safe com shared_mutex (leituras paralelas)\n\n";
    
    std::cout << "Executando testes (isso pode levar alguns minutos)...\n\n";
    
    // Executar testes
    for (int cores : core_counts) {
        std::cout << "  ► Testando com " << cores << " núcleo(s)... " << std::flush;
        
        TestResult result = run_test(cores, NUM_PROCESSES, QUANTUM, MAX_CYCLES);
        results.push_back(result);
        
        std::cout << "✓ Concluído em " << std::fixed << std::setprecision(2) 
                  << result.execution_time_ms << " ms\n";
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
