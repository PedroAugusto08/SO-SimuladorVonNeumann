#ifndef SCHEDULER_BASE_HPP
#define SCHEDULER_BASE_HPP

#include <algorithm>
#include <atomic>
#include <vector>
#include <deque>
#include <limits>
#include "Core.hpp"
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
    
    // Common statistics calculator
    Statistics calculate_statistics(
        const std::vector<PCB*>& finished_list,
        const std::vector<std::unique_ptr<Core>>& cores,
        uint64_t simulation_start_ns,
        int context_switches = 0) const
    {
        Statistics stats;
        if (finished_list.empty()) return stats;

        uint64_t total_wait_ns = 0;
        uint64_t total_turnaround_ns = 0;
        uint64_t total_response_ns = 0;
        uint64_t earliest_arrival = std::numeric_limits<uint64_t>::max();
        uint64_t latest_finish = 0;

        int non_failed_finished = 0;
        for (const auto* pcb : finished_list) {
            if (pcb->get_state() == State::Failed) continue;
            non_failed_finished++;

            // total_wait: prefer host-based ns value if present, otherwise use simulated cycles
            const uint64_t wait_ns_host = pcb->total_wait_time.load();
            const uint64_t wait_sim = pcb->total_wait_sim_time.load();
            uint64_t wait_ns = 0;
            if (wait_ns_host > 0) {
                wait_ns = wait_ns_host;
            } else if (wait_sim > 0) {
                wait_ns = cpu_time::cycles_to_ns(wait_sim);
            }
            total_wait_ns += wait_ns;

            // turnaround: prefer host-based ns value, otherwise simulated cycles
            const uint64_t tat_host = pcb->get_turnaround_time();
            const uint64_t tat_sim = pcb->get_turnaround_sim_time();
            uint64_t tat_ns = 0;
            if (tat_host > 0) {
                tat_ns = tat_host;
            } else if (tat_sim > 0) {
                tat_ns = cpu_time::cycles_to_ns(tat_sim);
            }
            total_turnaround_ns += tat_ns;

            // response time: prefer host-based timestamps (ns) otherwise simulated
            const uint64_t start_time = pcb->start_time.load();
            const uint64_t arrival = pcb->arrival_time.load();
            const uint64_t start_sim = pcb->start_sim_time.load();
            const uint64_t arrival_sim = pcb->arrival_sim_time.load();
            if (start_time > 0 && arrival > 0 && start_time >= arrival) {
                total_response_ns += (start_time - arrival);
            } else if (start_sim > 0 && arrival_sim > 0 && start_sim >= arrival_sim) {
                total_response_ns += cpu_time::cycles_to_ns(start_sim - arrival_sim);
            }

            // Track earliest arrival and latest finish in host-time ns (fallback from sim-time)
            uint64_t arrival_ns_val = arrival;
            if (arrival_ns_val == 0 && arrival_sim > 0) arrival_ns_val = cpu_time::cycles_to_ns(arrival_sim);
            if (arrival_ns_val > 0) earliest_arrival = std::min(earliest_arrival, arrival_ns_val);

            uint64_t finish_ns_val = pcb->finish_time.load();
            if (finish_ns_val == 0 && pcb->finish_sim_time.load() > 0) finish_ns_val = cpu_time::cycles_to_ns(pcb->finish_sim_time.load());
            if (finish_ns_val > 0) latest_finish = std::max(latest_finish, finish_ns_val);
        }

        stats.total_processes = non_failed_finished;
        if (stats.total_processes == 0) return stats;

        const double inv_count = 1.0 / stats.total_processes;
        stats.avg_wait_time = cpu_time::ns_to_ms(static_cast<double>(total_wait_ns) * inv_count);
        stats.avg_turnaround_time = cpu_time::ns_to_ms(static_cast<double>(total_turnaround_ns) * inv_count);
        stats.avg_response_time = cpu_time::ns_to_ms(static_cast<double>(total_response_ns) * inv_count);
        
        const uint64_t span_ns = (latest_finish > earliest_arrival) ? (latest_finish - earliest_arrival) : 0;
        const double sim_elapsed_seconds = cpu_time::ns_to_seconds(span_ns);
        const double wall_elapsed_seconds = cpu_time::ns_to_seconds(cpu_time::now_ns() - simulation_start_ns);

        // Prefer pipeline cycle counts for CPU utilization (base approach)
        uint64_t total_pipeline_cycles = 0;
        for (const auto* pcb : finished_list) {
            total_pipeline_cycles += pcb->pipeline_cycles.load();
        }

        uint64_t total_busy_cycles = 0;
        uint64_t total_idle_cycles = 0;
        for (const auto& core : cores) {
            total_busy_cycles += core->get_busy_cycles();
            total_idle_cycles += core->get_idle_cycles();
        }

        const double busy_seconds = static_cast<double>(total_busy_cycles) / CLOCK_FREQ_HZ;
        const double busy_based_sim_elapsed_seconds = (cores.size() > 0) ? (busy_seconds / static_cast<double>(cores.size())) : busy_seconds;

        const double elapsed_seconds_raw = std::max({sim_elapsed_seconds, busy_based_sim_elapsed_seconds, wall_elapsed_seconds});
        const double min_elapsed_seconds = 1e-3; // 1 millisecond
        const double elapsed_seconds = (elapsed_seconds_raw < min_elapsed_seconds) ? min_elapsed_seconds : elapsed_seconds_raw;

        if (elapsed_seconds > 0.0) {
            stats.throughput = stats.total_processes / elapsed_seconds;
            // Compute CPU utilization using pipeline cycles (base method)
            const uint64_t total_cycles_estimated = total_pipeline_cycles + (stats.total_processes * 100);
            if (total_cycles_estimated > 0) {
                stats.avg_cpu_utilization = (100.0 * static_cast<double>(total_pipeline_cycles) / static_cast<double>(total_cycles_estimated));
            } else {
                const uint64_t capacity_cycles = total_busy_cycles + total_idle_cycles;
                if (capacity_cycles > 0) {
                    stats.avg_cpu_utilization = (static_cast<double>(total_busy_cycles) / capacity_cycles) * 100.0;
                } else {
                    stats.avg_cpu_utilization = 0.0;
                }
            }
        }
        
        stats.total_context_switches = context_switches;
        
        return stats;
    }
};

#endif // SCHEDULER_BASE_HPP
