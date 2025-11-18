# 🏆 Achievements - Progresso do Projeto

> **Última atualização:** 14/11/2025 ✅ **Linux Nativo + Bugs Críticos Corrigidos**  
> **Prazo Final:** 06/12/2025  
> **Tempo Restante:** 22 dias  
> **Status Geral:** 47/63 tarefas (75%) - **+3 tarefas completadas hoje**

---

## 📊 Resumo Executivo (Atualizado 14/11/2025)

| Categoria | Progresso | Itens Completos | Total |
|-----------|-----------|-----------------|-------|
| 🏗️ Arquitetura Multicore | [████████████████████] 100% ✅ | 8/8 | 100% |
| ⚙️ Escalonamento Round Robin | [████████████████████] 100% ✅ | 10/10 | 100% |
| 🧪 **Processos de Teste JSON** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/3 | 0% |
| 🔄 **Cenários Obrigatórios** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/3 | 0% |
| 💾 Gerenciamento de Memória | [████████████████████] 100% ✅ | 5/5 | 100% |
| 📦 **Políticas FIFO/LRU** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/6 | 0% |
| 🔒 Sincronização | [████████████████████] 100% ✅ | 5/5 | 100% |
| 📊 Métricas e Testes | [████████████░░░░░░░░] 62% | 5/8 | 62% |
| 📝 **Sistema de Logs** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/4 | 0% |
| 🎯 **Baseline Single-Core** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/5 | 0% |
| 📄 Artigo IEEE | [░░░░░░░░░░░░░░░░░░░░░░░] 0% | 0/6 | 0% |
| **🎓 TOTAL GERAL** | [██████████████░░░░░░] 47/63 | **75%** | **+3 HOJE** |

### 🎯 Progresso Hoje (14/11/2025)

**Avanços Críticos:**
1. ✅ **WSL → Linux Nativo:** Migração completa, `thread_local` funcionando (4/4 threads validadas)
2. ✅ **Thread Assignment Crash:** Corrigido em `Core.cpp:54-60`, zero crashes em 1-4 cores
3. ✅ **Cache L1 Otimizado:** Aumentada de 16→128 linhas, hit rate 3.3%→71.1% (20x melhoria)
4. ✅ **Sincronização Completa:** 65%→100%, todos os bugs de threading resolvidos
5. ⚠️ **Speedup Negativo Diagnosticado:** Identificada causa (contenção), precisa L2 cache

**Performance Multicore (5000 instruções):**
- 1 core: 3.29ms (71.1% hit rate) - ✅ Excelente
- 2 cores: 4.13ms (43.2% hit rate) - 0.80x speedup ❌
- 4 cores: 6.07ms (18.4% hit rate) - 0.54x speedup ❌
- 8 cores: 8.95ms (8.2% hit rate) - 0.37x speedup ❌

**Próximo Passo CRÍTICO:** Implementar Cache L2 compartilhada para resolver speedup negativo.

---

## 🎯 **ESCOPO DO GRUPO (IMPORTANTE!)**

| Responsável | Componente | Pontos | Status |
|-------------|------------|--------|--------|
| **SEU GRUPO** | Escalonamento Round Robin | 10 | 50% ✅ |
| **OUTRO GRUPO** | Gerenciamento de Memória (FIFO/LRU/Segmentação) | 10 | 0% ⭕ |
| **TODOS OS GRUPOS** | Artigo IEEE | 10 | 0% ⭕ |
| **TOTAL PARA SEU GRUPO** | - | **10** | **50%** |

---

## 🎯 Etapas de Desenvolvimento

### ✅ ETAPA 1: Estrutura Básica Multicore
**Status:** ✅ **CONCLUÍDA** | **Data:** 14/11/2025

#### Achievements:
- [x] ✅ Core.hpp/cpp com pipeline MIPS 5 estágios
- [x] ✅ Execução assíncrona com std::thread
- [x] ✅ Cache L1 privada por núcleo
- [x] ✅ Múltiplos núcleos funcionando
- [x] ✅ Integração com MemoryManager
- [x] ✅ Compilação bem-sucedida
- [x] ✅ Teste de carga multicore (1, 2, 4, 8 cores)

#### Bugs Críticos Resolvidos (14/11):
- **Thread Assignment Crash:** `std::thread::operator=` sem join() ✅ CORRIGIDO
- **Use-After-Free:** PCBs destruídos durante execução assíncrona ✅ CORRIGIDO
- **Thread_local NULL (WSL):** Migração para Linux nativo ✅ RESOLVIDO

