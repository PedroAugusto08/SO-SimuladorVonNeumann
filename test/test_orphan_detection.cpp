#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/pcb_loader.hpp"
#include "parser_json/parser_json.hpp"
#include "memory/MemoryManager.hpp"
#include "IO/IOManager.hpp"
#include "util/Log.hpp"

using namespace std;

static vector<string> default_process_files() {
    return {
        "processes/process_io_bound.json",
        "processes/process_quick.json",
        "processes/process_short.json",
    };
}

int run_orphan_test_on_scheduler(const string& name, int num_cores) {
    MemoryManager mem(4096, 32768);
    IOManager io;

    vector<PCB*> pcbs;

    auto files = default_process_files();
    for (size_t i = 0; i < files.size(); ++i) {
        auto pcb = new PCB();
        if (!load_pcb_from_json(files[i], *pcb)) {
            cerr << "Falha ao carregar PCB: " << files[i] << "\n";
            delete pcb;
            continue;
        }
        pcb->pid = static_cast<int>(i + 1);
        pcb->name = "P" + to_string(pcb->pid);
        pcb->arrival_time = 0;
        pcb->quantum = 10;
        // Load program to memory
        try {
            loadJsonProgram(files[i], mem, *pcb, static_cast<int>(i * 2048));
        } catch (const std::exception &ex) {
            cerr << "Falha loadJsonProgram: " << ex.what() << "\n";
            delete pcb;
            continue;
        }
        pcbs.push_back(pcb);
    }

    if (pcbs.empty()) {
        cerr << "Nenhum PCB foi carregado.\n";
        return 2;
    }

    if (name == "FCFS") {
        FCFSScheduler sched(num_cores, &mem, &io);
        for (auto *p : pcbs) sched.add_process(p);

        int max_cycles = 100000;
        int c = 0;
        while (sched.has_pending_processes() && c < max_cycles) {
            sched.schedule_cycle();
            ++c;
            this_thread::sleep_for(chrono::milliseconds(0));
        }

    } else if (name == "SJN") {
        SJNScheduler sched(num_cores, &mem, &io);
        for (auto *p : pcbs) sched.add_process(p);
        int max_cycles = 100000;
        int c = 0;
        while (sched.has_pending_processes() && c < max_cycles) {
            sched.schedule_cycle();
            ++c;
        }
    } else if (name == "PRIORITY") {
        PriorityScheduler sched(num_cores, &mem, &io);
        for (auto *p : pcbs) sched.add_process(p);
        int max_cycles = 100000;
        int c = 0;
        while (sched.has_pending_processes() && c < max_cycles) {
            sched.schedule_cycle();
            ++c;
        }
    } else if (name == "RR") {
        RoundRobinScheduler sched(num_cores, &mem, &io, 10);
        for (auto *p : pcbs) sched.add_process(p);
        int max_cycles = 100000;
        int c = 0;
        while (sched.has_pending_processes() && c < max_cycles) {
            sched.schedule_cycle();
            ++c;
        }
    } else {
        cerr << "Política desconhecida: " << name << "\n";
        for (auto*p : pcbs) delete p;
        return 2;
    }

    // After run, verify invariant by scanning PCBs states
    int total = static_cast<int>(pcbs.size());
    int finished = 0; int ready = 0; int blocked = 0; int running = 0; int failed = 0;
    for (auto* p : pcbs) {
        State s = p->get_state();
        switch (s) {
            case State::Finished: finished++; break;
            case State::Ready: ready++; break;
            case State::Blocked: blocked++; break;
            case State::Running: running++; break;
            case State::Failed: failed++; break;
        }
    }

    cout << "Resultado da política " << name << " em " << num_cores << " core(s): total=" << total
         << " finished=" << finished << " ready=" << ready << " blocked=" << blocked << " running=" << running << " failed=" << failed << "\n";

    if (finished + ready + blocked + running + failed != total) {
        cerr << "INVARIANTE FALHOU: sum=" << (finished + ready + blocked + running + failed) << " total=" << total << "\n";
        for (auto* p : pcbs) {
            cerr << "   -> P" << p->pid << " state=" << static_cast<int>(p->get_state()) << " assigned_core=" << p->assigned_core << "\n";
        }
        for (auto* p : pcbs) delete p;
        return 1;
    }

    for (auto* p : pcbs) delete p;
    return 0;
}

int main() {
    Log::init_from_env();
    vector<string> policies = {"FCFS", "SJN", "PRIORITY", "RR"};
    int num_cores = 3;
    int errors = 0;
    for (auto& pol : policies) {
        cout << "Verificando política: " << pol << "\n";
        int rc = run_orphan_test_on_scheduler(pol, num_cores);
        if (rc != 0) {
            cerr << "Teste falhou para política " << pol << " (rc=" << rc << ")\n";
            errors++;
        }
    }
    if (errors == 0) {
        cout << "Todos os testes de órfão passaram.\n";
    }
    return errors;
}
