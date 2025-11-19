#pragma once
#include <vector>
#include <deque>
#include <memory>
#include "PCB.hpp"
#include "Core.hpp"
#include "../IO/IOManager.hpp"
#include "memory/MemoryManager.hpp"

class SJNScheduler {
public:
    SJNScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager);
    void add_process(PCB* process);
    void schedule_cycle();
    bool all_finished() const;
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
};