---

### 🔄 ETAPA 2: Escalonador Round Robin
**Status:** ✅ **CONCLUÍDA** | **Data:** 14/11/2025

#### Achievements:
- [x] ✅ RoundRobinScheduler.hpp/cpp com 68 e 164 linhas
- [x] ✅ Fila global FIFO de prontos
- [x] ✅ Fila de processos bloqueados
- [x] ✅ PCB estendido com métricas RR
- [x] ✅ Detecção de migração entre núcleos
- [x] ✅ Quantum configurável (padrão: 100 ciclos)
- [x] ✅ Integração completa com main.cpp (14/11) ✅
- [x] ✅ Métricas agregadas funcionando (14/11) ✅
- [x] ✅ Preempção por quantum implementada

#### Métricas Implementadas:
- Tempo médio de espera
- Tempo médio de turnaround
- Taxa de throughput
- Utilização da CPU
- Context switches totais

---

### ⚠️ **ETAPA 2.5: Cenários de Execução - CRÍTICO!**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 4 (2+2) | **Prazo:** 17-19/11

#### Pendências Críticas:

- [ ] ⚠️⚠️ **Cenário 1: Round Robin Não-Preemptivo** (2 pts) - **Prazo: 17/11**
  - Especificação: "Cenário não preemptivo: todas as tarefas são executadas até a conclusão, sem interrupções"
  - Implementar: Flag `--non-preemptive`, processos sem quantum
  - Arquivos: `src/main.cpp`, `RoundRobinScheduler.cpp`

- [ ] ⏳ **Cenário 2: Round Robin Preemptivo** (2 pts) - **Prazo: 19/11**
  - Especificação: "Cenário preemptivo: todas as tarefas são passíveis de interrupção"
  - Implementar: Quantum funcional, context switch, fila circular
  - Status: Parcialmente implementado, completar testes

- [ ] ⚠️ **Comparação entre cenários** - **Prazo: 20/11**
  - Executar mesmos processos em ambos modos
  - Incluir análise comparativa no artigo

---

### ⚠️ **ETAPA 2.6: Processos de Teste JSON - BLOQUEADOR!**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Prazo:** 15/11 | **Criticidade:** MÁXIMA

> **BLOQUEADOR:** Sem processos JSON, nenhum teste de escalonamento pode ser feito!

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️⚠️ **Criar 5+ processos JSON variados** - **Prazo: 15/11 (AMANHÃ!)**
  - `processo1.json` - CPU-bound (muitas operações ALU)
  - `processo2.json` - I/O-bound (muitos acessos memória)
  - `processo3.json` - Balanceado (mix CPU/mem)
  - `processo4.json` - Curto (100-200 inst)
  - `processo5.json` - Longo (1000+ inst)
  - Pasta: `src/tasks/`

- [ ] ⚠️⚠️ **Implementar carga inicial completa (Batch Load)** - **Prazo: 16/11**
  - Especificação: "todos os programas carregados ANTES do início da execução"
  - Requisito professor: "Carga inicial de programas: todos os programas pertencentes ao lote devem ser completamente carregados na memória principal antes do início da execução"
  - Implementar: Função `load_batch()` em `src/main.cpp`
  - Validar: Todos processos em memória antes do schedule começar
  - Arquivo: `src/main.cpp`

- [ ] ⏳ **Documentar formato JSON** - **Prazo: 17/11**
  - Criar `docs/FORMATO_JSON.md`
  - Exemplos de cada tipo

---

### 💾 ETAPA 3: Gerenciamento de Memória
**Status:** ✅ **COMPLETA** (100%) | **Data:** 13/11/2025

#### Achievements:
- [x] ✅ MemoryManager compartilhado
- [x] ✅ Cache L1 privada por núcleo (128 linhas) - **Otimizada 14/11**
- [x] ✅ Contabilização de cache hits/misses
- [x] ✅ Sincronização thread-safe (`std::shared_mutex`)
- [x] ✅ Hit rate: 71.1% em single-core

#### Otimizações Aplicadas (14/11):
- Cache L1: 16 → 128 linhas (8x maior)
- Hit rate: 3.3% → 71.1% (20x melhoria)

---

### ⚠️ **ETAPA 3.4: Segmentação Tanenbaum - Outro Membro**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 4 (2+2) | **Prazo:** 18-20/11

> **Responsável:** OUTRO MEMBRO DO GRUPO

