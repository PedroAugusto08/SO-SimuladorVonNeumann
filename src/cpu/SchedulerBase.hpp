#ifndef SCHEDULER_BASE_HPP
#define SCHEDULER_BASE_HPP

#include <atomic>
#include <vector>
#include <deque>
#include "PCB.hpp"

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

        uint64_t total_wait = 0;
        uint64_t total_turnaround = 0;
        uint64_t total_response = 0;

        for (const auto& pcb : finished_processes) {
            total_wait += pcb->get_wait_time();
            total_turnaround += pcb->get_turnaround_time();
            // Response time = tempo até primeira execução
            if (pcb->start_time.load() > 0) {
                total_response += (pcb->start_time.load() - pcb->arrival_time.load());
            }
        }

        stats.total_processes = finished_processes.size();
        stats.avg_wait_time = static_cast<double>(total_wait) / stats.total_processes;
        stats.avg_turnaround_time = static_cast<double>(total_turnaround) / stats.total_processes;
        stats.avg_response_time = static_cast<double>(total_response) / stats.total_processes;
        
        // Throughput: processos por unidade de tempo
        uint64_t elapsed_time = total_execution_time - simulation_start_time;
        if (elapsed_time > 0) {
            stats.throughput = static_cast<double>(stats.total_processes) / elapsed_time;
        }
        
        // Utilização da CPU (estimativa)
        if (elapsed_time > 0) {
            uint64_t total_cpu_time = 0;
            for (const auto& pcb : finished_processes) {
                total_cpu_time += pcb->pipeline_cycles.load();
            }
            stats.avg_cpu_utilization = static_cast<double>(total_cpu_time) / elapsed_time;
        }
        
        stats.total_context_switches = context_switches;
        
        return stats;
    }
};

#endif // SCHEDULER_BASE_HPP
