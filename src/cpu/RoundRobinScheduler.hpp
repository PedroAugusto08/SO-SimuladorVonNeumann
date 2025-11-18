#ifndef ROUND_ROBIN_SCHEDULER_HPP
#define ROUND_ROBIN_SCHEDULER_HPP

#include <deque>
#include <vector>
#include <mutex>
#include <memory>
#include <cstdint>
#include "Core.hpp"
#include "PCB.hpp"

// Forward declarations for managers
class MemoryManager;
class IOManager;

class RoundRobinScheduler {
public:
    struct Statistics {
        double avg_wait_time{0.0};
        double avg_turnaround_time{0.0};
        double avg_cpu_utilization{0.0};
        double throughput{0.0};
        int total_context_switches{0};
    };

    RoundRobinScheduler(int num_cores,
                        MemoryManager* mem_manager,
                        IOManager* io_manager,
                        int default_quantum = 100);
    ~RoundRobinScheduler();

    void add_process(PCB* process);
    void schedule_cycle();
    bool has_pending_processes() const;
    int get_finished_count() const { return finished_count.load(); }
    int get_total_count() const { return total_count.load(); }
    Statistics get_statistics() const;

private:
    std::vector<std::unique_ptr<Core>> cores;
    std::deque<PCB*> ready_queue;
    std::deque<PCB*> blocked_queue;
    std::vector<PCB*> finished_list;

    int default_quantum{100};
    int num_cores{0};

    // CRITICAL: Atomics para evitar race conditions
    std::atomic<int> finished_count{0};
    std::atomic<int> total_count{0};

    uint64_t current_time{0};
    
    // Otimizações de performance
    std::atomic<int> ready_count{0};
    std::atomic<int> idle_cores{0};
    int batch_size{5};  // Scheduling a cada 5 ciclos (bom balanço)
    int collect_interval{1};  // Coletar processos SEMPRE (crítico!)

    mutable std::mutex scheduler_mutex;

    MemoryManager* memory_manager{nullptr};
    IOManager* io_manager{nullptr};

    // helpers
    void assign_process_to_core(PCB* process, Core* core);
    void collect_finished_processes();
    void handle_blocked_processes();
    void update_wait_times();
};

#endif // ROUND_ROBIN_SCHEDULER_HPP