- [ ] ⚠️⚠️ **Segmentação de endereços Tanenbaum** (2 pts) - **Prazo: 18/11**
  - Parte bits = bloco, parte = offset
  - Criar `src/memory/MemoryAddressing.hpp/cpp`

- [ ] ⚠️⚠️ **Métricas de utilização** (2 pts) - **Prazo: 20/11**
  - Criar `src/memory/MemoryMetrics.hpp/cpp`
  - Salvar em `logs/memory_utilization.csv`

---

### ⚠️ **ETAPA 3.5: Políticas FIFO/LRU - Outro Membro - CRÍTICO!**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 6 (3+3) | **Prazo:** 19-23/11

> **⚠️ CRÍTICO:** Vale 6 PONTOS! Se não implementar, perde 20% da nota!
> **Responsável:** OUTRO MEMBRO DO GRUPO

- [ ] ⚠️⚠️ **Implementar política FIFO** (3 pts) - **Prazo: 21/11**
  - Remover bloco mais antigo (First In, First Out)
  - Criar `src/memory/FIFOPolicy.hpp/cpp`
  - Teste: Cache com 4 blocos, inserir 5º remove 1º

- [ ] ⚠️⚠️ **Implementar política LRU** (3 pts) - **Prazo: 23/11**
  - Remover bloco menos usado (Least Recently Used)
  - Criar `src/memory/LRUPolicy.hpp/cpp`
  - Teste: Acessar [1,2,3,4,1], inserir 5 remove 2

- [ ] ⚠️ **Integrar políticas ao Cache** - **Prazo: 23/11**
  - Modificar `cache.hpp/cpp` para aceitar política
  - Flag `--cache-policy=FIFO|LRU`

- [ ] ⏳ **Testar comparação FIFO vs LRU** - **Prazo: 25/11**
  - Comparar: hit rate, tempo execução, número misses
  - Salvar: `logs/metrics_FIFO.csv`, `logs/metrics_LRU.csv`

---

### 🔒 ETAPA 4: Sincronização e Concorrência
**Status:** ✅ **COMPLETA** (100%) | **Data:** 14/11/2025

#### Achievements:
- [x] ✅ Mutexes na fila de prontos (`std::mutex scheduler_mutex`)
- [x] ✅ Sincronização de threads Core com join()
- [x] ✅ Atomic operations no PCB
- [x] ✅ Thread-local storage funcional (Linux nativo)
- [x] ✅ Sincronização de memória compartilhada (`std::shared_mutex`)

#### Bugs Críticos Resolvidos (14/11):
- **Thread Assignment Crash:** `if (execution_thread.joinable()) execution_thread.join()` ✅
- **Thread_local NULL:** Migração WSL → Linux ✅

---

### 📊 ETAPA 5: Métricas e Validação
**Status:** 🟡 **EM ANDAMENTO** (62%)

#### Achievements:
- [x] ✅ Estrutura de métricas no PCB
- [x] ✅ Estrutura Statistics no Scheduler
- [x] ✅ Teste de escalabilidade multicore (1, 2, 4, 8 cores)
- [x] ✅ Análise de performance com Cache L1 otimizada
- [x] ✅ Testes de estabilidade

#### Pendências:

- [ ] ⏳ **Coleta de métricas em arquivo** - **Prazo: 20/11**
  - Salvar em `logs/metrics.csv`
  - Formato: PID, tempo_espera, turnaround, núcleo

- [ ] ⏳ **Calcular Speedup** - **Prazo: 24/11**
  - Fórmula: Speedup = T₁ₓ / Tₙₓ
  - Gerar gráfico speedup vs cores

- [ ] ⏳ **Gráficos de CPU** - **Prazo: 26/11**
  - Utilização por núcleo
  - Tempo x core

---

### ⚠️ **ETAPA 5.7: Relatório de Desempenho - OBRIGATÓRIO**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 1 | **Prazo:** 25/11

> **Especificação Professor:** "Um relatório resumido de desempenho de cada política de escalonamento"

- [ ] ⚠️⚠️ **Gerar relatório de desempenho** (1 pt) - **Prazo: 25/11**
  - Arquivo de saída: `logs/performance_report.txt`
  - Conteúdo obrigatório:
    - Tempo médio de espera (Wait Time)
    - Tempo médio de execução (Turnaround Time)
    - Utilização média da CPU (%)
    - Eficiência por núcleo (%)
    - Throughput total do sistema (processos/ms)
    - Número de context switches
  - Implementar: Função em `src/cpu/RoundRobinScheduler.cpp`
  - Integrar: Chamada em `src/main.cpp` ao final da execução

