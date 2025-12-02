#pragma once
#include <vector>
#include <deque>
#include <memory>
#include <atomic>
#include "PCB.hpp"
#include "Core.hpp"
#include "../IO/IOManager.hpp"
#include "memory/MemoryManager.hpp"
#include "Constants.hpp"

class SJNScheduler {
public:
    struct Statistics {
        double avg_wait_time{0.0};
        double avg_turnaround_time{0.0};
        double avg_response_time{0.0};
        double avg_cpu_utilization{0.0};
        double throughput{0.0};
        int total_context_switches{0};
        int total_processes{0};
    };

    SJNScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager);
    void add_process(PCB* process);
    void schedule_cycle();
    bool all_finished() const;
    int get_finished_count() const { return finished_count.load(); }
    int get_total_count() const { return total_count.load(); }
    Statistics get_statistics() const;
    std::vector<std::unique_ptr<Core>>& get_cores();
    std::deque<PCB*>& get_ready_queue();
    std::vector<PCB*>& get_blocked_list();
    
private:
    int num_cores;
    MemoryManager* memManager;
    IOManager* ioManager;
    std::vector<std::unique_ptr<Core>> cores;
    std::deque<PCB*> ready_queue;
    std::vector<PCB*> blocked_list;
    std::vector<PCB*> finished_list;
    
    std::atomic<int> finished_count{0};
    std::atomic<int> total_count{0};
    std::atomic<uint64_t> total_execution_time{0};
    std::chrono::steady_clock::time_point simulation_start_time;  // 🆕 Tempo real
    std::atomic<uint64_t> total_simulation_cycles{0};  // 🆕 Total de ciclos da simulação
    int context_switches{0};  // Contador de trocas de contexto
};
