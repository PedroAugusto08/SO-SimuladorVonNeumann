# Sistema de Métricas

## Visão Geral

O simulador coleta métricas detalhadas de performance para análise e comparação entre políticas de escalonamento.

## Métricas Coletadas

### Por Processo

```cpp
struct ProcessMetrics {
    int pid;
    int priority;
    
    // Tempos
    uint64_t start_time;      // Ciclo de início
    uint64_t end_time;        // Ciclo de término
    uint64_t wait_time;       // Ciclos esperando na fila
    
    // Instruções
    uint64_t instructions_executed;
    
    // Context switches
    int context_switches;
    
    // Cálculos derivados
    uint64_t turnaround_time() {
        return end_time - start_time;
    }
    
    uint64_t execution_time() {
        return turnaround_time() - wait_time;
    }
    
    double throughput() {
        return (double)instructions_executed / execution_time();
    }
};
```

### Por Núcleo

```cpp
struct CoreMetrics {
    int core_id;
    uint64_t instructions_executed;
    uint64_t cycles_busy;
    uint64_t cycles_idle;
    
    double utilization() {
        return (double)cycles_busy / (cycles_busy + cycles_idle) * 100;
    }
};
```

### Globais

```cpp
struct SystemMetrics {
    uint64_t total_cycles;
    uint64_t total_instructions;
    int processes_completed;
    
    // Médias
    double avg_wait_time;
    double avg_turnaround_time;
    double avg_throughput;
    
    // Cache
    uint64_t cache_hits;
    uint64_t cache_misses;
    double cache_hit_rate;
};
```

## Fórmulas

### Turnaround Time

$$T_{turnaround} = T_{fim} - T_{chegada}$$

Tempo total desde a chegada do processo até sua conclusão.

### Waiting Time

$$T_{wait} = T_{turnaround} - T_{execução}$$

Tempo que o processo ficou na fila de prontos.

### Throughput

$$Throughput = \frac{Instruções}{Ciclos}$$

Taxa de instruções por ciclo.

### Utilização de CPU

$$U_{cpu} = \frac{Ciclos_{busy}}{Ciclos_{total}} \times 100\%$$

Percentual de tempo que o núcleo estava executando.

### Cache Hit Rate

$$Hit\ Rate = \frac{Hits}{Hits + Misses} \times 100\%$$

## Coleta de Métricas

### Durante Execução

```cpp
void Core::execute_instruction() {
    cycles++;
    
    if (current_process != nullptr) {
        cycles_busy++;
        current_process->instructions_executed++;
    } else {
        cycles_idle++;
    }
}

void Scheduler::on_process_finish(PCB* process) {
    process->end_time = global_cycle;
    process->wait_time = calculate_wait_time(process);
    
    metrics_collector.record(process);
}
```

### Exportação

```cpp
void MetricsCollector::export_csv(const std::string& filename) {
    std::ofstream file(filename);
    
    // Header
    file << "PID,Priority,StartTime,EndTime,WaitTime,";
    file << "Turnaround,Instructions,ContextSwitches\n";
    
    // Dados
    for (auto& m : process_metrics) {
        file << m.pid << "," << m.priority << ",";
        file << m.start_time << "," << m.end_time << ",";
        file << m.wait_time << "," << m.turnaround_time() << ",";
        file << m.instructions_executed << ",";
        file << m.context_switches << "\n";
    }
}
```

## Arquivo de Saída

O sistema gera `logs/metrics/detailed_metrics.csv`:

```csv
PID,Priority,StartTime,EndTime,WaitTime,Turnaround,Instructions,ContextSwitches
1,5,0,1500,200,1500,1300,3
2,7,10,1200,100,1190,1090,2
3,3,20,2000,500,1980,1480,4
```

## Interpretação de Resultados

### Métricas Boas vs Ruins

| Métrica | Bom | Ruim | Indica |
|---------|-----|------|--------|
| Wait Time | Baixo | Alto | Eficiência do escalonador |
| Turnaround | Baixo | Alto | Tempo de resposta |
| Throughput | Alto | Baixo | Performance geral |
| CPU Util | >80% | <50% | Uso eficiente |
| Cache Hit | >80% | <50% | Localidade dos acessos |

### Comparação entre Políticas

**Exemplo de resultado:**

| Política | Avg Wait | Avg Turnaround | Throughput |
|----------|----------|----------------|------------|
| FCFS | 500 | 1200 | 0.8 |
| SJN | **350** | **1000** | 0.85 |
| Round Robin | 450 | 1150 | 0.75 |
| Priority | 400 | 1100 | 0.82 |

**Interpretação:**
- SJN tem menor tempo de espera médio
- Round Robin tem menor throughput (overhead de context switch)
- Priority balanceia bem os tempos

## Uso

### Via CLI

```bash
# Executar e gerar métricas
./simulador --policy FCFS --cores 2 -p tasks.json process.json

# Métricas serão salvas em:
# logs/metrics/detailed_metrics.csv
```

### Análise Posterior

```python
import pandas as pd
import matplotlib.pyplot as plt

# Carregar métricas
df = pd.read_csv('logs/metrics/detailed_metrics.csv')

# Análise básica
print(df.describe())

# Gráfico de turnaround por política
df.groupby('Policy')['Turnaround'].mean().plot(kind='bar')
plt.title('Turnaround Médio por Política')
plt.show()
```

## Testes de Métricas

O projeto inclui testes específicos para validar métricas:

- `test/test_cpu_metrics.cpp` - Validação de métricas de CPU
- `test/test_metrics_complete.cpp` - Teste completo de coleta
- `test/test_multicore_comparative.cpp` - Comparação entre políticas

```bash
# Executar testes de métricas
make test_metrics
```