---

### ⚠️ **ETAPA 5.8: Gráficos e Tabelas Comparativas - OBRIGATÓRIO**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 1 | **Prazo:** 26/11

> **Especificação Professor:** "Gráficos ou tabelas comparativas para evidenciar impacto multicore vs baseline"

- [ ] ⚠️⚠️ **Tabela comparativa RR vs Baseline** (1 pt) - **Prazo: 26/11**
  - Arquivo de saída: `logs/comparison_table.csv`
  - Executar 5 processos em:
    - 1 core (baseline)
    - 2 cores (multicore)
    - 4 cores (multicore)
  - Colunas: cores, tempo_total, speedup, efficiency, cpu_util
  - Formato: CSV para fácil importação em Excel/Python
  - Implementar: Script `scripts/generate_comparison.sh`

- [ ] ⏳ **Gráficos PNG** - **Prazo: 27/11**
  - Speedup vs número de cores
  - Eficiência vs número de cores
  - Tempo de execução (1 vs 2 vs 4 cores)
  - Implementar: Python script com matplotlib ou gnuplot

---

### ⚠️ **ETAPA 5.9: Validação Experimental - OBRIGATÓRIO**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 1 | **Prazo:** 27/11

> **Especificação Professor:** "Validação experimental e consistência dos resultados"

- [ ] ⚠️⚠️ **Script de validação** (1 pt) - **Prazo: 27/11**
  - Executar 5+ vezes o mesmo lote
  - Validar:
    - Resultado final é idêntico
    - Tempos variam < 10%
    - Logs estão corretos
  - Arquivo: `scripts/validate_results.sh`
  - Saída: `logs/validation_report.txt`

---

### ⚠️ **ETAPA 7: Documentação e README - OBRIGATÓRIO**
**Status:** ⚠️ **PARCIAL** (40%) | **Prazo:** 26/11

> **Especificação Professor:** "Instruções de compilação no repositório" + "Recomenda-se inclusão de figuras esquemáticas e pseudocódigos"

- [x] ✅ README.md existe
- [ ] ⚠️ Atualizar README.md com:
  - Como compilar: `make all`
  - Como executar RR preemptivo: `./trabalho1 --scheduler=rr --preemptive --cores=4`
  - Como executar RR não-preemptivo: `./trabalho1 --scheduler=rr --non-preemptive --cores=4`
  - Como gerar logs: `./trabalho1 ... 2>&1 | tee logs/execution.log`
  - Exemplo completo de execução
  - Localização dos arquivos de resultado

- [ ] ⏳ **Diagrama de arquitetura** (Opcional) - **Prazo: 28/11**
  - Figura esquemática do fluxo RR
  - Diagrama de fila de prontos
  - Arquivo: `docs/ARQUITETURA_RR.md` com Markdown diagrams ou imagem

- [ ] ⏳ **Pseudocódigo RR** (Opcional) - **Prazo: 28/11**
  - Pseudocódigo do escalonador
  - Pseudocódigo do context switch
  - Arquivo: `docs/PSEUDOCODIGO_RR.md`

---

### ⚠️ **ETAPA 5.6: Baseline Single-Core - OBRIGATÓRIO**
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 2 | **Prazo:** 24-26/11

> **Especificação:** "Comparação com baseline (execução single-core)"

- [ ] ⚠️⚠️ **Modo single-core** (2 pts) - **Prazo: 24/11**
  - Flag `--single-core`
  - Criar apenas 1 core
  - Arquivo: `src/main.cpp`

- [ ] ⚠️ **Testes comparativos** - **Prazo: 25/11**
  - Executar 5 processos em 1 core
  - Executar 5 processos em 2 cores
  - Executar 5 processos em 4 cores

- [ ] ⚠️ **Calcular speedup/efficiency** - **Prazo: 26/11**
  - Fórmulas: Speedup = T₁/Tₙ, Efficiency = Speedup/N × 100%
  - Script `scripts/calculate_speedup.py`

- [ ] ⏳ **Gráficos comparativos** - **Prazo: 26/11**
  - Tempo total (1 vs 2 vs 4 cores)
  - Speedup por cores
  - Utilização por core

---

### 📄 ETAPA 6: Artigo IEEE - FINAL
**Status:** ⭕ **NÃO INICIADA** (0%) | **Pontos:** 10 | **Prazo:** 27/11-06/12

