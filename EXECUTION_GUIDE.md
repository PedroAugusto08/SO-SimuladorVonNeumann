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

## 🔬 Testando Escalabilidade + Métricas + Cache (Fluxo Único)

Todos os testes legados (`test-multicore`, `test-multicore-comparative`, `test-throughput`, `test-metrics-complete` e afins) foram consolidados em **um único comando**:

```bash
make test-metrics
```

**O que ele entrega:**
- Executa RR, FCFS, SJN e PRIORITY com 1, 2, 4 e 6 cores
- Calcula tempo, speedup, eficiência e coeficiente de variação (CV)
- Coleta métricas completas (tempo de espera, turnaround, CPU %, throughput)
- Captura contadores de cache/hit-rate por política e número de cores
- Gera todos os CSVs que a GUI consome (agora em `dados_graficos/csv/`) + `dados_graficos/reports/relatorio_comparativo.txt`

**Arquivos gerados em `dados_graficos/csv/`:**
- `unified_complete.csv` – dataset completo (Politica, Cores, Tempo, Speedup, etc.)
- `escalonadores_multicore.csv` – compatibilidade com o formato antigo
- `metricas_escalonadores.csv` – métricas agregadas (referência em 2 cores)
- `memoria_<POLITICA>.csv` – cache hits/misses por número de cores

**Relatório textual:**
- `dados_graficos/reports/relatorio_comparativo.txt` – pronto para apresentações

Além desses datasets legados, cada execução também gera snapshots exclusivos para o número de núcleos definido em `DEFAULT_NUM_CORES`:
- `dados_graficos/csv/metricas_<N>cores.csv`
- `dados_graficos/reports/relatorio_metricas_<N>cores.txt`
onde `<N>` representa a contagem efetiva de cores utilizada na coleta das métricas.

> 💡 Execute `make test-metrics` sempre que precisar atualizar os dados da GUI ou obter um relatório completo. Não é necessário rodar nenhum outro binário auxiliar.

---

## 🧪 Executando Testes Automatizados

### Bateria Completa de Testes

```bash
make test-all
```

**Executa todos os 8 testes ativos:**

1. ✅ Hash Register Test
2. ✅ Register Bank Test
3. ✅ Preemption Test
4. ✅ CPU Metrics Test
5. ✅ Deep Inspection Test
6. ✅ Race Condition Debug Test
7. ✅ Single-Core Serial Test
8. ✅ Metrics Test (gera todos os CSVs/relatórios)

**Tempo estimado:** 5 minutos

---

### Testes Individuais

#### Teste de Registradores

```bash
# Testar sistema de registradores MIPS
make test-hash

# Testar banco de registradores
make test-bank
```

**Valida:**
- Mapeamento correto de 32 registradores MIPS
- Leitura/escrita funcionando
- Proteção do registrador $zero

---

#### Teste de Preempção

```bash
make test-preemption
```

**Valida:**
- Preempção por quantum funciona
- Context switch preserva estado
- PCB salva/restaura corretamente

---

#### Teste de Prioridades

```bash
make test-priority-preemptive
```

**Cenário:**
- 3 processos: alta, média e baixa prioridade
- Valida que processo de alta prioridade executa primeiro
- Verifica preempção quando chega processo mais prioritário

---

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

```bash
make test-race-debug
```

**Valida:**
- Ausência de race conditions no scheduler
- Consistência de contadores atômicos
- Sincronização correta de threads

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
```bash
# Os testes legados foram removidos. Use o alvo unificado:
make test-metrics

# Dúvidas? Confira os comandos atuais
make help
```

---

### Problema 4: Resultados Inconsistentes

**Sintoma:** Speedup/eficiência variam muito entre execuções

**Solução:**
```bash
# O teste de métricas executa 20 iterações + filtro de outliers
make test-metrics

# Verifique a coluna CV_Pct nos CSVs/GUI para estabilidade (< 15%)
```

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

# Rodar todos os testes
make test-all

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

- [ ] Compilar o simulador
- [ ] Executar com configuração padrão
- [ ] Testar as 4 políticas de escalonamento
- [ ] Rodar teste de escalabilidade multicore
- [ ] Executar teste comparativo de políticas
- [ ] Validar throughput com teste confiável
- [ ] Rodar bateria completa de testes (test-all)
- [ ] Analisar resultados em CSV
- [ ] Gerar gráficos de desempenho
- [ ] Validar ausência de race conditions

---

Para mais informações, consulte:
- [README.md](README.md) - Documentação técnica completa
- [docs/MAKEFILE_COMMANDS.md](docs/MAKEFILE_COMMANDS.md) - Referência de comandos Make
- `./bin/simulador --help` - Ajuda integrada
