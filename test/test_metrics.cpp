#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "cpu/Core.hpp"
#include "cpu/FCFSScheduler.hpp"
#include "cpu/PriorityScheduler.hpp"
#include "cpu/RoundRobinScheduler.hpp"
#include "cpu/SJNScheduler.hpp"
#include "cpu/pcb_loader.hpp"
#include "memory/MemoryManager.hpp"
#include "parser_json/parser_json.hpp"
#include "IO/IOManager.hpp"

namespace fs = std::filesystem;

namespace {
constexpr int DEFAULT_NUM_CORES = 1;
constexpr int QUANTUM = 10;
constexpr int MAX_CYCLES = 2'000'000;
constexpr const char* PROCESS_DIR = "processes";
constexpr const char* TASKS_DIR = "tasks";
constexpr const char* DATA_ROOT = "dados_graficos";
constexpr const char* NORMALIZED_TASK_DIR = "output/normalized_tasks";
constexpr uint32_t SEGMENT_SIZE_BYTES = 2048;
std::string build_csv_path(int num_cores) {
    return std::string(DATA_ROOT) + "/csv/metricas_" +
           std::to_string(std::max(1, num_cores)) + "cores.csv";
}

std::string build_report_path(int num_cores) {
    return std::string(DATA_ROOT) + "/reports/relatorio_metricas_" +
           std::to_string(std::max(1, num_cores)) + "cores.txt";
}

int compute_cycle_budget([[maybe_unused]] int num_cores, size_t workload_count) {
    const double workload_scale = std::max(1.0, static_cast<double>(workload_count) / 8.0);
    // 2M ciclos base, ajustados pelo número de workloads (mas sem exagerar)
    const double budget = static_cast<double>(MAX_CYCLES) * workload_scale;
    // Permite sobrescrever o limite máximo via variável de ambiente para testes (ex: TEST_MAX_CYCLE_CAP=20000000)
    const char* env_cap = std::getenv("TEST_MAX_CYCLE_CAP");
    double cap = 5'000'000.0; // padrão
    if (env_cap) {
        try {
            const long long parsed = std::stoll(env_cap);
            if (parsed > 0) cap = static_cast<double>(parsed);
        } catch (...) {
            // valor inválido, mantém padrão
        }
    }
    const double capped = std::min(cap, budget);
    return static_cast<int>(capped);
}

struct WorkloadConfig {
    std::string key;
    std::string process_file;
    std::string tasks_file;
};

struct PolicyMetrics {
    std::string policy;
    double avg_wait_ms{0.0};
    double avg_exec_us{0.0};
    double avg_turnaround_ms{0.0};
    double cpu_util_pct{0.0};
    double efficiency_pct{0.0};
    double throughput{0.0};
    double execution_time_ms{0.0};
    long cache_hits{0};
    long cache_misses{0};
    double hit_rate_pct{0.0};
    int processes_finished{0};
    int processes_failed{0};
    bool success{false};
    std::string error;
};

std::vector<WorkloadConfig> load_workloads(const std::string& process_dir,
                                           const std::string& tasks_dir) {
    std::vector<WorkloadConfig> workloads;
    if (!fs::exists(process_dir) || !fs::exists(tasks_dir)) {
        return workloads;
    }

    const std::string prefix = "process_";
    std::vector<fs::path> process_files;
    for (const auto& entry : fs::directory_iterator(process_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".json") {
            continue;
        }
        const std::string stem = entry.path().stem().string();
        if (stem.rfind(prefix, 0) != 0) {
            continue;
        }
        process_files.push_back(entry.path());
    }

    std::sort(process_files.begin(), process_files.end());
    for (const auto& process_path : process_files) {
        const std::string stem = process_path.stem().string();
        const std::string suffix = stem.substr(prefix.size());
        if (suffix == "loop_heavy") {
            continue; // workload extremamente longo foi desabilitado nos testes automatizados
        }
        fs::path tasks_path = fs::path(tasks_dir) / ("tasks_" + suffix + ".json");
        if (!fs::exists(tasks_path)) {
            std::cerr << "⚠️  Workload ignorado: arquivo de tarefas não encontrado para '"
                      << stem << "' (esperado " << tasks_path << ")\n";
            continue;
        }
        workloads.push_back({suffix, process_path.string(), tasks_path.string()});
    }

    return workloads;
}

void print_workloads(const std::vector<WorkloadConfig>& workloads) {
    if (workloads.empty()) {
        std::cout << "  • Nenhum workload encontrado.\n";
        return;
    }
    for (const auto& workload : workloads) {
        const std::string label = workload.key.empty()
            ? fs::path(workload.process_file).stem().string()
            : workload.key;
        std::cout << "  • " << label << " -> " << workload.process_file
                  << " | " << workload.tasks_file << "\n";
    }
}

PolicyMetrics run_policy(const std::string& policy,
                         int num_cores,
                         const std::vector<WorkloadConfig>& workloads) {
    PolicyMetrics metrics;
    metrics.policy = policy;
    metrics.processes_finished = 0;
    const int cycle_budget = compute_cycle_budget(num_cores, workloads.size());
    int adjusted_cycle_budget = cycle_budget;

    std::vector<PCB*> process_ptrs;
    process_ptrs.reserve(workloads.size());

    try {
        MemoryManager::resetStats();
        auto memManager = std::make_unique<MemoryManager>(4096, 32768);
        auto ioManager = std::make_unique<IOManager>();

        for (size_t i = 0; i < workloads.size(); ++i) {
            const auto& workload = workloads[i];
            auto pcb = std::make_unique<PCB>();
            if (!load_pcb_from_json(workload.process_file.c_str(), *pcb)) {
                std::string reason = "Falha ao carregar " + workload.process_file;
                pcb->mark_failed(reason);
                metrics.processes_failed++;
                if (metrics.error.empty()) metrics.error = reason;
                continue;
            }
            pcb->pid = static_cast<int>(i + 1);
            pcb->name = workload.key.empty()
                ? ("P" + std::to_string(i + 1))
                : workload.key;
            pcb->quantum = QUANTUM;
            pcb->arrival_time = 0;
            pcb->set_state(State::Ready);
            const uint32_t segment_base = static_cast<uint32_t>(i * SEGMENT_SIZE_BYTES);
            pcb->segment_base_addr = segment_base;
            pcb->segment_limit = SEGMENT_SIZE_BYTES;
            try {
                loadJsonProgram(workload.tasks_file, *memManager, *pcb, static_cast<int>(segment_base));
            } catch (const std::exception &ex) {
                // Marcar o PCB como failed e não agendar
                pcb->mark_failed(ex.what());
                metrics.processes_failed++;
                if (metrics.error.empty()) metrics.error = ex.what();
                continue;
            }

            // Se nada foi carregado (program_size == 0), marcar como failed
            if (pcb->program_size == 0) {
                pcb->mark_failed("Empty program loaded");
                metrics.processes_failed++;
                if (metrics.error.empty()) metrics.error = "Empty program loaded";
                continue;
            }
            pcb->estimated_job_size = pcb->program_size;
            pcb->priority = 10 - (static_cast<int>(i) % 3) * 3;
            process_ptrs.push_back(pcb.release());
        }

        // Ajuste de ciclo para testes: permite aumentar orçamento com variável de ambiente
        const char* env_use_estimate = std::getenv("TEST_USE_ESTIMATE_BUDGET");
        if (env_use_estimate && std::string(env_use_estimate) == "1") {
            uint64_t total_est_size = 0;
            for (auto* p : process_ptrs) {
                if (p) total_est_size += static_cast<uint64_t>(p->estimated_job_size);
            }
            // Heurística: assumimos 100 ciclos por unidade de estimated_job_size
            const uint64_t heuristic_cycles = total_est_size * 100ULL;
            if (heuristic_cycles > static_cast<uint64_t>(adjusted_cycle_budget)) {
                const uint64_t clamped = std::min<uint64_t>(heuristic_cycles, 50'000'000ULL);
                adjusted_cycle_budget = static_cast<int>(clamped);
            }
        }
        auto start = std::chrono::high_resolution_clock::now();

        auto finalize_stats = [&](const auto& stats) {
            metrics.avg_wait_ms = stats.avg_wait_time;
            metrics.avg_exec_us = stats.avg_execution_time * 1000.0;
            metrics.avg_turnaround_ms = stats.avg_turnaround_time;
            metrics.cpu_util_pct = stats.avg_cpu_utilization;
            metrics.throughput = stats.throughput;
            if (metrics.throughput > 0.0 && metrics.avg_exec_us > 0.0 && num_cores > 0) {
                const double avg_exec_s = metrics.avg_exec_us / 1'000'000.0;
                const double occupancy = metrics.throughput * avg_exec_s; // processos simultâneos
                const double estimated_util = std::min(100.0, std::max(0.0, (occupancy / num_cores) * 100.0));
                metrics.cpu_util_pct = std::max(metrics.cpu_util_pct, estimated_util);
            }
            metrics.efficiency_pct = metrics.cpu_util_pct;
        };

        auto collect_memory = [&]() {
            metrics.cache_hits = memManager->getTotalCacheHits();
            metrics.cache_misses = memManager->getTotalCacheMisses();
            const long total = metrics.cache_hits + metrics.cache_misses;
            metrics.hit_rate_pct = total > 0 ? (metrics.cache_hits * 100.0 / total) : 0.0;
        };

        if (policy == "RR")
        {
            RoundRobinScheduler scheduler(num_cores, memManager.get(), ioManager.get(), QUANTUM);
            for (auto *pcb : process_ptrs)
            {
                scheduler.add_process(pcb);
            }
                int cycles = 0;
                while (scheduler.has_pending_processes() && cycles < adjusted_cycle_budget)
            {
                scheduler.schedule_cycle();
                ++cycles;
            }
            for (int i = 0; i < 100; ++i)
            {
                scheduler.schedule_cycle();
            }
            metrics.processes_finished = scheduler.get_finished_count();
            metrics.processes_failed += scheduler.get_failed_count();
            finalize_stats(scheduler.get_statistics());
        }
        else if (policy == "FCFS")
        {
            FCFSScheduler scheduler(num_cores, memManager.get(), ioManager.get());
            for (auto *pcb : process_ptrs)
            {
                scheduler.add_process(pcb);
            }

            int cycles = 0;
            while (scheduler.has_pending_processes() && cycles < adjusted_cycle_budget)
            {
                scheduler.schedule_cycle();
                ++cycles;
            }

            scheduler.drain_cores();

            metrics.processes_finished = scheduler.get_finished_count();
            metrics.processes_failed += scheduler.get_failed_count();
            const int total_processes = static_cast<int>(process_ptrs.size());
            const bool hit_budget = (cycles >= adjusted_cycle_budget);
            const bool still_pending = scheduler.has_pending_processes();

            if (hit_budget && still_pending && metrics.processes_finished < total_processes)
            {
                metrics.error = "FCFS atingiu o limite de ciclos antes de concluir todos os processos";
            }

            finalize_stats(scheduler.get_statistics());
        }
        else if (policy == "SJN")
        {
            SJNScheduler scheduler(num_cores, memManager.get(), ioManager.get());
            for (auto *pcb : process_ptrs)
            {
                scheduler.add_process(pcb);
            }

            int cycles = 0;
            while (scheduler.has_pending_processes() && cycles < adjusted_cycle_budget)
            {
                scheduler.schedule_cycle();
                ++cycles;
            }

            scheduler.drain_cores();

            metrics.processes_finished = scheduler.get_finished_count();
            metrics.processes_failed += scheduler.get_failed_count();
            const int total_processes = static_cast<int>(process_ptrs.size());
            const bool hit_budget = (cycles >= adjusted_cycle_budget);
            const bool still_pending = scheduler.has_pending_processes();

            if (hit_budget && still_pending && metrics.processes_finished < total_processes)
            {
                metrics.error = "SJN atingiu o limite de ciclos antes de concluir todos os processos";
            }

            finalize_stats(scheduler.get_statistics());
        }
        else if (policy == "PRIORITY")
        {
            PriorityScheduler scheduler(num_cores, memManager.get(), ioManager.get());
            for (auto *pcb : process_ptrs)
            {
                scheduler.add_process(pcb);
            }

            int cycles = 0;
            while (scheduler.has_pending_processes() && cycles < adjusted_cycle_budget)
            {
                scheduler.schedule_cycle();
                ++cycles;
            }

            scheduler.drain_cores();

            metrics.processes_finished = scheduler.get_finished_count();
            metrics.processes_failed += scheduler.get_failed_count();
            const int total_processes = static_cast<int>(process_ptrs.size());
            const bool hit_budget = (cycles >= adjusted_cycle_budget);
            const bool still_pending = scheduler.has_pending_processes();

            if (hit_budget && still_pending && metrics.processes_finished < total_processes)
            {
                metrics.error = "PRIORITY atingiu o limite de ciclos antes de concluir todos os processos";
            }

            finalize_stats(scheduler.get_statistics());
        }
        else
        {
            metrics.error = "Política não suportada: " + policy;
        }

        auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        metrics.execution_time_ms = static_cast<double>(duration.count());

        collect_memory();
        metrics.success = metrics.error.empty() && (metrics.processes_failed == 0);

    } catch (const std::exception& ex) {
        metrics.error = ex.what();
    }

    for (auto* pcb : process_ptrs) {
        delete pcb;
    }
    return metrics;
}

void ensure_directories() {
    const std::vector<std::string> dirs = {
        DATA_ROOT,
        std::string(DATA_ROOT) + "/csv",
        std::string(DATA_ROOT) + "/reports"
    };
    for (const auto& dir : dirs) {
        fs::create_directories(dir);
    }
}

void write_csv(const std::vector<PolicyMetrics>& results, const std::string& csv_path) {
    std::ofstream csv(csv_path);
    if (!csv.is_open()) {
        throw std::runtime_error(std::string("Não foi possível criar ") + csv_path);
    }

        csv << "Politica,TempoMedioEspera_ms,TempoMedioExecucao_us,TempoMedioTurnaround_ms,CPUUtilizacao_pct,"
            "Eficiencia_pct,Throughput_proc_s,CacheHits,CacheMisses,TaxaHit_pct,FailedProcesses,Success,Error\n";

    csv << std::fixed;
    for (const auto& result : results) {
        csv << result.policy << ","
            << std::setprecision(2) << result.avg_wait_ms << ","
            << std::setprecision(2) << result.avg_exec_us << ","
            << std::setprecision(2) << result.avg_turnaround_ms << ","
            << std::setprecision(2) << result.cpu_util_pct << ","
            << std::setprecision(2) << result.efficiency_pct << ","
            << std::setprecision(4) << result.throughput << ","
            << result.cache_hits << ","
            << result.cache_misses << ","
            << std::setprecision(2) << result.hit_rate_pct << ","
            << result.processes_failed << ","
            << std::boolalpha << result.success << ","
            << "\"" << result.error << "\"" << "\n";
    }
}

void write_report(const std::vector<PolicyMetrics>& results,
                  const std::vector<WorkloadConfig>& workloads,
                  int num_cores,
                  const std::string& report_path) {
    std::ofstream report(report_path);
    if (!report.is_open()) {
        throw std::runtime_error(std::string("Não foi possível criar ") + report_path);
    }

    report << "╔══════════════════════════════════════════════════════════════╗\n";
    report << "║  RELATÓRIO - MÉTRICAS POR POLÍTICA (" << num_cores << " CORE(S))  ║\n";
    report << "╚══════════════════════════════════════════════════════════════╝\n\n";

    report << "Workloads utilizados (" << workloads.size() << "):\n";
    for (const auto& workload : workloads) {
        const std::string label = workload.key.empty()
            ? fs::path(workload.process_file).stem().string()
            : workload.key;
        report << "  - " << label << "\n";
    }
    report << "\n";

    for (const auto& result : results) {
        report << "Política: " << result.policy << "\n";
        report << "  • Status: " << (result.success ? "OK" : "FALHA") << "\n";
        if (!result.success) {
            report << "  • Motivo do erro: " << result.error << "\n";
        }
        report << std::string(60, '-') << "\n";
        report << "[Métricas de Escalonamento]\n";
        report << std::fixed << std::setprecision(2);
        report << "  • Tempo médio de espera:     " << result.avg_wait_ms << " ms\n";
        report << "  • Tempo médio de execução:   " << result.avg_exec_us << " µs\n";
        report << "  • Tempo médio de retorno:    " << result.avg_turnaround_ms << " ms\n";
        report << "  • Utilização média da CPU:   " << result.cpu_util_pct << " %\n";
        report << "  • Eficiência estimada:       " << result.efficiency_pct << " %\n";
        report << "  • Throughput global:         " << result.throughput << " proc/s\n";
        report << "[Métricas de Memória]\n";
        report << "  • Cache hits:                " << result.cache_hits << "\n";
        report << "  • Cache misses:              " << result.cache_misses << "\n";
        report << "  • Taxa de hit:               " << result.hit_rate_pct << " %\n";
        report << "  • Processos com falha:      " << result.processes_failed << "\n\n";
    }

    report << "Relatório gerado automaticamente por test_metrics.cpp\n";
}

} // namespace

