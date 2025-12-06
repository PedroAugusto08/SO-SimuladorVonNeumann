#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "IO/IOManager.hpp"
#include "cpu/PCB.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/pcb_loader.hpp"
#include "memory/MemoryManager.hpp"
#include "parser_json/parser_json.hpp"

namespace {

namespace fs = std::filesystem;

struct WorkloadEntry {
    std::string key;
    std::string process_path;
    std::string task_path;
};

std::vector<WorkloadEntry> load_workloads(const std::string& process_dir,
                                          const std::string& tasks_dir) {
    std::vector<WorkloadEntry> workloads;
    if (!fs::exists(process_dir) || !fs::exists(tasks_dir)) {
        return workloads;
    }

    const std::string prefix = "process_";
    std::vector<fs::path> process_files;
    for (const auto& entry : fs::directory_iterator(process_dir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;
        const std::string stem = entry.path().stem().string();
        if (stem.rfind(prefix, 0) != 0) continue;
        process_files.push_back(entry.path());
    }

    std::sort(process_files.begin(), process_files.end());
    for (const auto& process_path : process_files) {
        const std::string stem = process_path.stem().string();
        const std::string suffix = stem.substr(prefix.size());
        if (suffix == "loop_heavy") {
            continue;
        }
        fs::path tasks_path = fs::path(tasks_dir) / ("tasks_" + suffix + ".json");
        if (!fs::exists(tasks_path)) {
            std::cerr << "⚠️  Workload ignorado: ausência de " << tasks_path
                      << " correspondente a " << process_path << "\n";
            continue;
        }
        workloads.push_back({suffix, process_path.string(), tasks_path.string()});
    }

    return workloads;
}

} // namespace

int main() {
    std::cout << "\n==============================================================\n";
    std::cout << "  TESTE: Round Robin single-core (sem threads explícitas)\n";
    std::cout << "==============================================================\n\n";

    constexpr int MAX_CYCLES_PER_PROCESS = 2'000'000;
    constexpr int IO_DRAIN_EXTRA_CYCLES = 500'000;
    constexpr int IO_DRAIN_SLEEP_MS = 5;
    constexpr int ROUND_ROBIN_QUANTUM = 10;
    const std::string process_dir = "processes";
    const std::string tasks_dir = "tasks";

    const auto workloads = load_workloads(process_dir, tasks_dir);
    if (workloads.empty()) {
        std::cerr << "❌ Nenhum par process/tasks encontrado em '" << process_dir
                  << "' e '" << tasks_dir << "'.\n";
        return 1;
    }

    MemoryManager memory_manager(4096, 32768);
    IOManager io_manager;
    (void)io_manager;

    RoundRobinScheduler scheduler(1, &memory_manager, &io_manager, ROUND_ROBIN_QUANTUM);

    std::vector<std::unique_ptr<PCB>> processes;
    processes.reserve(workloads.size());

    std::cout << "Carregando processos (" << workloads.size() << ")...\n";
    for (size_t i = 0; i < workloads.size(); ++i) {
        const auto& workload = workloads[i];
        auto pcb = std::make_unique<PCB>();
        if (!load_pcb_from_json(workload.process_path.c_str(), *pcb)) {
            std::cerr << "⚠️  Falha ao carregar " << workload.process_path
                      << ". Abortando." << std::endl;
            return 1;
        }

        int start_addr = static_cast<int>(i) * 2048;
        int end_addr = loadJsonProgram(workload.task_path, memory_manager, *pcb, start_addr);
        pcb->program_start_addr = start_addr;
        pcb->program_size = end_addr - start_addr;
        pcb->regBank.pc.write(start_addr);
        pcb->state = State::Ready;
        pcb->pid = static_cast<int>(i + 1);
        pcb->name = workload.key;
        pcb->quantum = ROUND_ROBIN_QUANTUM;

        std::cout << "  • " << workload.key << ": P" << pcb->pid
                  << " start=" << start_addr
                  << " size=" << pcb->program_size << " bytes" << std::endl;

        scheduler.add_process(pcb.get());
        processes.push_back(std::move(pcb));
    }

    auto all_finished = [&]() {
        for (const auto& pcb : processes) {
            if (pcb->state != State::Finished) {
                return false;
            }
        }
        return true;
    };

    std::cout << "\nExecutando Round Robin em um único core...\n";
    const int max_total_cycles = MAX_CYCLES_PER_PROCESS * static_cast<int>(workloads.size());
    int rr_cycles = 0;
    while (!all_finished() && rr_cycles < max_total_cycles) {
        if (!scheduler.has_pending_processes()) {
            break;
        }
        scheduler.schedule_cycle();
        ++rr_cycles;
    }

    if (!all_finished() && scheduler.has_pending_processes()) {
        std::cout << "[SingleCore/RR] Aguardando conclusão de I/O pendente...\n";
        int drain_cycles = 0;
        while (!all_finished() && drain_cycles < IO_DRAIN_EXTRA_CYCLES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(IO_DRAIN_SLEEP_MS));
            scheduler.schedule_cycle();
            ++rr_cycles;
            ++drain_cycles;
        }
    }

