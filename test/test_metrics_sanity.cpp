#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <algorithm>

#include "cpu/FCFSScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/pcb_loader.hpp"
#include "memory/MemoryManager.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

namespace {

constexpr int NUM_CORES = 2;
constexpr int QUANTUM = 50;
constexpr int MAX_CYCLES = 500000;

struct TestResult {
    std::string policy;
    bool passed = false;
    std::string message;
};

PCB* create_test_pcb(int pid, const std::string& name, const std::string& tasks_file, MemoryManager& memManager, uint32_t segment_base) {
    auto pcb = new PCB();
    pcb->pid = pid;
    pcb->name = name;
    pcb->quantum = QUANTUM;
    pcb->priority = 5;
    pcb->segment_base_addr = segment_base;
    pcb->segment_limit = 2048; // Tamanho do segmento

    loadJsonProgram(tasks_file, memManager, *pcb, static_cast<int>(segment_base));

    pcb->estimated_job_size = pcb->program_size;
    pcb->set_state(State::Ready);
    return pcb;
}

template<typename SchedulerT, typename... Args>
void run_and_check_policy(std::vector<PCB*>& processes, Args&&... args) {
    SchedulerT scheduler(std::forward<Args>(args)...);

    for (auto* pcb : processes) {
        scheduler.add_process(pcb);
    }

    int cycles = 0;
    while (scheduler.has_pending_processes() && cycles < MAX_CYCLES) {
        scheduler.schedule_cycle();
        cycles++;
    }

    if (scheduler.has_pending_processes()) {
        throw std::runtime_error("Simulação atingiu o limite de ciclos.");
    }

    auto stats = scheduler.get_statistics();

    // Sanity checks
    if (stats.total_processes != static_cast<int>(processes.size())) {
        throw std::runtime_error("Número de processos finalizados incorreto.");
    }
    if (stats.avg_cpu_utilization < 0.0 || stats.avg_cpu_utilization > 100.0) {
        throw std::runtime_error("Utilização de CPU fora do intervalo [0, 100].");
    }
    // Aumentando o limite para acomodar execuções muito rápidas
    if (stats.throughput < 0.0 || stats.throughput > 100000.0) { 
        throw std::runtime_error(std::string("Throughput com valor irrealista: ") + std::to_string(stats.throughput));
    }
    if (stats.avg_wait_time < 0.0 || stats.avg_turnaround_time < 0.0) {
        throw std::runtime_error("Tempos médios não podem ser negativos.");
    }
}

TestResult run_sanity_check(const std::string& policy) {
    TestResult result;
    result.policy = policy;

    auto memManager = std::make_unique<MemoryManager>(4096, 8192);
    auto ioManager = std::make_unique<IOManager>();

    std::vector<PCB*> processes;
    processes.push_back(create_test_pcb(1, "proc1", "tasks/tasks_quick.json", *memManager, 0));
    processes.push_back(create_test_pcb(2, "proc2", "tasks/tasks_short.json", *memManager, 2048));

    try {
        if (policy == "RR") {
            run_and_check_policy<RoundRobinScheduler>(processes, NUM_CORES, memManager.get(), ioManager.get(), QUANTUM);
        } else if (policy == "FCFS") {
            run_and_check_policy<FCFSScheduler>(processes, NUM_CORES, memManager.get(), ioManager.get());
        } else if (policy == "SJN") {
            run_and_check_policy<SJNScheduler>(processes, NUM_CORES, memManager.get(), ioManager.get());
        } else if (policy == "PRIORITY") {
            run_and_check_policy<PriorityScheduler>(processes, NUM_CORES, memManager.get(), ioManager.get());
        } else {
            throw std::runtime_error("Política desconhecida: " + policy);
        }

        result.passed = true;
        result.message = "OK";

    } catch (const std::exception& e) {
        result.passed = false;
        result.message = e.what();
    }

    for (auto* pcb : processes) {
        delete pcb;
    }

    return result;
}

} // namespace

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         TESTE DE SANIDADE DAS MÉTRICAS DO ESCALONADOR        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    const std::vector<std::string> policies = {"RR", "FCFS", "SJN", "PRIORITY"};
    std::vector<TestResult> results;
    int failed_tests = 0;

    for (const auto& policy : policies) {
        std::cout << "  → Testando política: " << policy << "..." << std::flush;
        TestResult res = run_sanity_check(policy);
        results.push_back(res);
        if (res.passed) {
            std::cout << " ✅ PASSOU\n";
        } else {
            std::cout << " ❌ FALHOU\n";
            std::cerr << "    Motivo: " << res.message << "\n";
            failed_tests++;
        }
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Resumo dos Testes de Sanidade:\n";
    for (const auto& res : results) {
        std::cout << "  • " << res.policy << ": " << (res.passed ? "✅ PASSOU" : "❌ FALHOU") << "\n";
    }
    std::cout << std::string(60, '=') << "\n\n";

    if (failed_tests > 0) {
        std::cerr << "🚨 " << failed_tests << " teste(s) de sanidade falharam.\n";
        return 1;
    }

    std::cout << "✅ Todos os testes de sanidade passaram com sucesso!\n";
    return 0;
}