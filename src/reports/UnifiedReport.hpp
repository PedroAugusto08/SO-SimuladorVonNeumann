#ifndef UNIFIED_REPORT_HPP
#define UNIFIED_REPORT_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "../cpu/SchedulerBase.hpp"

/**
 * UnifiedReport - Gera relatório consolidado combinando:
 * - Métricas de escalonamento (wait time, turnaround, throughput, etc.)
 * - Utilização de memória (hits/misses de cache, uso de RAM/disco)
 * - Informações de configuração (cores, política, quantum)
 */
class UnifiedReport {
public:
    struct MemoryStats {
        uint64_t total_cache_hits{0};
        uint64_t total_cache_misses{0};
        double cache_hit_rate{0.0};
        uint64_t peak_main_memory{0};
        uint64_t peak_secondary_memory{0};
        uint64_t avg_main_memory{0};
        uint64_t avg_secondary_memory{0};
        int memory_samples{0};
        std::string memory_timeline_file{};
    };

    struct Configuration {
        int num_cores{1};
        std::string policy{"RR"};
        int quantum{100};
        int num_processes{0};
        std::string program_file{};
    };

    UnifiedReport(const std::string& output_file) 
        : output_file_(output_file) {}

    // Define configuração da execução
    void set_configuration(const Configuration& config) {
        config_ = config;
    }

    // Adiciona métricas de escalonamento
    void set_scheduler_stats(const SchedulerBase::Statistics& stats) {
        scheduler_stats_ = stats;
    }

    // Adiciona estatísticas de memória
    void set_memory_stats(const MemoryStats& stats) {
        memory_stats_ = stats;
    }

    // Adiciona tempo total de execução
    void set_execution_time(double time_ms) {
        execution_time_ms_ = time_ms;
    }

    // Gera o relatório consolidado em CSV
    bool generate_csv() {
        std::ofstream csv(output_file_);
        if (!csv.is_open()) {
            return false;
        }

        // Cabeçalho
        csv << "Metric,Value,Unit\n";
        
        // Configuração
        csv << "\n# CONFIGURATION\n";
        csv << "Cores," << config_.num_cores << ",count\n";
        csv << "Policy," << config_.policy << ",name\n";
        csv << "Quantum," << config_.quantum << ",cycles\n";
        csv << "Processes," << config_.num_processes << ",count\n";
        csv << "Program," << config_.program_file << ",path\n";
        csv << "Execution_Time," << std::fixed << std::setprecision(2) 
            << execution_time_ms_ << ",ms\n";

        // Métricas de Escalonamento
        csv << "\n# SCHEDULING_METRICS\n";
        csv << "Avg_Wait_Time," << std::fixed << std::setprecision(2) 
            << scheduler_stats_.avg_wait_time << ",ms\n";
        csv << "Avg_Turnaround_Time," << std::fixed << std::setprecision(2) 
            << scheduler_stats_.avg_turnaround_time << ",ms\n";
        csv << "Avg_Response_Time," << std::fixed << std::setprecision(2) 
            << scheduler_stats_.avg_response_time << ",ms\n";
        csv << "CPU_Utilization," << std::fixed << std::setprecision(2) 
            << scheduler_stats_.avg_cpu_utilization << ",%\n";
        csv << "Throughput," << std::fixed << std::setprecision(4) 
            << scheduler_stats_.throughput << ",proc/ms\n";
        csv << "Context_Switches," << scheduler_stats_.total_context_switches << ",count\n";
        csv << "Processes_Completed," << scheduler_stats_.total_processes << ",count\n";

        // Métricas de Memória
        csv << "\n# MEMORY_METRICS\n";
        csv << "Cache_Hits," << memory_stats_.total_cache_hits << ",count\n";
        csv << "Cache_Misses," << memory_stats_.total_cache_misses << ",count\n";
        csv << "Cache_Hit_Rate," << std::fixed << std::setprecision(2) 
            << memory_stats_.cache_hit_rate << ",%\n";
        csv << "Peak_Main_Memory," << memory_stats_.peak_main_memory << ",bytes\n";
        csv << "Peak_Secondary_Memory," << memory_stats_.peak_secondary_memory << ",bytes\n";
        csv << "Avg_Main_Memory," << memory_stats_.avg_main_memory << ",bytes\n";
        csv << "Avg_Secondary_Memory," << memory_stats_.avg_secondary_memory << ",bytes\n";
        csv << "Memory_Samples," << memory_stats_.memory_samples << ",count\n";
        csv << "Memory_Timeline," << memory_stats_.memory_timeline_file << ",path\n";

        // Métricas Derivadas
        csv << "\n# DERIVED_METRICS\n";
        double avg_time_per_process = (scheduler_stats_.total_processes > 0) 
            ? execution_time_ms_ / scheduler_stats_.total_processes 
            : 0.0;
        csv << "Avg_Time_Per_Process," << std::fixed << std::setprecision(2) 
            << avg_time_per_process << ",ms\n";
        
        double efficiency = (config_.num_cores > 0 && execution_time_ms_ > 0)
            ? (scheduler_stats_.total_processes * 1000.0) / (config_.num_cores * execution_time_ms_) * 100.0
            : 0.0;
        csv << "Parallel_Efficiency," << std::fixed << std::setprecision(2) 
            << efficiency << ",%\n";

        uint64_t total_cache_accesses = memory_stats_.total_cache_hits + memory_stats_.total_cache_misses;
        double cache_miss_penalty = (total_cache_accesses > 0)
            ? (memory_stats_.total_cache_misses * 100.0) / total_cache_accesses
            : 0.0;
        csv << "Cache_Miss_Penalty," << std::fixed << std::setprecision(2) 
            << cache_miss_penalty << ",%\n";

        csv.close();
        return true;
    }

