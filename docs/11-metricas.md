# Métricas de Desempenho

## 🎯 Objetivo

Implementar sistema completo de coleta e análise de métricas para avaliar o desempenho do simulador multicore e gerar dados para o artigo IEEE.

---

## 📊 Métricas Essenciais

### 1. Métricas de CPU

```cpp
struct CPUMetrics {
    // Por Core
    int core_id;
    int processes_executed;
    long long cycles_executed;
    long long idle_cycles;
    double utilization; // %
    
    // Tempos
    double total_execution_time; // ms
    double busy_time; // ms
    double idle_time; // ms
    
    // Context Switches
    int context_switches;
    double avg_context_switch_time; // ms
    
    void calculate() {
        utilization = (double)cycles_executed / 
                     (cycles_executed + idle_cycles) * 100.0;
        busy_time = total_execution_time - idle_time;
    }
    
    void print() const {
        std::cout << "Core " << core_id << ":\n";
        std::cout << "  Utilization: " << utilization << "%\n";
        std::cout << "  Processes: " << processes_executed << "\n";
        std::cout << "  Context Switches: " << context_switches << "\n";
    }
};
```

---

### 2. Métricas de Processos

```cpp
struct ProcessMetrics {
    int pid;
    std::string name;
    
    // Tempos (em ms)
    double arrival_time;
    double start_time;
    double completion_time;
    double waiting_time;
    double turnaround_time;
    double response_time;
    
    // Execução
    int burst_time;
    int remaining_time;
    int quantum_count;
    int preemptions;
    
    // Core assignment
    int assigned_core;
    std::vector<int> cores_used; // Para migração
    
    void calculate() {
        turnaround_time = completion_time - arrival_time;
        waiting_time = turnaround_time - burst_time;
        response_time = start_time - arrival_time;
    }
    
    void print() const {
        std::cout << "Process " << pid << " (" << name << "):\n";
        std::cout << "  Turnaround Time: " << turnaround_time << " ms\n";
        std::cout << "  Waiting Time: " << waiting_time << " ms\n";
        std::cout << "  Response Time: " << response_time << " ms\n";
    }
};
```

---

### 3. Métricas de Memória

```cpp
struct MemoryMetrics {
    // Utilização
    int total_memory;
    int used_memory;
    int free_memory;
    double utilization; // %
    
    // Segmentação
    int total_segments;
    int segments_in_memory;
    int segments_on_disk;
    
    // Paginação
    int page_faults;
    int page_hits;
    double hit_rate; // %
    
    // Swapping
    int swap_ins;
    int swap_outs;
    double avg_swap_time; // ms
    
    // Fragmentação
    int internal_fragmentation;
    int external_fragmentation;
    
    void calculate() {
        utilization = (double)used_memory / total_memory * 100.0;
        hit_rate = (double)page_hits / (page_hits + page_faults) * 100.0;
    }
    
    void print() const {
        std::cout << "Memory:\n";
        std::cout << "  Utilization: " << utilization << "%\n";
        std::cout << "  Hit Rate: " << hit_rate << "%\n";
        std::cout << "  Page Faults: " << page_faults << "\n";
        std::cout << "  Swaps: " << (swap_ins + swap_outs) << "\n";
    }
};
```

---

### 4. Métricas do Sistema

```cpp
struct SystemMetrics {
    // Geral
    int num_cores;
    int total_processes;
    int completed_processes;
    
    // Throughput
    double throughput; // processos/segundo
    
    // Tempos médios
    double avg_turnaround_time;
    double avg_waiting_time;
    double avg_response_time;
    
    // Fairness
    double fairness_index; // 0-1 (Jain's Fairness Index)
    
    // Load Balancing
    std::vector<int> processes_per_core;
    double load_balance_coefficient; // Desvio padrão
    
    void calculate(const std::vector<ProcessMetrics>& processes) {
        // Throughput
        double total_time = 0;
        for (const auto& p : processes) {
            if (p.completion_time > total_time) {
                total_time = p.completion_time;
            }
        }
        throughput = completed_processes / (total_time / 1000.0);
        
        // Médias
        double sum_turnaround = 0, sum_waiting = 0, sum_response = 0;
        for (const auto& p : processes) {
            sum_turnaround += p.turnaround_time;
            sum_waiting += p.waiting_time;
            sum_response += p.response_time;
        }
        
        int n = processes.size();
        avg_turnaround_time = sum_turnaround / n;
        avg_waiting_time = sum_waiting / n;
        avg_response_time = sum_response / n;
        
        // Fairness Index (Jain's)
        calculateFairnessIndex(processes);
    }
    
private:
    void calculateFairnessIndex(const std::vector<ProcessMetrics>& processes) {
        double sum = 0, sum_sq = 0;
        int n = processes.size();
        
        for (const auto& p : processes) {
            sum += p.waiting_time;
            sum_sq += p.waiting_time * p.waiting_time;
        }
        
        fairness_index = (sum * sum) / (n * sum_sq);
    }
};
```

