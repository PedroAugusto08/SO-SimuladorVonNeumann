# 🚀 Guia de Execução - Simulador Von Neumann Multicore

> **Guia completo passo a passo para explorar todas as funcionalidades do simulador**

---

## 📋 Índice

1. [Primeiros Passos](#-primeiros-passos)
2. [Executando o Simulador Principal](#-executando-o-simulador-principal)
3. [Testando Políticas de Escalonamento](#-testando-políticas-de-escalonamento)
4. [Testando Escalabilidade Multicore](#-testando-escalabilidade-multicore)
5. [Executando Testes Automatizados](#-executando-testes-automatizados)
6. [Analisando Resultados](#-analisando-resultados)
7. [Cenários Avançados](#-cenários-avançados)
8. [Troubleshooting](#-troubleshooting)

---

## 🎯 Primeiros Passos

### 1. Compilar o Projeto

```bash
# Limpar compilações anteriores (opcional)
make clean

# Compilar o simulador principal
make simulador

# Verificar se compilou corretamente
ls -l bin/simulador
```

**Saída esperada:**
```
✓ Simulador multicore compilado com sucesso!
-rwxr-xr-x 1 user user 2847352 Dec  1 10:30 bin/simulador
```

### 2. Verificar Ajuda

```bash
./bin/simulador --help
```

**Você verá:**
- Lista completa de opções CLI
- Descrição das 4 políticas de escalonamento
- Exemplos de uso
- Arquivos de saída gerados

---

## 💻 Executando o Simulador Principal

### Execução Básica (Padrão)

```bash
./bin/simulador
```

**Configuração padrão:**
- 2 núcleos
- Política: Round Robin
- Quantum: 100 ciclos
- Programa: `examples/programs/tasks.json`
- Processo: `examples/processes/process1.json`

**Saída:**
```
===========================================
  SIMULADOR MULTICORE
===========================================
Configuração:
  - Núcleos: 2
  - Política: Round Robin
  - Quantum: 100 ciclos
===========================================

[Scheduler] Inicializando com 2 núcleos e quantum=100
[Core 0] Inicializado com cache L1 privada
[Core 1] Inicializado com cache L1 privada
✓ 1 processo(s) carregado(s)

===========================================
Iniciando escalonador...
===========================================
...
```

### Personalizar Número de Núcleos

```bash
# 1 núcleo (sequencial)
./bin/simulador --cores 1

# 4 núcleos
./bin/simulador --cores 4

# 8 núcleos (máximo)
./bin/simulador --cores 8
```

### Personalizar Quantum

```bash
# Quantum pequeno (mais preempção)
./bin/simulador --quantum 50

# Quantum grande (menos preempção)
./bin/simulador --quantum 200
```

---

## 🎛️ Testando Políticas de Escalonamento

### 1. Round Robin (RR)

**Características:**
- Preemptivo com quantum
- Distribuição justa de CPU
- Ideal para sistemas interativos

```bash
./bin/simulador --policy RR --cores 4 --quantum 100
```

**O que observar:**
- Processos alternam após quantum
- Context switches frequentes
- Tempo de resposta baixo

---

### 2. First Come First Served (FCFS)

**Características:**
- Não preemptivo
- Ordem de chegada
- Simples mas pode causar convoy effect

```bash
./bin/simulador --policy FCFS --cores 4
```

**O que observar:**
- Processos executam até completar
- Sem preempção (quantum ignorado)
- Pode ter wait time alto para processos que chegam depois

---

### 3. Shortest Job Next (SJN)

**Características:**
- Não preemptivo
- Menor job primeiro
- Minimiza tempo médio de espera

```bash
./bin/simulador --policy SJN --cores 4
```

**O que observar:**
- Processos com menor `estimated_job_size` executam primeiro
- Pode causar starvation de jobs grandes
- Melhor média de turnaround time

---

### 4. Priority Scheduling

**Características:**
- Preemptivo por prioridade
- Processos críticos primeiro
- Usa quantum como fallback

```bash
./bin/simulador --policy PRIORITY --cores 4 --quantum 100
```

**O que observar:**
- Processos com maior `priority` executam primeiro
- Preempção quando chega processo mais prioritário
- Pode causar starvation de baixa prioridade

---

## 🔬 Testando e Coletando Métricas (Atualizado em 06/12/2025)

> **Importante:** Os alvos `test-multicore`, `test-throughput`, `test-all`, `test_metrics_complete`, `test-preemption` e derivados foram aposentados durante a limpeza de dezembro/2025. Utilize os comandos abaixo para validar o estado atual do simulador.

### Teste de Métricas Multicore Consolidado

```bash
make test-metrics
```

**O que faz:**
- Executa FCFS, SJN e Priority com a configuração multicore padrão (4 núcleos)
- Drena os schedulers com as mesmas rotinas do binário principal
- Gera relatórios em `dados_graficos/csv/metricas_4cores.csv` e `dados_graficos/reports/relatorio_metricas_4cores.txt`

**Use este teste quando:**
- Precisar comparar o comportamento das políticas sem comandos adicionais
- Desejar material direto para os gráficos do artigo
- Quiser confirmar se a coleta de métricas não regressou após mudanças no core

### Teste Single-Core Determinístico (Sem Threads)

```bash
make test-single-core
```

**O que faz:**
- Compila e executa `test/test_single_core_no_threads.cpp`
- Roda todo o pipeline em um único core, eliminando concorrência
- Salva artefatos em `test/output/`

**Use este teste quando:**
- Precisar depurar instruções/pipeline sem interferência de múltiplos núcleos
- Verificar regressões causadas pelos novos escalonadores
- Demonstrar execução determinística para o relatório

### Testes Estruturais de Registradores

```bash
# Hash map dos registradores MIPS
make test-hash

# Banco completo de registradores
make test-bank
```

Ambos continuam relevantes para validar integridade do mapeamento MIPS, mesmo após a remoção dos demais testes automatizados.

---

## 🧪 Executando os Alvos Disponíveis

- `make test-metrics`: métrica multicore end-to-end.
- `make test-single-core`: execução determinística sem threads.
- `make test-hash` / `make test-bank`: testes unitários da camada de registradores.

Combine esses alvos com `make simulador` + `make run-sim` para validar a aplicação completa.

## 📊 Analisando Resultados

### Arquivos de Saída

#### 1. Resultados do Processo

```bash
cat output/resultados.dat
```

**Conteúdo:**
```
=== Resultados de Execução ===
PID: 1
Nome: processo_teste_1
Quantum: 100
Prioridade: 1
Ciclos de Pipeline: 3450
Ciclos de Memória: 892
Cache Hits: 1234
Cache Misses: 456
Ciclos de IO: 0
```

---

#### 2. Saída Lógica do Programa

```bash
cat output/output.dat
```

**Conteúdo:**
```
=== Saída Lógica do Programa ===
Registradores principais:
  $zero = 0
  $t0 = 100
  $t1 = 5
  $t2 = 105
  ...

=== Operações Executadas ===
LI t0 = 100
ADDI t1 = 5
ADD t2 = t0 + t1 = 105
...
```

---

#### 3. Logs de Multicore

```bash
cat logs/multicore/multicore_results.csv
```

**Formato CSV:**
```csv
cores,time_ms,speedup,efficiency,processes
1,2500,1.00,100.0,4
2,1282,1.95,97.5,4
4,661,3.78,94.5,4
8,389,6.42,80.2,4
```

**Importar no Excel/LibreOffice:**
- Criar gráfico de speedup vs núcleos
- Plotar eficiência vs núcleos

---

#### 4. Utilização de Memória

```bash
cat logs/memory/memory_utilization.csv
```

**Formato CSV:**
```csv
timestamp,main_memory_used,secondary_memory_used,cache_hits,cache_misses
0,256,0,0,0
100,512,0,45,12
200,768,0,123,34
...
```

**Análise:**
- Plotar uso de memória ao longo do tempo
- Calcular taxa de hit da cache
- Identificar picos de uso

---

### Visualização com Python

```python
import pandas as pd
import matplotlib.pyplot as plt

# Carregar dados
df = pd.read_csv('logs/multicore/multicore_results.csv')

# Plotar speedup
plt.figure(figsize=(10, 6))
plt.plot(df['cores'], df['speedup'], marker='o')
plt.plot(df['cores'], df['cores'], linestyle='--', label='Linear Ideal')
plt.xlabel('Número de Núcleos')
plt.ylabel('Speedup')
plt.title('Escalabilidade Multicore')
plt.legend()
plt.grid(True)
plt.savefig('speedup_chart.png')
```

---

## 🎓 Cenários Avançados

### Cenário 1: Comparação Detalhada de Políticas

```bash
#!/bin/bash
# Script: compare_policies.sh

echo "=== Comparando Políticas de Escalonamento ==="

for POLICY in RR FCFS SJN PRIORITY; do
    echo ""
    echo "Testando política: $POLICY"
    
    ./bin/simulador --cores 4 --policy $POLICY \
        --process examples/programs/tasks.json examples/processes/process_high.json \
        --process examples/programs/tasks.json examples/processes/process_medium.json \
        --process examples/programs/tasks.json examples/processes/process_low.json
    
    cp output/resultados.dat "results_${POLICY}.dat"
done

echo ""
echo "✅ Resultados salvos em results_*.dat"
```

**Executar:**
```bash
chmod +x compare_policies.sh
./compare_policies.sh
```

---

### Cenário 2: Teste de Carga Pesada

```bash
# 8 processos simultâneos em 8 núcleos
./bin/simulador --cores 8 --policy RR --quantum 50 \
    -p examples/programs/tasks.json examples/processes/process1.json \
    -p examples/programs/tasks.json examples/processes/process_high.json \
    -p examples/programs/tasks.json examples/processes/process_medium.json \
    -p examples/programs/tasks.json examples/processes/process_low.json \
    -p examples/programs/tasks_simple.json examples/processes/process1.json \
    -p examples/programs/tasks_simple.json examples/processes/process_high.json \
    -p examples/programs/tasks_simple.json examples/processes/process_medium.json \
    -p examples/programs/tasks_simple.json examples/processes/process_low.json
```

**O que observar:**
- Todos os cores ocupados simultaneamente
- Cache hits/misses em cada core
- Distribuição de carga entre cores

---

### Cenário 3: Análise de Cache Performance

```bash
# Teste com programa grande (muitos acessos à memória)
./bin/simulador --cores 1 --policy RR \
    -p examples/programs/tasks.json examples/processes/process1.json

# Verificar taxa de hit
grep "Cache" output/resultados.dat
```

**Calcular hit rate:**
```
Hit Rate = Cache Hits / (Cache Hits + Cache Misses)
```

**Exemplo:**
```
Cache Hits: 1234
Cache Misses: 456
Hit Rate = 1234 / (1234 + 456) = 73.0%
```

---

### Cenário 4: Debugging de Race Conditions

O alvo `make test-race-debug` foi removido. Para investigar condições de corrida hoje:

```bash
# 1) Reproduzir sem concorrência para validar lógica
make test-single-core

# 2) Reproduzir em modo multicore com coleta detalhada
make test-metrics

# 3) Opcional: instrumentar com TSAN
make CXXFLAGS="-Wall -Wextra -g -std=c++17 -Isrc -fsanitize=thread" simulador
./bin/simulador
```

- Compare os relatórios de `test/output/` (single-core) com `dados_graficos/csv/metricas_4cores.csv` para localizar divergências.
- Utilize `gdb` ou `tsan` quando suspeitar de condições de corrida após as mudanças no scheduler.

---

## 🔧 Troubleshooting

### Problema 1: Erro ao Executar

**Erro:**
```
Erro ao carregar 'examples/programs/tasks.json'
```

**Solução:**
```bash
# Verificar se arquivos existem
ls examples/programs/tasks.json
ls examples/processes/process1.json

# Se não existirem, verificar estrutura
ls -R examples/
```

---

### Problema 2: Compilação Falha

**Erro:**
```
undefined reference to pthread_create
```

**Solução:**
```bash
# Verificar flags do compilador
grep LDFLAGS Makefile
# Deve conter: -lpthread

# Recompilar
make clean
make simulador
```

---

### Problema 3: Testes Não Executam

**Erro:**
```
make: *** No rule to make target 'test-multicore'
```

**Solução:**
- Esse alvo foi removido em 06/12/2025. Utilize `make test-metrics` para obter as métricas multicore oficiais ou `make test-single-core` para execuções determinísticas.
- Rode `make help` para ver todos os comandos que ainda existem.

---

### Problema 4: Resultados Inconsistentes

**Sintoma:** Métricas variam muito entre execuções

**Solução:**
```bash
# Rodar o teste oficial de métricas mais de uma vez
make test-metrics
make test-metrics

# Comparar os CSVs gerados
diff -u dados_graficos/csv/metricas_4cores.csv dados_graficos/csv/metricas_4cores.csv.bak

# Se precisar eliminar concorrência
make test-single-core
```
- Se a variância permanecer alta, habilite logs adicionais no scheduler e investigue possíveis starvation ou filas vazias.

---

### Problema 5: Programa Não Termina

**Sintoma:** Simulador trava sem finalizar

**Debug:**
```bash
# Executar com timeout
timeout 30s ./bin/simulador

# Se travar, verificar logs
tail -f output/output.dat

# Debug com GDB
gdb ./bin/simulador
(gdb) run
# Quando travar: Ctrl+C
(gdb) bt  # Backtrace
```

---

## 📚 Referências Rápidas

### Comandos Essenciais

```bash
# Compilar
make simulador

# Executar padrão
./bin/simulador

# Ver ajuda
./bin/simulador --help

# Limpar build
make clean
```

### Estrutura de Arquivos

```
📁 Entradas:
  examples/programs/*.json      # Programas MIPS
  examples/processes/*.json     # Configurações PCB

📁 Saídas:
  output/resultados.dat         # Métricas de execução
  output/output.dat             # Saída lógica do programa
  logs/multicore/*.csv          # Resultados multicore
  logs/memory/*.csv             # Utilização de memória
  logs/metrics/*.csv            # Métricas detalhadas
```

### Opções CLI

| Opção | Valores | Padrão | Descrição |
|-------|---------|--------|-----------|
| `--cores` | 1-8 | 2 | Número de núcleos |
| `--quantum` | 1-1000 | 100 | Quantum em ciclos |
| `--policy` | RR, FCFS, SJN, PRIORITY | RR | Política de escalonamento |
| `--process` | PROG PCB | tasks.json process1.json | Par programa+processo |

---

## 🎯 Checklist de Execução Completa

- [ ] Compilar o simulador (`make simulador`)
- [ ] Executar com a configuração padrão (`make run-sim`)
- [ ] Rodar `make test-metrics` e arquivar `dados_graficos/csv/metricas_4cores.csv`
- [ ] Rodar `make test-single-core` para confirmar execução determinística
- [ ] Validar registradores com `make test-hash` e `make test-bank`
- [ ] Exportar gráficos/relatórios desejados

---

Para mais informações, consulte:
- [README.md](README.md) - Documentação técnica completa
- [docs/MAKEFILE_COMMANDS.md](docs/MAKEFILE_COMMANDS.md) - Referência de comandos Make
- `./bin/simulador --help` - Ajuda integrada
