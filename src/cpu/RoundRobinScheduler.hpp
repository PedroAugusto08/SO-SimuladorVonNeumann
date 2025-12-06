#ifndef ROUND_ROBIN_SCHEDULER_HPP
#define ROUND_ROBIN_SCHEDULER_HPP

#include <deque>
#include <vector>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "Core.hpp"
#include "PCB.hpp"
#include "SchedulerBase.hpp" // Incluir a classe base

// Forward declarations for managers
class MemoryManager;
class IOManager;

<<<<<<< Updated upstream
class RoundRobinScheduler {
public:
    struct Statistics {
        double avg_wait_time{0.0};
        double avg_turnaround_time{0.0};
        double avg_cpu_utilization{0.0};
        double throughput{0.0};
        int total_context_switches{0};
    };

=======
#include "Constants.hpp"

class RoundRobinScheduler : public SchedulerBase {
public:
>>>>>>> Stashed changes
    RoundRobinScheduler(int num_cores,
                        MemoryManager* mem_manager,
                        IOManager* io_manager,
                        int default_quantum = 100);
    ~RoundRobinScheduler() override;

<<<<<<< Updated upstream
    void add_process(PCB* process);
    void schedule_cycle();
    bool has_pending_processes() const;
    int get_finished_count() const { return finished_count; }
    int get_total_count() const { return total_count; }
    Statistics get_statistics() const;
=======
    void add_process(PCB* process) override;
    void schedule_cycle() override;
    bool has_pending_processes() const override;
    int get_failed_count() const { return failed_count.load(); }
    Statistics get_statistics() const override;
>>>>>>> Stashed changes

private:
    std::vector<std::unique_ptr<Core>> cores;
    std::deque<PCB*> ready_queue;
    std::deque<PCB*> blocked_queue;
    std::vector<PCB*> finished_list;

    int default_quantum{100};
    int num_cores{0};

<<<<<<< Updated upstream
    int finished_count{0};
    int total_count{0};

    uint64_t current_time{0};
=======
    // CRITICAL: Atomics para evitar race conditions
    std::atomic<int> failed_count{0}; // Mantido por enquanto

    uint64_t current_time{0};
    std::atomic<uint64_t> total_simulation_cycles{0};
    
    // Otimizações de performance
    std::atomic<int> ready_count{0};
    std::atomic<int> idle_cores{0};
    int batch_size{5};
    int collect_interval{1};
>>>>>>> Stashed changes

    mutable std::mutex scheduler_mutex;

    MemoryManager* memory_manager{nullptr};
    IOManager* io_manager{nullptr};
<<<<<<< Updated upstream
=======
    
    // Métricas globais
    std::atomic<uint64_t> global_context_switches{0};
    uint64_t start_ns{0}; // Substitui simulation_start
>>>>>>> Stashed changes

    // helpers
    void assign_process_to_core(PCB* process, Core* core);
    void collect_finished_processes();
    void handle_blocked_processes();
<<<<<<< Updated upstream
    void update_wait_times();
=======
    void enqueue_ready_process(PCB* process);
    
    // Registry to track all processes added to the scheduler for debugging/invariant checks
    std::unordered_map<int, PCB*> process_registry;
>>>>>>> Stashed changes
};

#endif // ROUND_ROBIN_SCHEDULER_HPP