---

## 🔧 Coletor de Métricas

```cpp
class MetricsCollector {
private:
    // Métricas armazenadas
    std::vector<CPUMetrics> cpu_metrics;
    std::vector<ProcessMetrics> process_metrics;
    MemoryMetrics memory_metrics;
    SystemMetrics system_metrics;
    
    // Sincronização
    std::mutex metrics_mutex;
    
    // Timestamps
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    MetricsCollector(int num_cores) {
        cpu_metrics.resize(num_cores);
        for (int i = 0; i < num_cores; i++) {
            cpu_metrics[i].core_id = i;
        }
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    // Registrar eventos
    void recordProcessStart(int pid, int core_id) {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        
        auto& pm = getProcessMetrics(pid);
        pm.start_time = getElapsedTime();
        pm.assigned_core = core_id;
        
        cpu_metrics[core_id].processes_executed++;
    }
    
    void recordProcessCompletion(int pid) {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        
        auto& pm = getProcessMetrics(pid);
        pm.completion_time = getElapsedTime();
        pm.calculate();
        
        system_metrics.completed_processes++;
    }
    
    void recordContextSwitch(int core_id, double switch_time) {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        
        cpu_metrics[core_id].context_switches++;
        
        // Atualizar média
        auto& metrics = cpu_metrics[core_id];
        int n = metrics.context_switches;
        metrics.avg_context_switch_time = 
            (metrics.avg_context_switch_time * (n - 1) + switch_time) / n;
    }
    
    void recordCPUCycle(int core_id, bool idle) {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        
        if (idle) {
            cpu_metrics[core_id].idle_cycles++;
        } else {
            cpu_metrics[core_id].cycles_executed++;
        }
    }
    
    void recordPageFault() {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        memory_metrics.page_faults++;
    }
    
    void recordPageHit() {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        memory_metrics.page_hits++;
    }
    
    void recordSwap(bool swap_in) {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        
        if (swap_in) {
            memory_metrics.swap_ins++;
        } else {
            memory_metrics.swap_outs++;
        }
    }
    
    // Finalizar e calcular
    void finalize() {
        std::lock_guard<std::mutex> lock(metrics_mutex);
        
        // Calcular métricas de CPU
        for (auto& cpu : cpu_metrics) {
            cpu.calculate();
        }
        
        // Calcular métricas de memória
        memory_metrics.calculate();
        
        // Calcular métricas do sistema
        system_metrics.num_cores = cpu_metrics.size();
        system_metrics.total_processes = process_metrics.size();
        system_metrics.calculate(process_metrics);
    }
    
    // Exportar dados
    void exportToCSV(const std::string& filename) {
        std::ofstream file(filename);
        
        // Header
        file << "PID,Name,ArrivalTime,StartTime,CompletionTime,";
        file << "WaitingTime,TurnaroundTime,ResponseTime,";
        file << "BurstTime,AssignedCore\n";
        
        // Dados
        for (const auto& pm : process_metrics) {
            file << pm.pid << ","
                 << pm.name << ","
                 << pm.arrival_time << ","
                 << pm.start_time << ","
                 << pm.completion_time << ","
                 << pm.waiting_time << ","
                 << pm.turnaround_time << ","
                 << pm.response_time << ","
                 << pm.burst_time << ","
                 << pm.assigned_core << "\n";
        }
        
        file.close();
    }
    
    void exportSystemMetrics(const std::string& filename) {
        std::ofstream file(filename);
        
        file << "Metric,Value\n";
        file << "NumCores," << system_metrics.num_cores << "\n";
        file << "TotalProcesses," << system_metrics.total_processes << "\n";
        file << "CompletedProcesses," << system_metrics.completed_processes << "\n";
        file << "Throughput," << system_metrics.throughput << "\n";
        file << "AvgTurnaroundTime," << system_metrics.avg_turnaround_time << "\n";
        file << "AvgWaitingTime," << system_metrics.avg_waiting_time << "\n";
        file << "AvgResponseTime," << system_metrics.avg_response_time << "\n";
        file << "FairnessIndex," << system_metrics.fairness_index << "\n";
        
        file.close();
    }
    
    // Imprimir relatório
    void printReport() {
        std::cout << "\n========================================\n";
        std::cout << "        PERFORMANCE REPORT\n";
        std::cout << "========================================\n\n";
        
        // Sistema
        std::cout << "SYSTEM METRICS:\n";
        std::cout << "  Cores: " << system_metrics.num_cores << "\n";
        std::cout << "  Total Processes: " << system_metrics.total_processes << "\n";
        std::cout << "  Completed: " << system_metrics.completed_processes << "\n";
        std::cout << "  Throughput: " << system_metrics.throughput << " proc/s\n";
        std::cout << "  Avg Turnaround Time: " << system_metrics.avg_turnaround_time << " ms\n";
        std::cout << "  Avg Waiting Time: " << system_metrics.avg_waiting_time << " ms\n";
        std::cout << "  Fairness Index: " << system_metrics.fairness_index << "\n\n";
        
        // CPUs
        std::cout << "CPU METRICS:\n";
        for (const auto& cpu : cpu_metrics) {
            cpu.print();
            std::cout << "\n";
        }
        
        // Memória
        std::cout << "MEMORY METRICS:\n";
        memory_metrics.print();
        
        std::cout << "\n========================================\n";
    }
    
private:
    ProcessMetrics& getProcessMetrics(int pid) {
        for (auto& pm : process_metrics) {
            if (pm.pid == pid) {
                return pm;
            }
        }
        
        // Criar novo
        ProcessMetrics pm;
        pm.pid = pid;
        process_metrics.push_back(pm);
        return process_metrics.back();
    }
    
    double getElapsedTime() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time);
        return duration.count();
    }
};
```