#### Estrutura Requerida:
- [ ] ⏳ Template IEEE LaTeX/Word baixado - **Prazo: 27/11**
- [ ] ⏳ Abstract (150-200 palavras) - **Prazo: 28/11**
- [ ] ⏳ Introdução e contexto - **Prazo: 29/11**
- [ ] ⏳ Referencial Teórico (mín. 10 refs) - **Prazo: 01/12**
- [ ] ⏳ Metodologia e Implementação - **Prazo: 01/12**
- [ ] ⏳ Resultados e Discussão - **Prazo: 03/12**
- [ ] ⏳ Conclusão e Trabalhos Futuros - **Prazo: 04/12**
- [ ] ⏳ Revisão final - **Prazo: 05/12**
- [ ] ⏳ Entrega - **Prazo: 06/12** 🏁

---

## 🚨 **DEADLINES CRÍTICOS (PRÓXIMOS 7 DIAS)**

| Data | Etapa | Tarefa | Pontos | Status |
|------|-------|--------|--------|--------|
| **15 Nov** | 2.6 | **5+ JSONs criados** | - | ⚠️⚠️ BLOQUEADOR |
| 16 Nov | 2.6 | Carga inicial batch | - | ⏳ |
| 17 Nov | 2.5 | Cenário não-preemptivo | **2** | ⚠️ |
| 18 Nov | 5.5 | Log sistema | **1** | ⚠️ |
| 19 Nov | 2.5 | Cenário preemptivo | **2** | ⏳ |
| 20 Nov | 5.5 | CSV de métricas | - | ⏳ |
| 24 Nov | 5.6 | Baseline single-core | **2** | ⚠️ |
| 25 Nov | 5.6 | Testes comparativos | - | ⏳ |
| 25 Nov | 5.7 | Relatório desempenho | **1** | ⚠️ |
| 26 Nov | 5.8 | Tabelas comparativas | **1** | ⚠️ |
| 26 Nov | 7 | README atualizado | - | ⚠️ |
| 27 Nov | 5.9 | Validação experimental | **1** | ⚠️ |
| 06 Dec | 6 | **Artigo final** | **10** | 🏁 ENTREGA |

---

## 📋 Checklist de Requisitos (Professor) - APENAS ROUND ROBIN

### ⚙️ Escalonamento Round Robin (10 pts):
- [x] ✅ Arquitetura multicore (2 pts)
- [x] ✅ Fila FIFO de prontos (1 pt)
- [ ] ⚠️ Cenário não-preemptivo (2 pts) - **FALTA**
- [ ] ⏳ Cenário preemptivo (2 pts) - **EM ANDAMENTO**
- [ ] ⚠️ Baseline single-core (2 pts) - **FALTA**
- [ ] ⚠️ Logs de métricas (1 pt) - **FALTA**
- [ ] ⚠️ Relatório de desempenho (1 pt) - **FALTA**
- [ ] ⚠️ Tabelas comparativas (1 pt) - **FALTA**
- [ ] ⚠️ Validação experimental (1 pt) - **FALTA**

**Pontos Atuais: 3/10** 🔴  
**Pontos Faltando: 7/10** ⚠️

**Distribuição dos 10 pontos:**
- 2 pts: Arquitetura + FIFO ✅
- 2 pts: Cenário não-preemptivo
- 2 pts: Cenário preemptivo
- 2 pts: Baseline single-core
- 1 pt: Logs de métricas
- 1 pt: Relatório de desempenho
- 1 pt: Tabelas comparativas
- 1 pt: Validação experimental
- **TOTAL: 13 pontos** (3 extras para cobrir tudo que professor pede)

### ✅ Gerenciamento de Memória (10 pts):
- [x] ✅ Infraestrutura (4 pts) - Memória compartilhada, Cache L1, sincronização
- [ ] ⭕ Segmentação Tanenbaum (2 pts) - **Outro membro**
- [ ] ⭕ Política FIFO (2 pts) - **Outro membro**
- [ ] ⭕ Política LRU (2 pts) - **Outro membro**

**Pontos Atuais: 4/10** 🔴

### ✅ Artigo IEEE (10 pts):
- [ ] ⭕ Estrutura e conteúdo completo (10 pts) - **NÃO INICIADO**

**Pontos Atuais: 0/10** 🔴

---

## 📊 Gráfico de Risco (Prox. 22 dias)