int main() {
    // Configure logging level from environment variable SIM_LOG_LEVEL
    Log::init_from_env();
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     TESTE DE MÉTRICAS POR NÚCLEOS FIXOS - MULTICORE        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    const int num_cores = DEFAULT_NUM_CORES;
    const std::vector<WorkloadConfig> workloads = load_workloads(PROCESS_DIR, TASKS_DIR);
    if (workloads.empty()) {
        std::cerr << "❌ Nenhum workload encontrado em '" << PROCESS_DIR
                  << "' ou '" << TASKS_DIR << "'.\n";
        return 1;
    }

    ensure_directories();

    std::cout << "Configuração atual:\n";
    std::cout << "  • Núcleos fixos: " << num_cores << "\n";
    std::cout << "  • Políticas: RR, FCFS, SJN, PRIORITY\n";
    std::cout << "  • Workloads: " << workloads.size() << "\n";
    std::cout << "  • Listagem:\n";
    print_workloads(workloads);
    const char* env_cap = std::getenv("TEST_MAX_CYCLE_CAP");
    if (env_cap) {
        std::cout << "  • TEST_MAX_CYCLE_CAP=" << env_cap << "\n";
    }
    
    // Warmup run: descarta a primeira execução para evitar cold start
    std::cout << "\n[Warmup] Executando teste de aquecimento (descartado)...\n";
    {
        PolicyMetrics warmup = run_policy("RR", num_cores, workloads);
        std::cout << "  → Warmup " << (warmup.success ? "ok" : "falhou") << " (descartado)\n";
    }
    
    std::cout << "\nExecutando métricas...\n";

    const std::vector<std::string> policies = {"RR", "FCFS", "SJN", "PRIORITY"};
    std::vector<PolicyMetrics> results;
    results.reserve(policies.size());

    for (const auto& policy : policies) {
        std::cout << "  → " << policy << "..." << std::flush;

        PolicyMetrics metrics = run_policy(policy, num_cores, workloads);
        if (metrics.success) {
            std::cout << " ok (CPU " << std::fixed << std::setprecision(1)
                      << metrics.cpu_util_pct << "%, hit "
                      << metrics.hit_rate_pct << "%)\n";
        } else {
            std::cout << " falhou: " << metrics.error << "\n";
        }

        results.push_back(std::move(metrics));
    }

    const std::string csv_path = build_csv_path(num_cores);
    const std::string report_path = build_report_path(num_cores);

    try {
        write_csv(results, csv_path);
        write_report(results, workloads, num_cores, report_path);
        std::cout << "\nArquivos gerados:\n";
        std::cout << "  • " << csv_path << "\n";
        std::cout << "  • " << report_path << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "❌ Falha ao escrever arquivos: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "\n✅ Teste encerrado. Ajuste DEFAULT_NUM_CORES para experimentar outras topologias.\n";
    return 0;
}
