# Testes do Cenário Não-Preemptivo

## Como usar

### 1. Testar cenário não-preemptivo:
```bash
./test_non_preemptive.sh
```

### 2. Testar manualmente com diferentes configurações:

**Baseline (1 núcleo, não-preemptivo):**
```bash
./simulador --cores 1 --non-preemptive
```

**2 núcleos, não-preemptivo:**
```bash
./simulador --cores 2 --non-preemptive
```

**4 núcleos, não-preemptivo:**
```bash
./simulador --cores 4 --non-preemptive
```

**Modo preemptivo (para comparação):**
```bash
./simulador --cores 2 --quantum 100
```

### 3. Verificar resultados:

Os logs serão salvos em `logs/` com informações sobre:
- Modo de execução (PREEMPTIVO vs NÃO-PREEMPTIVO)
- Atribuição de processos aos núcleos
- Ausência de preempção por quantum no modo não-preemptivo

## Processos de Teste

Três tipos de processos estão disponíveis em `test_programs/`:

1. **cpu_intensive.json** - Operações aritméticas intensivas (1000 iterações)
2. **io_bound.json** - Múltiplas operações de I/O (prints)
3. **mixed_workload.json** - Mix de CPU e I/O

## O que esperar no modo não-preemptivo:

- ✓ Processos executam até completar ou bloquear por I/O
- ✓ Sem interrupções por quantum
- ✓ Ordem FIFO é mantida
- ✓ Console mostra "modo=NÃO-PREEMPTIVO"
- ✓ Quantum mostrado como muito alto (999999) ou não exibido

## Argumentos disponíveis:

```
--cores, -c N         Número de núcleos (padrão: 2)
--quantum, -q N       Quantum em ciclos (padrão: 100)
--non-preemptive, -np Modo não-preemptivo (sem quantum)
--help, -h            Exibe ajuda
```