---

## 📈 Análise Comparativa

### Comparar Single-Core vs Multicore

```cpp
class ComparativeAnalysis {
public:
    struct ComparisonResult {
        double speedup;
        double efficiency;
        double throughput_improvement;
        double avg_turnaround_improvement;
    };
    
    static ComparisonResult compare(
        const SystemMetrics& single_core,
        const SystemMetrics& multi_core) {
        
        ComparisonResult result;
        
        // Speedup = Tempo(1 core) / Tempo(N cores)
        result.speedup = single_core.avg_turnaround_time / 
                        multi_core.avg_turnaround_time;
        
        // Eficiência = Speedup / N
        result.efficiency = result.speedup / multi_core.num_cores;
        
        // Melhoria de throughput
        result.throughput_improvement = 
            (multi_core.throughput - single_core.throughput) / 
            single_core.throughput * 100.0;
        
        // Melhoria de turnaround
        result.avg_turnaround_improvement =
            (single_core.avg_turnaround_time - multi_core.avg_turnaround_time) /
            single_core.avg_turnaround_time * 100.0;
        
        return result;
    }
    
    static void printComparison(const ComparisonResult& result) {
        std::cout << "\n=== COMPARATIVE ANALYSIS ===\n";
        std::cout << "Speedup: " << result.speedup << "x\n";
        std::cout << "Efficiency: " << (result.efficiency * 100) << "%\n";
        std::cout << "Throughput Improvement: " << result.throughput_improvement << "%\n";
        std::cout << "Turnaround Improvement: " << result.avg_turnaround_improvement << "%\n";
    }
};
```

---

## 📊 Geração de Gráficos (Python)

```python
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def generate_performance_graphs(csv_file):
    # Ler dados
    df = pd.read_csv(csv_file)
    
    # Gráfico 1: Turnaround Time por Processo
    plt.figure(figsize=(12, 6))
    plt.bar(df['PID'], df['TurnaroundTime'])
    plt.xlabel('Process ID')
    plt.ylabel('Turnaround Time (ms)')
    plt.title('Turnaround Time per Process')
    plt.savefig('turnaround_time.png')
    plt.close()
    
    # Gráfico 2: Waiting Time vs Burst Time
    plt.figure(figsize=(10, 6))
    plt.scatter(df['BurstTime'], df['WaitingTime'])
    plt.xlabel('Burst Time (ms)')
    plt.ylabel('Waiting Time (ms)')
    plt.title('Waiting Time vs Burst Time')
    plt.savefig('waiting_vs_burst.png')
    plt.close()
    
    # Gráfico 3: Distribuição por Core
    plt.figure(figsize=(10, 6))
    core_counts = df['AssignedCore'].value_counts().sort_index()
    plt.bar(core_counts.index, core_counts.values)
    plt.xlabel('Core ID')
    plt.ylabel('Number of Processes')
    plt.title('Process Distribution Across Cores')
    plt.savefig('core_distribution.png')
    plt.close()

# Uso
generate_performance_graphs('metrics.csv')
```

---

## 🔗 Próximos Passos

- ➡️ [Estratégia de Testes](12-testes.md)
- ➡️ [Estrutura do Artigo IEEE](15-estrutura-artigo.md)

---

## 📚 Referências

- JAIN, R. The Art of Computer Systems Performance Analysis
- LILJA, D. J. Measuring Computer Performance
