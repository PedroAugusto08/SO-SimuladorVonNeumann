#ifndef SCHEDULER_BASE_HPP
#define SCHEDULER_BASE_HPP

#include <atomic>
#include <vector>
#include <deque>
#include <limits>
#include "PCB.hpp"
#include "Constants.hpp"
#include "TimeUtils.hpp"

/**
 * SchedulerBase - Interface comum para todos os escalonadores
 * 
 * Fornece estrutura e métodos compartilhados entre:
 * - RoundRobinScheduler
 * - FCFSScheduler
 * - SJNScheduler
 * - PriorityScheduler
 * 
 * Objetivo: Reduzir duplicação de código e garantir consistência
 */
class SchedulerBase {
public:
    /**
     * Estrutura de métricas de desempenho do escalonador
     * Compartilhada por todos os tipos de escalonadores
     */
    struct Statistics {
        double avg_wait_time{0.0};
        double avg_turnaround_time{0.0};
        double avg_response_time{0.0};
        double avg_cpu_utilization{0.0};
        double throughput{0.0};
        int total_context_switches{0};
        int total_processes{0};
    };

    virtual ~SchedulerBase() = default;

    // Interface obrigatória para todos os escalonadores
    virtual void add_process(PCB* process) = 0;
    virtual void schedule_cycle() = 0;
    virtual Statistics get_statistics() const = 0;
    
    // Métodos comuns para controle de processos
    virtual int get_finished_count() const { return finished_count.load(); }
    virtual int get_total_count() const { return total_count.load(); }
    
    // Métodos de verificação de estado
    virtual bool has_pending_processes() const {
        return get_finished_count() < get_total_count();
    }
    
    virtual bool all_finished() const {
        return get_finished_count() >= get_total_count();
    }

protected:
    // Contadores atômicos compartilhados
    std::atomic<int> finished_count{0};
    std::atomic<int> total_count{0};
    
    // Calculador de estatísticas comum
    Statistics calculate_statistics(
        const std::vector<PCB*>& finished_processes,
        uint64_t total_execution_time,
        uint64_t simulation_start_time,
        int context_switches = 0) const
    {
        Statistics stats;
        if (finished_processes.empty()) return stats;

        uint64_t total_wait_ns = 0;
        uint64_t total_turnaround_ns = 0;
        uint64_t total_response_ns = 0;
        uint64_t earliest_arrival = std::numeric_limits<uint64_t>::max();
        uint64_t latest_finish = 0;
        uint64_t total_pipeline_cycles = 0;

        for (const auto& pcb : finished_processes) {
            total_wait_ns += pcb->get_wait_time();
            total_turnaround_ns += pcb->get_turnaround_time();
            // Response time = tempo até primeira execução
            const uint64_t start_time = pcb->start_time.load();
            const uint64_t arrival = pcb->arrival_time.load();
            if (start_time > 0 && start_time >= arrival) {
                total_response_ns += (start_time - arrival);
            }
            earliest_arrival = std::min(earliest_arrival, arrival);
            latest_finish = std::max(latest_finish, pcb->finish_time.load());
            total_pipeline_cycles += pcb->pipeline_cycles.load();
        }

        stats.total_processes = finished_processes.size();
        const double inv_count = 1.0 / stats.total_processes;
        stats.avg_wait_time = cpu_time::ns_to_ms(static_cast<double>(total_wait_ns) * inv_count);
        stats.avg_turnaround_time = cpu_time::ns_to_ms(static_cast<double>(total_turnaround_ns) * inv_count);
        stats.avg_response_time = cpu_time::ns_to_ms(static_cast<double>(total_response_ns) * inv_count);
        if (earliest_arrival == std::numeric_limits<uint64_t>::max()) {
            earliest_arrival = simulation_start_time;
        }
        
        uint64_t span_ns = 0;
        if (latest_finish > earliest_arrival) {
            span_ns = latest_finish - earliest_arrival;
        } else if (total_execution_time > simulation_start_time) {
            span_ns = total_execution_time - simulation_start_time;
        }
        const double elapsed_seconds = cpu_time::ns_to_seconds(span_ns);
        if (elapsed_seconds > 0.0) {
            stats.throughput = stats.total_processes / elapsed_seconds;
            const double busy_seconds = static_cast<double>(total_pipeline_cycles) / CLOCK_FREQ_HZ;
            const double capacity_seconds = elapsed_seconds * 1.0; // Assumir 1 core se não informado
            if (capacity_seconds > 0.0) {
                stats.avg_cpu_utilization = (busy_seconds / capacity_seconds) * 100.0;
            }
        }
        
        stats.total_context_switches = context_switches;
        
        return stats;
    }
};

#endif // SCHEDULER_BASE_HPP
