# Test Programs - Simulador Multicore

Este diretório contém programas de teste para validar o simulador multicore.

## Processos Disponíveis

### 1. CPU Intensive (`cpu_intensive.json`)
- **Tipo:** CPU-bound
- **Descrição:** Loop de 1000 iterações com operações aritméticas (ADD, SUB, MULT, DIV)
- **PCB:** `pcb_cpu_intensive.json` (PID 100, quantum 100)

### 2. I/O Bound (`io_bound.json`)
- **Tipo:** I/O-bound
- **Descrição:** 5 operações sequenciais de PRINT (causa bloqueios)
- **PCB:** `pcb_io_bound.json` (PID 101, quantum 100)

### 3. Mixed Workload (`mixed_workload.json`)
- **Tipo:** Misto (CPU + I/O)
- **Descrição:** Loop de 50 iterações com computação e prints periódicos
- **PCB:** `pcb_mixed.json` (PID 102, quantum 100)

### 4. Short CPU (`short_cpu.json`)
- **Tipo:** CPU-bound curto
- **Descrição:** Loop de 100 iterações (processo rápido)
- **PCB:** `pcb_short_cpu.json` (PID 103, quantum 50)

### 5. I/O Light (`io_light.json`)
- **Tipo:** I/O leve
- **Descrição:** 10 iterações com print a cada ciclo
- **PCB:** `pcb_io_light.json` (PID 104, quantum 75)

## Carregamento de Múltiplos Processos

**IMPORTANTE:** Conforme especificação, todos os processos do lote são carregados na memória principal ANTES do início da execução.

### Como usar

### Processo Único (Baseline)
```bash
./simulador -c 1 -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json
```

### Múltiplos Processos (3 processos)
```bash
./simulador -c 2 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json
```

### Lote com 5 Processos
```bash
./simulador -c 2 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    -p test_programs/short_cpu.json test_programs/pcb_short_cpu.json \
    -p test_programs/io_light.json test_programs/pcb_io_light.json
```

### Modo Não-Preemptivo
```bash
./simulador -c 2 --non-preemptive \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json
```

## Scripts de Teste Automatizados

### `test_multiprocess.sh` - Suite Completa
Executa 5 cenários:
- Processo único (baseline)
- 3 processos, 1/2/4 cores
- Modo não-preemptivo

```bash
./test_multiprocess.sh
```

### `test_5processes.sh` - Lote com 5 Processos
```bash
./test_5processes.sh
```

### `test_non_preemptive.sh` - Cenário Não-Preemptivo
```bash
./test_non_preemptive.sh
```

## Verificação dos Resultados

```bash
# Ver processos carregados
grep "Carregado no endereço" logs/*.log

# Ver processos finalizados
grep "FINALIZADO" logs/*.log

# Ver métricas
grep "METRICAS FINAIS" logs/*.log -A 10

# Contar processos executados
grep -c "FINALIZADO" logs/test_3processes_2cores.log
```

## Argumentos de Linha de Comando

```
--cores, -c N              Número de núcleos (padrão: 2)
--quantum, -q N            Quantum em ciclos (padrão: 100)
--non-preemptive, -np      Modo não-preemptivo
--process, -p PROG PCB     Adiciona processo ao lote (pode repetir)
--help, -h                 Exibe ajuda
```

## Comportamento Esperado

### Modo Preemptivo
- Processos são interrompidos após consumir quantum
- Voltam para fila de prontos
- Troca de contexto frequente

### Modo Não-Preemptivo
- Processos executam até completar ou bloquear por I/O
- Sem interrupções por quantum (quantum = 999999)
- Ordem FIFO respeitada

### Carregamento
```
===========================================
  CARREGAMENTO DE PROCESSOS
===========================================
Total de processos a carregar: 3

Processo 1/3:
  ├─ PID:       100
  ├─ Nome:      cpu_intensive
  ├─ Quantum:   100
  ├─ PCB:       test_programs/pcb_cpu_intensive.json
  └─ Programa:  test_programs/cpu_intensive.json
     └─> Carregando instruções na memória...
     └─> ✓ Carregado no endereço base 0x0
...
===========================================
✓ Todos os 3 processos foram carregados na memória
===========================================
```
