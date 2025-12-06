#pragma once
#include <vector>
#include <deque>
#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "PCB.hpp"
#include "Core.hpp"
#include "SchedulerBase.hpp" // Incluir a classe base
#include "../IO/IOManager.hpp"
#include "memory/MemoryManager.hpp"
#include "Constants.hpp"

class FCFSScheduler : public SchedulerBase {
public:
    FCFSScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager);
    ~FCFSScheduler() override;
    void add_process(PCB* process) override;
    void schedule_cycle() override;
    Statistics get_statistics() const override;

    // Métodos específicos do FCFS
    void initialize_cores();
    void shutdown_cores();
    void drain_cores();
    bool all_finished() const override;
    bool has_pending_processes() const override;
    int get_failed_count() const { return failed_count.load(); }
    std::vector<std::unique_ptr<Core>>& get_cores();
    void dump_state(const std::string& reason, uint64_t cycles, uint64_t max_cycles) const;
    
private:
    int num_cores;
    MemoryManager* memManager;
    IOManager* ioManager;
    std::vector<std::unique_ptr<Core>> cores;
    std::deque<PCB*> ready_queue;
    std::vector<PCB*> blocked_list;
    std::vector<PCB*> finished_list;
    mutable std::mutex ready_queue_mutex;
    
    std::atomic<int> failed_count{0};
    uint64_t start_ns{0};
    std::atomic<uint64_t> total_simulation_cycles{0};
    std::atomic<uint64_t> total_execution_time{0}; // Re-adicionado
    
    std::atomic<int> ready_count{0};
    std::atomic<int> idle_cores_count{0};
    int batch_size{5}; // Re-adicionado
    mutable std::mutex scheduler_mutex;
    
    int context_switches{0};
    std::unordered_map<int, PCB*> process_registry;
    
    // Helpers
    void collect_finished_processes();
    void enqueue_ready_process(PCB* process);
    PCB* dequeue_ready_process();
    size_t ready_queue_size() const;
    bool ready_queue_empty() const;
};
