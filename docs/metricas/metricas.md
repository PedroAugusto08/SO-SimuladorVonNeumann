# Sistema de Métricas

## Visão Geral

O simulador coleta métricas detalhadas de performance para análise e comparação entre políticas de escalonamento.

> **Última atualização:** 06/12/2025

## Medição de Tempo Atômico

### Namespace `cpu_time`

O projeto utiliza um namespace dedicado para garantir medições de tempo **thread-safe** e de **alta precisão**:

```cpp
// src/cpu/TimeUtils.hpp
namespace cpu_time {

// Retorna o timestamp atual em nanosegundos (desde epoch do steady_clock)
inline uint64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Converte nanosegundos para milissegundos
inline double ns_to_ms(uint64_t value_ns) {
    return static_cast<double>(value_ns) / 1'000'000.0;
}

// Converte nanosegundos para segundos
inline double ns_to_seconds(uint64_t value_ns) {
    return static_cast<double>(value_ns) / 1'000'000'000.0;
}

} // namespace cpu_time
```

### Por que `steady_clock`?

| Característica | `steady_clock` | `system_clock` |
|----------------|----------------|----------------|
| Monotônico | ✅ Sim | ❌ Não |
| Afetado por ajustes de sistema | ❌ Não | ✅ Sim |
| Precisão | Nanosegundos | Variável |
| Uso em métricas | **Recomendado** | Não recomendado |

O `std::chrono::steady_clock` é **monotônico**, ou seja, nunca retrocede. Isso é essencial para medições de performance, pois mudanças de horário do sistema (NTP sync, daylight saving, etc.) não afetam as medições.

### Contadores Atômicos no PCB

Os tempos são armazenados em contadores **atômicos** para garantir consistência em ambiente **multithread**:

```cpp
// src/cpu/PCB.hpp
struct PCB {
    // Timestamps atômicos (nanosegundos)
    std::atomic<uint64_t> arrival_time{0};    // Momento de chegada
    std::atomic<uint64_t> start_time{0};      // Primeira execução
    std::atomic<uint64_t> finish_time{0};     // Término
    std::atomic<uint64_t> total_wait_time{0}; // Tempo acumulado em espera
    
    // Controle de entrada/saída da fila ready
    std::atomic<uint64_t> ready_queue_enter_time{0};
    
    // Contadores de cache atômicos
    std::atomic<uint64_t> cache_hits{0};
    std::atomic<uint64_t> cache_misses{0};
};
```

### Cálculo do Tempo de Espera

O tempo de espera é calculado **incrementalmente** cada vez que o processo sai da fila de prontos:

```cpp
void PCB::enter_ready_queue() {
    // Marca o momento de entrada na fila
    ready_queue_enter_time.store(cpu_time::now_ns(), std::memory_order_relaxed);
}

void PCB::leave_ready_queue() {
    // Obtém timestamp de entrada (e reseta para 0)
    const uint64_t start = ready_queue_enter_time.exchange(0, std::memory_order_relaxed);
    if (start == 0) return;  // Não estava na fila
    
    const uint64_t end = cpu_time::now_ns();
    if (end > start) {
        // Adiciona atomicamente ao tempo total de espera
        total_wait_time.fetch_add(end - start, std::memory_order_relaxed);
    }
}
```

### Memory Ordering

O projeto usa `std::memory_order_relaxed` para contadores de métricas porque:

1. **Não há dependências de ordenação**: Cada contador é independente
2. **Performance**: Menor overhead que `memory_order_seq_cst`
3. **Atomicidade garantida**: A operação ainda é atômica, apenas a ordenação com outras operações é relaxada

```cpp
// Incremento atômico relaxado - alta performance
total_wait_time.fetch_add(delta, std::memory_order_relaxed);

// vs. sequencialmente consistente - mais lento
total_wait_time.fetch_add(delta, std::memory_order_seq_cst);
```

## Métricas Coletadas

### Por Processo

```cpp
struct ProcessMetrics {
    int pid;
    int priority;
    
    // Tempos (nanosegundos desde epoch do steady_clock)
    uint64_t start_time;      // Instante da primeira execução
    uint64_t end_time;        // Instante de término
    uint64_t wait_time;       // Tempo acumulado na fila ready
    
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
    uint64_t cycles_busy;   // ciclos de pipeline contabilizados
    uint64_t cycles_idle;   // ciclos equivalentes que o núcleo permaneceu ocioso
    
    double utilization(double elapsed_seconds, int total_cores) {
        const double busy_seconds = cycles_busy / CLOCK_FREQ_HZ;
        const double capacity_seconds = elapsed_seconds * total_cores;
        return (capacity_seconds > 0.0)
            ? (busy_seconds / capacity_seconds) * 100.0
            : 0.0;
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

$$Throughput = \frac{Processos\ Concluídos}{T_{fim\ geral} - T_{início\ geral}}$$

Taxa de conclusão de processos por segundo (tempo real medido com `steady_clock`).

### Utilização de CPU

$$U_{cpu} = \frac{Ciclos_{busy} / f_{clock}}{N_{cores} \times (T_{fim} - T_{início})} \times 100\%$$

Percentual do tempo disponível (considerando todos os núcleos) que foi efetivamente usado para executar instruções.

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
# Executar simulador e gerar métricas
./bin/simulador --policy FCFS --cores 2 -p tasks.json process.json

# Executar teste de métricas completo
make test-metrics
```

**Arquivos de saída:**
```
dados_graficos/
├── csv/
│   ├── metricas_1cores.csv
│   ├── metricas_2cores.csv
│   ├── metricas_4cores.csv
│   └── metricas_6cores.csv
└── reports/
    └── relatorio_metricas_Xcores.txt
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

O projeto inclui testes específicos para validar e coletar métricas:

### `test_metrics.cpp` (Principal)

Teste completo que avalia as políticas FCFS, SJN e Priority:

```bash
make test-metrics
```

**Saídas geradas:**
- `dados_graficos/csv/metricas_Xcores.csv` - Dados estruturados
- `dados_graficos/reports/relatorio_metricas_Xcores.txt` - Relatório legível

### `test_single_core_no_threads.cpp`

Teste determinístico single-core para validação de comportamento:

```bash
make test-single-core
```

**Uso:** Debugging e validação sem interferência de threads.

## Visualização com GUI

O projeto inclui uma interface gráfica para visualização de métricas:

```bash
python3 gui/monitor_v2.py
```

**Funcionalidades:**
- Seleção de políticas e métricas
- Gráficos de linha, barra e dispersão
- Exportação de gráficos (PNG) e dados (CSV)
- Comparação visual entre políticas

Para mais detalhes, consulte a [documentação da GUI](../uso/gui.md).
