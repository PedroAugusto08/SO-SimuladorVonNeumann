# Comandos de Uso

## Sintaxe Básica

```bash
./simulador [opções] -p <programa.json> <processo.json>
```

## Opções Disponíveis

| Opção | Descrição | Valores | Padrão |
|-------|-----------|---------|--------|
| `--cores N` | Número de núcleos | 1-8 | 2 |
| `--policy POLICY` | Política de escalonamento | FCFS, SJN, RR, PRIORITY, PRIORITY_PREEMPT | FCFS |
| `--quantum N` | Quantum para Round Robin | 100-10000 | 1000 |
| `--cache-policy` | Política de cache | FIFO, LRU | LRU |
| `-p <prog> <proc>` | Par programa/processo | Arquivos JSON | - |
| `--help` | Mostra ajuda | - | - |

## Exemplos de Uso

### Execução Simples

```bash
# FCFS com 2 núcleos
./simulador --policy FCFS --cores 2 \
    -p examples/programs/tasks.json examples/processes/process1.json
```

### Round Robin

```bash
# Round Robin com quantum de 500 ciclos
./simulador --policy RR --cores 4 --quantum 500 \
    -p examples/programs/tasks.json examples/processes/process1.json
```

### Múltiplos Processos

```bash
# Executar 3 processos diferentes
./simulador --policy FCFS --cores 4 \
    -p examples/programs/tasks.json examples/processes/process_high.json \
    -p examples/programs/tasks.json examples/processes/process_medium.json \
    -p examples/programs/tasks.json examples/processes/process_low.json
```

### Comparação de Políticas

```bash
# FCFS
./simulador --policy FCFS --cores 2 -p tasks.json proc.json > fcfs.log

# SJN
./simulador --policy SJN --cores 2 -p tasks.json proc.json > sjn.log

# Round Robin
./simulador --policy RR --cores 2 --quantum 1000 -p tasks.json proc.json > rr.log

# Comparar resultados
diff fcfs.log sjn.log
```

## Formato dos Arquivos

### Arquivo de Programa (tasks.json)

```json
{
    "program": [
        {"opcode": "LOAD", "op1": "R0", "op2": "0"},
        {"opcode": "ADD", "op1": "R0", "op2": "R1"},
        {"opcode": "STORE", "op1": "R0", "op2": "100"},
        {"opcode": "HALT"}
    ]
}
```

### Arquivo de Processo (process.json)

```json
{
    "pid": 1,
    "priority": 5,
    "program_file": "tasks.json"
}
```

## Saída do Simulador

### Console

```
=== Simulador Von Neumann Multicore ===
Configuração:
  Núcleos: 4
  Política: FCFS
  Quantum: N/A

Iniciando execução...

[Ciclo 0] Core 0: Iniciando processo PID=1
[Ciclo 0] Core 1: Iniciando processo PID=2
[Ciclo 100] Core 0: LOAD R0, [0] -> R0 = 42
...

=== Resultados ===
Processos concluídos: 3
Ciclos totais: 5000
Throughput médio: 0.85 inst/ciclo
```

### Arquivo de Métricas

Gerado em `logs/metrics/detailed_metrics.csv`:

```csv
PID,Priority,StartTime,EndTime,WaitTime,Turnaround,Instructions,ContextSwitches
1,5,0,1500,200,1500,1300,0
2,7,0,1200,100,1200,1100,0
3,3,100,2000,500,1900,1400,0
```

## Make Targets

```bash
# Compilar e executar com configuração padrão
make run

# Executar testes específicos
make test-metrics      # gera CSV/TXT em dados_graficos
make test-single-core  # validações determinísticas em test/output

# Limpar e recompilar
make clean && make

# Ver todos os targets
make help
```

## Scripts Auxiliares

### run_comparison.sh

```bash
#!/bin/bash
# Compara todas as políticas

POLICIES="FCFS SJN RR PRIORITY"
CORES=4

for policy in $POLICIES; do
    echo "=== $policy ==="
    ./simulador --policy $policy --cores $CORES \
        -p tasks.json process.json 2>&1 | tee ${policy,,}.log
    echo
done
```

### benchmark.sh

```bash
#!/bin/bash
# Benchmark com diferentes configurações

for cores in 1 2 4 8; do
    for quantum in 500 1000 2000; do
        echo "Cores=$cores Quantum=$quantum"
        time ./simulador --policy RR --cores $cores --quantum $quantum \
            -p tasks.json process.json > /dev/null
    done
done
```

## Dicas de Uso

### Performance

- Use `--cores` igual ao número de processos para paralelismo máximo
- Para Round Robin, quantum ~1000 é um bom equilíbrio
- SJN geralmente dá menor tempo de espera médio

### Debug

```bash
# Executar com saída verbose
./simulador --policy FCFS --cores 1 -p tasks.json process.json 2>&1 | less

# Salvar logs para análise
./simulador --policy FCFS --cores 2 -p tasks.json process.json > execution.log 2>&1
```

### Análise

```bash
# Extrair métricas específicas
grep "Throughput" execution.log
grep "Wait time" execution.log

# Contar context switches
grep -c "Context switch" execution.log
```