    const bool hit_budget = rr_cycles >= max_total_cycles && !all_finished();
    const bool scheduler_pending = scheduler.has_pending_processes();

    if (hit_budget && scheduler_pending) {
        std::cerr << "[SingleCore/RR] Limite de ciclos (" << max_total_cycles
                  << ") atingido com processos pendentes.\n";
    }

    int finished_pcbs = 0;
    int failed_pcbs = 0;
    std::vector<std::string> pending_labels;
    for (const auto& pcb : processes) {
        if (pcb->state == State::Finished) {
            finished_pcbs++;
            if (pcb->failed.load()) {
                failed_pcbs++;
            }
        } else {
            pending_labels.push_back(pcb->name.empty() ? ("P" + std::to_string(pcb->pid)) : pcb->name);
        }
    }

    const auto stats = scheduler.get_statistics();


    auto print_metric = [](const std::string& label, double value, int precision, const std::string& unit) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        if (!unit.empty()) {
            oss << " " << unit;
        }
        std::cout << "| " << std::left << std::setw(32) << label
                  << " | " << std::right << std::setw(13) << oss.str() << " |\n";
    };

    std::cout << "\n+----------------------------------+---------------+\n";
    std::cout << "| Métrica                          | Valor         |\n";
    std::cout << "+----------------------------------+---------------+\n";
    print_metric("Tempo médio de espera", stats.avg_wait_time, 2, "ms");
    print_metric("Tempo médio de execução", stats.avg_turnaround_time, 2, "ms");
    print_metric("Utilização média da CPU", stats.avg_cpu_utilization, 2, "%");
    print_metric("Throughput", stats.throughput, 4, "proc/s");
    print_metric("Eficiência", stats.avg_cpu_utilization, 2, "%");
    std::cout << "+----------------------------------+---------------+\n";

    if (!pending_labels.empty()) {
        std::cout << "Pendentes: ";
        for (size_t i = 0; i < pending_labels.size(); ++i) {
            std::cout << pending_labels[i];
            if (i + 1 < pending_labels.size()) {
                std::cout << ", ";
            }
        }
        std::cout << "\n";
    }
    if (finished_pcbs == static_cast<int>(workloads.size()) && failed_pcbs == 0 && !hit_budget) {
        std::cout << "  ✅ Todos os processos concluídos via Round Robin!" << std::endl;
    } else {
        std::cout << "  ⚠️  Nem todos os processos finalizaram. Verifique o log." << std::endl;
    }

    std::cout << "==============================================================\n\n";
    const bool success = (finished_pcbs == static_cast<int>(workloads.size())) && failed_pcbs == 0 && !hit_budget;
    return success ? 0 : 2;
}
