# 🧪 Guia Completo de Testes - Simulador Von Neumann Multicore

> **Última atualização:** 25/11/2025  
> **Status:** Documentação completa de todos os testes disponíveis

---

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Testes Disponíveis](#testes-disponíveis)
3. [Como Executar](#como-executar)
4. [Interpretação de Resultados](#interpretação-de-resultados)
5. [Solução de Problemas](#solução-de-problemas)
6. [Criação de Novos Testes](#criação-de-novos-testes)

---

## 🎯 Visão Geral

O simulador possui **10 testes automatizados** cobrindo:
- Performance multicore (1-6 cores)
- Métricas de escalonamento (5 políticas)
- Validação de corretude
- Detecção de race conditions
- Análise comparativa

**Status atual:** ✅ Todos os testes passando (100% success rate)

---

## 📝 Testes Disponíveis

### 1. **test_metrics_complete** ⭐ PRINCIPAL

**Objetivo:** Coletar métricas detalhadas de todas as 5 políticas de escalonamento.

**O que testa:**
- FCFS (First Come First Served)
- SJN (Shortest Job Next)
- Round Robin
- PRIORITY (não-preemptivo)
- PRIORITY_PREEMPT (preemptivo)

**Métricas coletadas:**
- Tempo médio de espera
- Tempo médio de turnaround
- Tempo médio de resposta
- Utilização da CPU (%)
- Throughput (processos/segundo)
- Context switches
- Total de processos

**Saída:**
- Terminal: Tabela formatada com todas métricas
- CSV: `logs/detailed_metrics.csv` (5 linhas, 8 colunas)

**Como executar:**
```bash
make test_metrics_complete
./test_metrics_complete
cat logs/detailed_metrics.csv
```

**Tempo de execução:** ~15 segundos

**Exemplo de saída:**
```
==========================================
  TESTE DE MÉTRICAS COMPLETAS
==========================================

Teste 1/5: FCFS (First Come First Served)
  Tempo médio de espera:         2.00 ciclos
  Tempo médio de turnaround:     4,941,974.50 ciclos
  Tempo médio de resposta:       2,219,352.00 ciclos
  Utilização da CPU:             100.00%
  Throughput:                    36.36 processos/segundo
  Context switches:              0
  Total de processos:            2
  
... (4 políticas adicionais)

CSV salvo em: logs/detailed_metrics.csv
```

**Quando usar:**
- Para coletar dados para o artigo
- Comparar políticas de escalonamento
- Validar métricas após mudanças no código

---

### 2. **test_multicore_comparative** ⭐ PRINCIPAL

**Objetivo:** Análise completa de performance multicore (escalabilidade).

**O que testa:**
- 5 políticas × 4 configurações de cores (1, 2, 4, 6)
- 3 iterações por configuração (após warm-up)
- Total: 60 testes executados

**Métricas coletadas:**
- Tempo de execução (ms)
- Speedup (relativo a 1 core)
- Eficiência (% speedup ideal)
- Coeficiente de variação (CV%)

**Saída:**
- Terminal: Tabela comparativa completa
- CSV: `logs/multicore_comparative_results.csv` (20 linhas)

**Como executar:**
```bash
make test_multicore_comparative
./test_multicore_comparative
cat logs/multicore_comparative_results.csv
```

**Tempo de execução:** ~60 segundos

**Exemplo de saída:**
```
==========================================
  TESTE COMPARATIVO MULTICORE
==========================================

CONFIGURAÇÃO:
  - 5 políticas de escalonamento
  - 4 configurações de núcleos (1, 2, 4, 6)
  - 3 iterações por configuração
  - 1 warm-up por política

[1/60] Executando RR com 1 core (warm-up)...
[2/60] Executando RR com 1 core (iteração 1/3)...
...

RESULTADOS COMPARATIVOS - TODAS AS POLÍTICAS
==============================================

Best at 1 core:  PRIORITY_PREEMPT (118.56ms)
Best at 2 cores: PRIORITY_PREEMPT (115.19ms)
Best at 4 cores: PRIORITY_PREEMPT (112.33ms)
Best at 6 cores: FCFS (113.09ms)

Overall Best: PRIORITY_PREEMPT (114.8ms average)

CSV salvo em: logs/multicore_comparative_results.csv
```

**Formato CSV:**
```csv
Politica,Cores,Tempo_ms,Speedup,Eficiencia_%,CV_%
RR,1,145.95,1.00,100.00,7.00
RR,2,120.61,1.21,60.50,1.48
...
```

**Quando usar:**
- Avaliar escalabilidade de políticas
- Gerar gráficos de speedup
- Comparar eficiência multicore
- Detectar anomalias de performance

---

### 3. **test_multicore_throughput**

**Objetivo:** Medir throughput e variabilidade de uma política específica.

**O que testa:**
- 1, 2, 4, 8 cores
- 10 iterações + 3 warm-ups
- Remoção de outliers (>1.5σ)

**Métricas coletadas:**
- Tempo de execução (ms)
- Speedup
- Eficiência
- CV% (coeficiente de variação)

**Saída:**
- Terminal: Tabela por configuração
- CSV: `logs/multicore_time_results.csv`

**Como executar:**
```bash
make test_multicore_throughput
./test_multicore_throughput
```

**Tempo de execução:** ~30 segundos

**Quando usar:**
- Testar nova política individualmente
- Validar estabilidade (CV < 5%)
- Detectar regressões de performance

---

### 4. **test_race_debug**

**Objetivo:** Detectar race conditions em execução paralela.

**O que testa:**
- 50 iterações consecutivas
- Verificação de 8/8 processos finalizando
- Timeout detection (10 segundos)
- Validação de contadores atômicos

**Saída:**
- Terminal: Taxa de sucesso (X/50)
- Status: PASS/FAIL por iteração

**Como executar:**
```bash
make test_race_debug
./test_race_debug
```

**Tempo de execução:** ~15 segundos

**Exemplo de saída:**
```
==========================================
  TESTE DE RACE CONDITIONS (50x)
==========================================

Iteração 1/50... ✓ 8/8 processos finalizados
Iteração 2/50... ✓ 8/8 processos finalizados
...
Iteração 50/50... ✓ 8/8 processos finalizados

==========================================
RESULTADO FINAL: 50/50 TESTES PASSARAM! ✅
==========================================
Taxa de sucesso: 100.00%
```

**Quando usar:**
- Após mudanças em código de sincronização
- Validar correção de bugs de threading
- Garantir confiabilidade do sistema

---

### 5. **test_verify_execution**

**Objetivo:** Validação completa de execução básica.

**O que testa:**
- Carregamento de programa (tasks.json)
- Execução com 1, 2, 4 cores
- Throughput (processos/segundo)
- Tempo de execução

**Saída:**
- Terminal: Métricas por configuração

**Como executar:**
```bash
make test_verify
./test_verify
```

**Tempo de execução:** ~5 segundos

**Exemplo de saída:**
```
Teste com 1 core:
  Tempo: 149.76ms
  Throughput: 53.42 processos/segundo
  Finalizados: 8/8 ✓

Teste com 2 cores:
  Tempo: 115.42ms
  Throughput: 69.31 processos/segundo
  Finalizados: 8/8 ✓
```

**Quando usar:**
- Smoke test após compilação
- Validar funcionalidade básica
- Teste rápido antes de commits

---

### 6. **test_priority_preemptive**

**Objetivo:** Testar política PRIORITY_PREEMPT isoladamente.

**O que testa:**
- Preempção por prioridade
- Execução multicore
- Métricas básicas

**Como executar:**
```bash
make test_priority_preemptive
./test_priority_preemptive
```

**Tempo de execução:** ~3 segundos

**Quando usar:**
- Debugar PRIORITY_PREEMPT específico
- Validar lógica de preempção

---

### 7. **test_deep_inspection**

**Objetivo:** Debugging profundo de execução de processos.

**O que testa:**
- Estado interno de processos
- Logs detalhados de pipeline
- Inspeção de registradores

**Como executar:**
```bash
make test_deep_inspection
./test_deep_inspection
```

**Quando usar:**
- Debugar bugs complexos
- Inspecionar estado interno
- Análise forense de execução

---

### 8. **test_core**

**Objetivo:** Teste unitário do componente Core.

**O que testa:**
- Inicialização de core
- Cache L1
- Execução básica de processo

**Como executar:**
```bash
# Compilar manualmente
g++ -std=c++17 -pthread test_core.cpp src/cpu/Core.cpp ... -o test_core
./test_core
```

**Quando usar:**
- Testar Core isoladamente
- Validar mudanças em Core.cpp

---

### 9. **test_preemption**

**Objetivo:** Testar mecanismo de preempção por quantum.

**O que testa:**
- Preempção funciona corretamente
- Processos retornam à fila
- Quantum é respeitado

**Como executar:**
```bash
# Compilar via Makefile customizado
make test_preemption
./test_preemption
```

**Quando usar:**
- Validar lógica de preempção
- Debugar problemas de quantum

---

### 10. **test_simple**

**Objetivo:** Teste minimalista de 1 processo em 1 core.

**O que testa:**
- Funcionalidade básica de execução
- Sem multicore, sem escalonamento

**Como executar:**
```bash
# Verificar se existe
ls test_simple
./test_simple
```

**Quando usar:**
- Teste mais simples possível
- Isolar problemas de execução

---

## 🚀 Como Executar

### Pré-requisitos

```bash
# Instalar dependências
sudo apt update
sudo apt install -y build-essential g++ make

# Verificar versão GCC
g++ --version  # Deve ser >= 9.0 para C++17
```

### Compilação

```bash
# Opção 1: Compilar teste específico
make test_metrics_complete
make test_multicore_comparative

# Opção 2: Compilar todos os testes
make clean
make all
```

### Execução

```bash
# Executar teste
./test_metrics_complete

# Redirecionar saída para arquivo
./test_multicore_comparative > results.txt 2>&1

# Executar e salvar apenas CSV
./test_metrics_complete && cat logs/detailed_metrics.csv
```

### Limpeza

```bash
# Limpar binários compilados
make clean

# Limpar logs
rm -f logs/*.csv logs/*.log
```

---

## 📊 Interpretação de Resultados

### Métricas de Tempo

**Tempo médio de espera (Wait Time):**
- Tempo que processo ficou na fila aguardando
- **Menor é melhor**
- SJN tende a ter menor valor (favorece jobs curtos)

**Tempo médio de turnaround:**
- Tempo total: chegada → término
- **Menor é melhor**
- Round Robin pode ter melhor valor (completa mais processos)

**Tempo médio de resposta (Response Time):**
- Tempo até primeira execução
- **Menor é melhor**
- PRIORITY geralmente tem menor valor

### Métricas de Performance

**Throughput (processos/segundo):**
- Quantos processos completam por unidade de tempo
- **Maior é melhor**
- Políticas não-preemptivas tendem a ter maior throughput

**Utilização da CPU (%):**
- Quanto da CPU está sendo usada
- **Próximo de 100% é ideal**
- Valores <90% indicam overhead ou idle time

**Speedup:**
- `Speedup = Tempo(1 core) / Tempo(N cores)`
- **Maior é melhor**
- Speedup ideal = N (linear scaling)
- Speedup < 1.0 indica problema de contenção

**Eficiência (%):**
- `Eficiência = (Speedup / N cores) × 100%`
- **Próximo de 100% é ideal**
- Eficiência cai com mais cores (Lei de Amdahl)

**Coeficiente de Variação (CV%):**
- `CV = (desvio_padrão / média) × 100%`
- **Menor é melhor**
- CV < 5% = excelente confiabilidade
- CV > 10% = instabilidade, possível race condition

### Valores de Referência

**Bom:**
- CV < 5%
- Speedup > 1.0
- Eficiência > 25% (com 4 cores)
- Throughput > 30 proc/s

**Aceitável:**
- CV < 10%
- Speedup > 0.8
- Eficiência > 15%
- Throughput > 20 proc/s

**Ruim (requer investigação):**
- CV > 15%
- Speedup < 0.8 (regressão!)
- Eficiência < 10%
- Throughput < 10 proc/s

---

## 🔧 Solução de Problemas

### Problema: Teste não compila

**Sintomas:**
```
error: 'std::thread' was not declared in this scope
```

**Solução:**
```bash
# Verificar flags de compilação
grep "std=c++17" Makefile
grep "pthread" Makefile

# Recompilar com flags corretos
g++ -std=c++17 -pthread test.cpp -o test
```

---

### Problema: CV muito alto (>10%)

**Sintomas:**
```
CV_% = 23.45  ❌ (instável)
```

**Possíveis causas:**
1. Race conditions não resolvidas
2. Sistema sobrecarregado (rodar em sistema dedicado)
3. Bug em contadores atômicos

**Solução:**
```bash
# Executar test_race_debug para detectar races
./test_race_debug

# Verificar carga do sistema
top

# Isolar sistema (fechar outros programas)
```

---

### Problema: Speedup negativo (<1.0)

**Sintomas:**
```
Speedup = 0.85x  ❌ (regressão)
```

**Possíveis causas:**
1. Contenção de memória (MemoryManager)
2. Cache thrashing
3. Overhead de sincronização

**Solução:**
```bash
# Verificar com diferentes números de cores
./test_multicore_comparative

# Aumentar cache L1
vim src/memory/cache.hpp
# Alterar CACHE_CAPACITY para 256 ou 512
```

---

### Problema: Timeout (teste nunca termina)

**Sintomas:**
```
[Scheduler] Ciclo 10000... (travado)
```

**Possíveis causas:**
1. Deadlock
2. has_pending_processes() retorna true infinito
3. Processo em loop infinito

**Solução:**
```bash
# Adicionar timeout no código
std::chrono::seconds timeout(10);
auto start = std::chrono::steady_clock::now();

while (scheduler.has_pending_processes()) {
    if (std::chrono::steady_clock::now() - start > timeout) {
        std::cout << "TIMEOUT!\n";
        break;
    }
    scheduler.schedule_cycle();
}
```

---

### Problema: CSV não é gerado

**Sintomas:**
```
ls logs/detailed_metrics.csv
No such file or directory
```

**Solução:**
```bash
# Criar diretório logs
mkdir -p logs

# Verificar permissões
chmod 755 logs

# Executar teste novamente
./test_metrics_complete
```

---

## 🛠️ Criação de Novos Testes

### Template Básico

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include "src/cpu/RoundRobinScheduler.hpp"
#include "src/memory/MemoryManager.hpp"
#include "src/IO/IOManager.hpp"

int main() {
    // 1. Configuração
    const int NUM_CORES = 2;
    const int QUANTUM = 1000;
    
    // 2. Inicialização de componentes
    MemoryManager memManager(1024, 2048);
    IOManager ioManager(1, 1, 1);
    
    // 3. Criar scheduler
    RoundRobinScheduler scheduler(NUM_CORES, &memManager, &ioManager, QUANTUM);
    
    // 4. Carregar processos
    std::vector<std::unique_ptr<PCB>> processes;
    // ... (código de carregamento)
    
    // 5. Executar
    while (scheduler.has_pending_processes()) {
        scheduler.schedule_cycle();
    }
    
    // 6. Coletar resultados
    auto stats = scheduler.get_statistics();
    std::cout << "Turnaround: " << stats.avg_turnaround_time << "\n";
    
    return 0;
}
```

### Compilação

```makefile
test_my_test: test_my_test.cpp src/cpu/RoundRobinScheduler.cpp ...
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
```

### Boas Práticas

1. **Sempre usar timeout:**
   ```cpp
   auto start = std::chrono::steady_clock::now();
   std::chrono::seconds timeout(30);
   ```

2. **Validar resultados:**
   ```cpp
   if (finished_count != expected_count) {
       std::cerr << "ERRO: Esperava " << expected_count 
                 << " mas obteve " << finished_count << "\n";
       return 1;
   }
   ```

3. **Imprimir progresso:**
   ```cpp
   std::cout << "[" << i << "/" << total << "] Executando teste...\n";
   ```

4. **Salvar logs:**
   ```cpp
   std::ofstream log("logs/test_results.txt");
   log << "Resultado: " << resultado << "\n";
   ```

---

## 📚 Referências

### Arquivos de Teste

- `test_metrics_complete.cpp` - Linha 1-540
- `test_multicore_comparative.cpp` - Linha 1-641
- `test_multicore_throughput.cpp` - Linha 1-503
- `test_race_debug.cpp` - Linha 1-85
- `test_verify_execution.cpp` - Linha 1-165

### Documentação Relacionada

- `docs/ACHIEVEMENTS.md` - Progresso do projeto
- `docs/COMPILACAO_SUCESSO.md` - Histórico de implementação
- `docs/08-round-robin.md` - Detalhes de Round Robin
- `docs/09-fcfs.md` - Detalhes de FCFS
- `docs/10-sjn.md` - Detalhes de SJN

### Makefile

- Localização: Raiz do projeto
- Targets: `test_metrics_complete`, `test_multicore_comparative`, etc.

---

## 🎯 Quick Reference

| Teste | Tempo | Objetivo | Saída CSV |
|-------|-------|----------|-----------|
| `test_metrics_complete` | 15s | Métricas de 5 políticas | `detailed_metrics.csv` |
| `test_multicore_comparative` | 60s | Performance multicore | `multicore_comparative_results.csv` |
| `test_multicore_throughput` | 30s | Throughput individual | `multicore_time_results.csv` |
| `test_race_debug` | 15s | Detectar races | Nenhum |
| `test_verify_execution` | 5s | Smoke test | Nenhum |

**Comandos rápidos:**

```bash
# Compilar + executar + ver CSV
make test_metrics_complete && ./test_metrics_complete && cat logs/detailed_metrics.csv

# Compilar + executar + ver CSV multicore
make test_multicore_comparative && ./test_multicore_comparative && cat logs/multicore_comparative_results.csv

# Validar estabilidade
./test_race_debug && echo "Sistema estável ✅"
```

---

**Última atualização:** 25/11/2025 01:45  
**Próxima revisão:** 27/11/2025
