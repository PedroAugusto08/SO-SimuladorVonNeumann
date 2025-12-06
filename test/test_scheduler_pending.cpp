#include <iostream>
#include <chrono>
#include <thread>
#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "memory/MemoryManager.hpp"
#include "IO/IOManager.hpp"
#include "cpu/pcb_loader.hpp"

int main() {
    MemoryManager mem(4096, 32768);
    IOManager io;

    // create a simple PCB
    // We'll create a fresh PCB for each scheduler test to avoid reusing `in_ready_queue`.

    // FCFS test: should be pending after adding process
    {
        FCFSScheduler sched(1, &mem, &io);
        PCB pcb1;
        pcb1.pid = 1;
        pcb1.name = "test_fcfs";
        pcb1.quantum = 10;
        pcb1.arrival_time = 0;
        pcb1.set_state(State::Ready);
        sched.add_process(&pcb1);
        if (!sched.has_pending_processes()) {
            std::cerr << "FCFS: expected pending processes after add_process()\n";
            return 1;
        }
        // Try to run until scheduler finishes
        std::cout << "[FCFS TEST] starting schedule loop\n";
        std::cout << "[FCFS TEST] has_pending_processes()=" << sched.has_pending_processes() << "\n";
        for (int i = 0; i < 500 && sched.has_pending_processes(); ++i) {
            sched.schedule_cycle();
            // print core states for debugging
            for (auto& cptr : sched.get_cores()) {
                std::cout << "[FCFS DEBUG] Core " << cptr->get_id() << ": is_idle=" << cptr->is_idle() \
                          << " has_process=" << (cptr->get_current_process() ? 1 : 0) << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (sched.has_pending_processes()) {
            std::cerr << "FCFS: expected no pending processes after schedule cycles\n";
            return 1;
        }
    }

    // SJN test
    {
        SJNScheduler sched(1, &mem, &io);
        PCB pcb2;
        pcb2.pid = 2;
        pcb2.name = "test_sjn";
        pcb2.quantum = 10;
        pcb2.arrival_time = 0;
        pcb2.set_state(State::Ready);
        sched.add_process(&pcb2);
        if (!sched.has_pending_processes()) {
            std::cerr << "SJN: expected pending processes after add_process()\n";
            return 1;
        }
        for (int i = 0; i < 500 && sched.has_pending_processes(); ++i) {
            sched.schedule_cycle();
        }
        if (sched.has_pending_processes()) {
            std::cerr << "SJN: expected no pending processes after schedule cycles\n";
            return 1;
        }
    }

    // Priority test
    {
        PriorityScheduler sched(1, &mem, &io);
        PCB pcb3;
        pcb3.pid = 3;
        pcb3.name = "test_priority";
        pcb3.quantum = 10;
        pcb3.arrival_time = 0;
        pcb3.set_state(State::Ready);
        sched.add_process(&pcb3);
        if (!sched.has_pending_processes()) {
            std::cerr << "PRIORITY: expected pending processes after add_process()\n";
            return 1;
        }
        for (int i = 0; i < 500 && sched.has_pending_processes(); ++i) {
            sched.schedule_cycle();
        }
        if (sched.has_pending_processes()) {
            std::cerr << "PRIORITY: expected no pending processes after schedule cycles\n";
            return 1;
        }
    }

    // RoundRobin test
    {
        RoundRobinScheduler sched(1, &mem, &io, 10);
        PCB pcb4;
        pcb4.pid = 4;
        pcb4.name = "test_rr";
        pcb4.quantum = 10;
        pcb4.arrival_time = 0;
        pcb4.set_state(State::Ready);
        sched.add_process(&pcb4);
        if (!sched.has_pending_processes()) {
            std::cerr << "RR: expected pending processes after add_process()\n";
            return 1;
        }
        // drain and wait a bit
        for (int i = 0; i < 200 && sched.has_pending_processes(); ++i) {
            sched.schedule_cycle();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (sched.has_pending_processes()) {
            std::cerr << "RR: expected no pending processes after schedule_cycle() iterations\n";
            return 1;
        }
    }

    std::cout << "All scheduler pending tests passed.\n";
    return 0;
}