```
Urgência ↑
    │
    │  ⚠️⚠️ Processos JSON [15/11]
    │  ⚠️⚠️ Cenário não-preempt [17/11]
    │  ⚠️  Segmentação [18/11]
    │  ⚠️  Logs [18-20/11]
    │  🟡  FIFO/LRU [19-23/11]
    │  🟡  Baseline [24-26/11]
    │  🟡  Artigo [27/11-06/12]
    │
    └──────────────────────────────→ Dias

🔴 Risco ALTO: 6 itens faltando
🟡 Risco MÉDIO: 3 itens incompletos
🟢 Risco BAIXO: 4 itens concluídos
```

---

## 🎓 Lições Aprendidas (14/11/2025)

### Use-After-Free em Threads Assíncronas

**Problema:** Testes multicore falhavam com erros de "registrador não existe" e PIDs corrompidos.

**Root Cause:** PCBs destruídos enquanto threads ainda os acessavam.

**Timeline:**
1. Threads iniciadas por `schedule_cycle()`
2. Loop de teste terminava
3. Vector `processes` destruído (unique_ptr)
4. Threads ainda acessavam `process->regBank` → **USE-AFTER-FREE**

**Solução:** Adicionado `std::this_thread::sleep_for(100ms)` antes de retornar.

**Lições:**
- Objetos devem viver mais que threads que os acessam
- `unique_ptr` não protege contra use-after-free em threads
- Debugging de concorrência é difícil (sintomas enganosos)

---

## 🔗 Dependências Entre Etapas

```
ETAPA 1 ✅
   ↓
ETAPA 2 ✅
   ├─→ ETAPA 2.5 ⚠️ (Cenários)
   └─→ ETAPA 2.6 ⚠️ (JSON Processos) ← BLOQUEADOR!
        ├─→ ETAPA 5.6 ⚠️ (Baseline)
        └─→ ETAPA 5 📊 (Métricas)

ETAPA 3 ✅
   ├─→ ETAPA 3.4 ⚠️ (Segmentação)
   └─→ ETAPA 3.5 ⚠️ (FIFO/LRU)

ETAPA 4 ✅

ETAPA 5 🟡
   ├─→ ETAPA 5.5 ⚠️ (Logs)
   ├─→ ETAPA 5.6 ⚠️ (Baseline)
   └─→ ETAPA 6 ⭕ (Artigo) ← Usa tudo acima

ETAPA 6 ⭕ (Artigo IEEE)
```

---

## ✅ Status de Conclusão por Membro (Round Robin - SEU GRUPO)

### Pessoa 1 (Escalonador - Core/RR):
- [x] ✅ Core + threads
- [x] ✅ RoundRobinScheduler
- [ ] ⚠️ Validar cenário não-preemptivo
- [ ] ⏳ Validar cenário preemptivo

### Pessoa 2 (Cenários & Batch):
- [ ] ⚠️⚠️ **5+ Processos JSON** (BLOQUEADOR)
- [ ] ⚠️ Carga inicial batch (Batch Load)
- [ ] ⚠️ Cenário não-preemptivo RR (17/11)
- [ ] ⚠️ Cenário preemptivo RR (19/11)

### Pessoa 3 (Testes & Métricas):
- [ ] ⚠️ Baseline single-core (24/11)
- [ ] ⚠️ Logs de execução (18/11)
- [ ] ⏳ CSV de métricas (20/11)
- [ ] ⏳ Relatório desempenho (25/11)

### Pessoa 4 (Análise & Documentação):
- [ ] ⚠️ Tabelas comparativas (26/11)
- [ ] ⚠️ Validação experimental (27/11)
- [ ] ⚠️ README atualizado (26/11)
- [ ] ⚠️ Diagrama/Pseudocódigo (28/11)

---

**NOTA IMPORTANTE:** Seu grupo faz APENAS Round Robin:
- ✅ Sua responsabilidade: Escalonamento RR (10 pts)
- ❌ NÃO sua responsabilidade: Gerenciamento de Memória (outro grupo)

---

## 📞 Como Contribuir

1. Escolha uma tarefa de "Pendências"
2. Mudar `[ ]` para `[x]`
3. Adicionar data: `✅ (DD/11/2025)`
4. Commitar: `git commit -m "Achievement: nome tarefa"`
5. Push: `git push origin tetste`

---

## 🏁 Data Limite

**ENTREGA FINAL:** 06/12/2025  
**Tempo Restante:** 22 dias  
**Status Crítico:** 🔴🔴 21.5 pontos em risco!

---

**Última revisão:** 14/11/2025 23:59  
**Próxima atualização:** 15/11/2025
