# BATCH_CONFIG.md
# Configurações de Lote para Testes do Simulador Multicore

## Formato de Especificação de Lote

### Opção 1: Via Linha de Comando

```bash
./simulador [opções] -p programa1.json pcb1.json -p programa2.json pcb2.json ...
```

**Exemplo:**
```bash
./simulador -c 2 -q 100 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json
```

### Opção 2: Usando Processo Padrão

Se nenhum processo for especificado, o simulador carrega automaticamente:
- Programa: `tasks.json`
- PCB: `process1.json`

```bash
./simulador -c 2 -q 100
```

## Lotes Pré-Configurados

### Lote 1: Teste Básico (Processo Único)
```bash
./simulador -c 1 -p tasks.json process1.json
```
- **Objetivo:** Baseline single-core
- **Processos:** 1
- **Cores:** 1

### Lote 2: Carga Mista (3 Processos)
```bash
./simulador -c 2 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json
```
- **Objetivo:** Testar escalonamento com diferentes padrões de carga
- **Processos:** 3 (CPU-bound, I/O-bound, misto)
- **Cores:** 2

### Lote 3: Máximo Paralelismo
```bash
./simulador -c 4 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json
```
- **Objetivo:** Avaliar speedup com mais cores que processos
- **Processos:** 3
- **Cores:** 4

### Lote 4: Não-Preemptivo
```bash
./simulador -c 2 --non-preemptive \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json
```
- **Objetivo:** Teste do cenário não-preemptivo (Requisito 1)
- **Processos:** 2
- **Cores:** 2
- **Quantum:** Desabilitado (999999)

## Diretrizes de Carregamento

Conforme especificação do trabalho:

> "Todos os programas pertencentes ao lote devem ser **completamente carregados na memória principal antes do início da execução**."

O simulador implementa isso da seguinte forma:

1. **Parse de argumentos:** Identifica todos os pares (programa, PCB)
2. **Carregamento sequencial:** 
   - Para cada processo no lote:
     - Carrega PCB do arquivo JSON
     - Carrega programa na memória principal
     - Atribui endereço base (espaçamento de 1KB entre processos)
3. **Validação:** Se qualquer carregamento falhar, o simulador aborta antes da execução
4. **Execução:** Só inicia após todos os processos estarem na memória

### Visualização do Carregamento

Durante a execução, o simulador exibe:

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

Processo 2/3:
  ├─ PID:       101
  ├─ Nome:      io_bound
  ├─ Quantum:   100
  ├─ PCB:       test_programs/pcb_io_bound.json
  └─ Programa:  test_programs/io_bound.json
     └─> Carregando instruções na memória...
     └─> ✓ Carregado no endereço base 0x400

...

===========================================
✓ Todos os 3 processos foram carregados na memória
===========================================
```

## Criando Novos Processos

### 1. Criar arquivo de programa (ex: `my_program.json`)

```json
{
    "format": "tasks",
    "tasks": [
        {"instruction": "ADDI", "args": [1, 0, 10]},
        {"instruction": "ADDI", "args": [2, 0, 20]},
        {"instruction": "ADD", "args": [3, 1, 2]},
        {"instruction": "END"}
    ]
}
```

### 2. Criar arquivo PCB (ex: `my_pcb.json`)

```json
{
    "pid": 200,
    "name": "my_program",
    "quantum": 100,
    "priority": 1,
    "mem_weights": {
        "primary": 1,
        "secondary": 10
    }
}
```

### 3. Executar

```bash
./simulador -c 2 -p my_program.json my_pcb.json
```

## Verificação de Execução

Para verificar se todos os processos foram carregados e executados:

```bash
# Ver processos carregados
grep "Carregado no endereço" logs/test.log

# Ver processos finalizados
grep "FINALIZADO" logs/test.log

# Ver métricas
grep "METRICAS FINAIS" logs/test.log -A 10
```

## Troubleshooting

### Erro: "ERRO ao carregar PCB"
- Verifique se o arquivo existe
- Verifique se o JSON está válido
- Campos obrigatórios: `pid`, `name`, `quantum`, `priority`

### Erro: "ERRO ao carregar programa"
- Verifique se o arquivo existe
- Verifique se o formato está correto
- Use o formato `tasks` com array de instruções

### Processos não executam
- Verifique se há instrução `END` no programa
- Verifique se o endereço PC inicial está correto (normalmente 0)
- Veja logs detalhados no terminal