    // Gera relatório em formato texto legível
    bool generate_text_report() {
        std::string text_file = output_file_;
        size_t ext_pos = text_file.find_last_of('.');
        if (ext_pos != std::string::npos) {
            text_file = text_file.substr(0, ext_pos) + ".txt";
        } else {
            text_file += ".txt";
        }

        std::ofstream report(text_file);
        if (!report.is_open()) {
            return false;
        }

        report << "╔════════════════════════════════════════════════════════════════════╗\n";
        report << "║           RELATÓRIO CONSOLIDADO DE DESEMPENHO                      ║\n";
        report << "╚════════════════════════════════════════════════════════════════════╝\n\n";

        // Configuração
        report << "═══ CONFIGURAÇÃO ═══\n";
        report << "  Núcleos:              " << config_.num_cores << "\n";
        report << "  Política:             " << config_.policy << "\n";
        report << "  Quantum:              " << config_.quantum << " ciclos\n";
        report << "  Processos:            " << config_.num_processes << "\n";
        report << "  Programa:             " << config_.program_file << "\n";
        report << "  Tempo de Execução:    " << std::fixed << std::setprecision(2) 
               << execution_time_ms_ << " ms\n\n";

        // Métricas de Escalonamento
        report << "═══ MÉTRICAS DE ESCALONAMENTO ═══\n";
        report << std::fixed << std::setprecision(2);
        report << "  Tempo Médio de Espera:        " << scheduler_stats_.avg_wait_time << " ms\n";
        report << "  Tempo Médio de Turnaround:    " << scheduler_stats_.avg_turnaround_time << " ms\n";
        report << "  Tempo Médio de Resposta:      " << scheduler_stats_.avg_response_time << " ms\n";
        report << "  Utilização da CPU:            " << scheduler_stats_.avg_cpu_utilization << " %\n";
        report << "  Throughput:                   " << std::setprecision(4) 
               << scheduler_stats_.throughput << " proc/ms\n";
        report << "  Trocas de Contexto:           " << scheduler_stats_.total_context_switches << "\n";
        report << "  Processos Concluídos:         " << scheduler_stats_.total_processes << "\n\n";

        // Métricas de Memória
        report << "═══ MÉTRICAS DE MEMÓRIA ═══\n";
        report << "  Cache Hits:                   " << memory_stats_.total_cache_hits << "\n";
        report << "  Cache Misses:                 " << memory_stats_.total_cache_misses << "\n";
        report << "  Taxa de Acerto (Hit Rate):    " << std::fixed << std::setprecision(2) 
               << memory_stats_.cache_hit_rate << " %\n";
        report << "  Pico de Mem. Principal:       " << memory_stats_.peak_main_memory << " bytes\n";
        report << "  Pico de Mem. Secundária:      " << memory_stats_.peak_secondary_memory << " bytes\n";
        report << "  Média de Mem. Principal:      " << memory_stats_.avg_main_memory << " bytes\n";
        report << "  Média de Mem. Secundária:     " << memory_stats_.avg_secondary_memory << " bytes\n";
        report << "  Amostras Coletadas:           " << memory_stats_.memory_samples << "\n";
        if (!memory_stats_.memory_timeline_file.empty()) {
            report << "  Timeline Detalhado:           " << memory_stats_.memory_timeline_file << "\n";
        }
        report << "\n";

        // Análise
        report << "═══ ANÁLISE DE DESEMPENHO ═══\n";
        double avg_time_per_process = (scheduler_stats_.total_processes > 0) 
            ? execution_time_ms_ / scheduler_stats_.total_processes 
            : 0.0;
        report << "  Tempo Médio por Processo:     " << std::fixed << std::setprecision(2) 
               << avg_time_per_process << " ms\n";
        
        double efficiency = (config_.num_cores > 0 && execution_time_ms_ > 0)
            ? (scheduler_stats_.total_processes * 1000.0) / (config_.num_cores * execution_time_ms_) * 100.0
            : 0.0;
        report << "  Eficiência Paralela:          " << std::fixed << std::setprecision(2) 
               << efficiency << " %\n";

        uint64_t total_cache_accesses = memory_stats_.total_cache_hits + memory_stats_.total_cache_misses;
        double cache_miss_penalty = (total_cache_accesses > 0)
            ? (memory_stats_.total_cache_misses * 100.0) / total_cache_accesses
            : 0.0;
        report << "  Penalidade de Cache Miss:     " << std::fixed << std::setprecision(2) 
               << cache_miss_penalty << " %\n\n";

        // Utilização de Memória ao Longo do Tempo
        if (!memory_stats_.memory_timeline_file.empty() && memory_stats_.memory_samples > 0) {
            report << "═══ UTILIZAÇÃO DE MEMÓRIA AO LONGO DO TEMPO ═══\n";
            report << "  Arquivo de Timeline:          " << memory_stats_.memory_timeline_file << "\n";
            report << "  Total de Amostras:            " << memory_stats_.memory_samples << "\n";
            report << "  Frequência de Amostragem:     ~" << std::fixed << std::setprecision(1)
                   << (execution_time_ms_ / memory_stats_.memory_samples) << " ms/amostra\n\n";
            
            report << "  📊 Este arquivo contém o histórico temporal completo de:\n";
            report << "     • Uso de memória principal (bytes)\n";
            report << "     • Uso de memória secundária (bytes)\n";
            report << "     • Cache hits acumulados\n";
            report << "     • Cache misses acumulados\n";
            report << "     • Timestamps em millisegundos\n\n";
            
            report << "  💡 Visualize com:\n";
            report << "     cat " << memory_stats_.memory_timeline_file << "\n";
            report << "     ou importe em Excel/Python para gráficos\n\n";
        }
        
        report << "═══════════════════════════════════════════════════════════════════\n";
        report << "Relatório gerado em: " << output_file_ << "\n";
        if (!memory_stats_.memory_timeline_file.empty()) {
            report << "Timeline de memória: " << memory_stats_.memory_timeline_file << "\n";
        }
        report << "═══════════════════════════════════════════════════════════════════\n";

        report.close();
        return true;
    }

private:
    std::string output_file_;
    Configuration config_;
    SchedulerBase::Statistics scheduler_stats_;
    MemoryStats memory_stats_;
    double execution_time_ms_{0.0};
};

#endif // UNIFIED_REPORT_HPP
