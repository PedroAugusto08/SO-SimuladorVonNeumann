# 📋 ANÁLISE DE REQUISITOS - TRABALHO FINAL
**Data:** 25/11/2025  
**Prazo de Entrega:** 06/12/2025  
**Status:** ✅ QUASE COMPLETO (95%)

---

## 🎯 RESUMO EXECUTIVO

| Categoria | Status | Completude |
|-----------|--------|------------|
| **Arquitetura Multicore** | ✅ COMPLETO | 100% |
| **Escalonadores** | ✅ COMPLETO | 100% |
| **Gerenciamento de Memória** | ✅ COMPLETO | 100% |
| **Métricas de Desempenho** | ⚠️ PARCIAL | 60% |
| **Testes Comparativos** | ✅ COMPLETO | 100% |
| **Artigo IEEE** | ❌ PENDENTE | 0% |

**PONTUAÇÃO ESTIMADA:** 16-18/20 pontos (falta artigo + algumas métricas)

---

## ✅ REQUISITOS IMPLEMENTADOS

### 1. ✅ ARQUITETURA MULTICORE (100%)

#### Requisito: "n núcleos compartilhando memória principal unificada"
**STATUS:** ✅ COMPLETO

**Implementação:**
- ✅ Classe `Core` com pipeline MIPS completo
- ✅ Cache L1 privada por núcleo
- ✅ Memória RAM compartilhada (thread-safe)
- ✅ Execução assíncrona com threads
- ✅ Configurável: 1, 2, 4, 6 núcleos testados

**Arquivos:**
```cpp
src/cpu/Core.hpp             // Definição do núcleo
src/cpu/Core.cpp             // Implementação
src/cpu/CONTROL_UNIT.cpp     // Pipeline de execução
```

---

### 2. ✅ CARREGAMENTO DE PROGRAMAS (100%)

#### Requisito: "Ler lote inicial de programas do disco"
**STATUS:** ✅ COMPLETO

**Implementação:**
- ✅ Parser JSON para programas MIPS
- ✅ Carregamento completo antes da execução
- ✅ Sem chegada de novos processos durante execução
- ✅ Mapeamento de endereços base

**Arquivos:**
```cpp
src/parser_json/parser_json.cpp   // Leitura de JSON
src/cpu/pcb_loader.cpp            // Carregamento de PCB
tasks.json                         // Programas de teste
```

**Exemplo de uso:**
```cpp
loadJsonProgram("tasks.json", memManager, *pcb, base_address);
```

---

### 3. ✅ GERENCIAMENTO DE MEMÓRIA (100%)

#### Requisito: "Modelo inspirado em Tanenbaum com endereçamento por blocos"
**STATUS:** ✅ COMPLETO

**Implementação:**
- ✅ Memória principal (4KB-8KB configurável)
- ✅ Memória secundária (disco)
- ✅ Cache L1 privada por núcleo
- ✅ Cache L2 compartilhada
- ✅ Tradução de endereços (bloco + deslocamento)
- ✅ **2 Políticas de substituição:**
  - FIFO (First In First Out)
  - LRU (Least Recently Used) ⭐ NOVO!

**Arquivos:**
```cpp
src/memory/MemoryManager.hpp       // Gerenciador central
src/memory/cache.hpp               // Cache L1/L2
src/memory/cachePolicy.cpp         // FIFO + LRU
src/memory/MAIN_MEMORY.cpp         // RAM
src/memory/SECONDARY_MEMORY.cpp    // Disco
```

**Características:**
- Thread-safe com mutexes
- Métricas de cache hits/misses
- Page faults tratados
- Swapping entre RAM e disco

---

### 4. ✅ POLÍTICAS DE ESCALONAMENTO (100%)

#### Requisito: "FCFS, SJN, Round Robin, Prioridade"
**STATUS:** ✅ COMPLETO + BÔNUS

**Implementação - 5 POLÍTICAS:**

#### 4.1 ✅ FCFS (First Come First Served)
- Não-preemptivo
- Ordem de chegada
- Arquivo: `src/cpu/FCFSScheduler.cpp`

#### 4.2 ✅ SJN (Shortest Job Next)
- Não-preemptivo
- Ordenado por tamanho estimado do job
- Arquivo: `src/cpu/SJNScheduler.cpp`

#### 4.3 ✅ Round Robin
- **Preemptivo** com quantum configurável
- Quantum padrão: 1000 ciclos
- Context switch completo
- Arquivo: `src/cpu/RoundRobinScheduler.cpp`
- **Métricas incluídas:**
  ```cpp
  struct Statistics {
      double avg_wait_time;
      double avg_turnaround_time;
      double avg_cpu_utilization;
      double throughput;
      int total_context_switches;
  };
  ```

#### 4.4 ✅ PRIORITY (Não-preemptivo)
- Ordenação por prioridade estática
- Valores maiores = maior prioridade
- Arquivo: `src/cpu/PriorityScheduler.cpp`

#### 4.5 ✅ PRIORITY_PREEMPT (Preemptivo) ⭐ NOVO!
- **Preempção por prioridade**
- Processo de maior prioridade sempre executa
- Context switches automáticos
- Arquivo: `src/cpu/PriorityScheduler.cpp` (mesmo arquivo, quantum usado para compatibilidade)

**Demonstração de preempção:**
```
P1 (prioridade 1) executando...
→ P2 (prioridade 5) chega → PREEMPTA P1 ✅
→ P3 (prioridade 10) chega → PREEMPTA P2 ✅
```

---

### 5. ⚠️ MÉTRICAS DE DESEMPENHO (60%)

#### 5.1 ✅ Métricas Implementadas no RoundRobin

```cpp
Statistics {
    ✅ double avg_wait_time;          // Tempo médio de espera
    ✅ double avg_turnaround_time;    // Tempo médio de retorno
    ✅ double avg_cpu_utilization;    // Utilização da CPU
    ✅ double throughput;              // Taxa de throughput
    ✅ int total_context_switches;    // Trocas de contexto
}
```

**Arquivo:** `src/cpu/RoundRobinScheduler.cpp` (linhas 356-382)

#### 5.2 ❌ Métricas FALTANDO nos outros escalonadores

**PROBLEMA:** FCFS, SJN, PRIORITY não coletam métricas detalhadas!

**Métricas que FALTAM:**
- ❌ Tempo médio de espera
- ❌ Tempo médio de execução (turnaround)
- ❌ Utilização média da CPU por núcleo
- ❌ Context switches (FCFS/SJN não têm)
- ❌ Eficiência individual por política

**SOLUÇÃO NECESSÁRIA:**
1. Adicionar struct `Statistics` em FCFSScheduler, SJNScheduler, PriorityScheduler
2. Implementar método `get_statistics()` em cada um
3. Coletar tempos de chegada, início, fim em cada PCB
4. Calcular métricas após execução

---

### 6. ✅ TESTES COMPARATIVOS (100%)

#### 6.1 ✅ Teste Multicore Comparativo
**Arquivo:** `test_multicore_comparative.cpp`

**O que testa:**
- ✅ 5 políticas: RR, FCFS, SJN, PRIORITY, PRIORITY_PREEMPT
- ✅ 4 configurações: 1, 2, 4, 6 núcleos
- ✅ 3 iterações + 1 warm-up
- ✅ Remoção de outliers (>1.5σ)
- ✅ Cálculo de CV% (Coeficiente de Variação)
- ✅ Speedup e Eficiência
- ✅ Ranking por configuração
- ✅ Geração de CSV

**Métricas coletadas:**
```cpp
TestResult {
    string policy;
    int num_cores;
    double execution_time_ms;      // ✅
    double speedup;                 // ✅
    double efficiency;              // ✅
    double cv_percent;              // ✅ (confiabilidade)
    int processes_finished;         // ✅
}
```

**Saída gerada:**
- `logs/comparative_5policies.txt` - Relatório completo
- `logs/multicore_comparative_results.csv` - Dados para análise

#### 6.2 ✅ Teste de Preempção
**Arquivo:** `test_priority_preemptive.cpp`

**O que testa:**
- ✅ Preempção por prioridade funciona
- ✅ Processos de alta prioridade interrompem os de baixa
- ✅ Context switches são contabilizados
- ✅ Demonstração clara de preempção

---

### 7. ✅ BASELINE SINGLE-CORE

#### Requisito: "Comparação com arquitetura single-core"
**STATUS:** ✅ COMPLETO

**Implementação:**
- ✅ Teste com 1 núcleo = baseline
- ✅ Speedup calculado: `baseline_time / multicore_time`
- ✅ Eficiência calculada: `speedup / num_cores * 100%`

**Resultados obtidos:**

| Política | 1 Core | 6 Cores | Speedup | Eficiência |
|----------|--------|---------|---------|------------|
| RR | 145.89ms | 115.41ms | 1.26x | 21.1% |
| FCFS | 118.38ms | 108.69ms | 1.09x | 18.2% |
| SJN | 118.02ms | 108.01ms | 1.09x | 18.2% |
| PRIORITY | 117.34ms | 108.14ms | 1.09x | 18.1% |
| PRIORITY_PREEMPT | 117.55ms | 109.25ms | 1.08x | 17.9% |

**Interpretação:**
- Speedup modesto devido a workload pequeno
- RR tem maior overhead de context switches
- Políticas não-preemptivas são ligeiramente mais rápidas

---

### 8. ✅ CENÁRIOS EXPERIMENTAIS

#### 8.1 ✅ Cenário Não-Preemptivo
**Políticas:** FCFS, SJN, PRIORITY (não-preemptivo)
- ✅ Executam até conclusão
- ✅ Sem interrupções
- ✅ Testados e funcionando

#### 8.2 ✅ Cenário Preemptivo
**Políticas:** RR, PRIORITY_PREEMPT
- ✅ Quantum de tempo definido (RR: 1000 ciclos)
- ✅ Context switch preserva estado
- ✅ Processos podem ser interrompidos
- ✅ Testados e funcionando

---

### 9. ✅ LOGS E RELATÓRIOS

#### Requisito: "Gerar arquivos de log com métricas"
**STATUS:** ✅ COMPLETO

**Arquivos gerados:**
```
logs/
├── comparative_5policies.txt          # Relatório completo
├── multicore_comparative_results.csv  # Dados para gráficos
├── comparative_test_final.txt         # Teste anterior (4 políticas)
└── multicore_results.csv              # Resultados antigos
```

**Conteúdo dos logs:**
- ✅ Tempo de execução por política
- ✅ Speedup e eficiência
- ✅ CV% (confiabilidade estatística)
- ✅ Rankings por configuração
- ✅ Análise comparativa

---

## ❌ REQUISITOS PENDENTES

### 1. ❌ ARTIGO IEEE (0%) - **10 PONTOS**

#### Requisito: "Artigo científico no formato IEEE Conference Template"
**STATUS:** ❌ NÃO INICIADO

**Seções necessárias:**
1. ❌ Resumo (Abstract)
2. ❌ Introdução
3. ❌ Referencial Teórico
   - Arquiteturas multicore
   - Escalonadores
   - Gerenciamento de memória
4. ❌ Metodologia e Implementação
5. ❌ Resultados e Discussão
   - Gráficos comparativos
   - Análise de speedup
   - Análise de eficiência
6. ❌ Conclusão e Trabalhos Futuros
7. ❌ Referências

**Template:** https://pt.overleaf.com/latex/templates/ieee-conference-template/grfzhhncsfqn

**Dados disponíveis para o artigo:**
- ✅ Todos resultados em CSV
- ✅ Relatórios de teste
- ✅ Código documentado
- ✅ Métricas de desempenho

---

### 2. ⚠️ MÉTRICAS COMPLETAS (60%) - **AFETA 4 PONTOS**

#### Problema: Métricas detalhadas só no RoundRobin

**O que FALTA implementar:**

#### 2.1 Adicionar Statistics em FCFS/SJN/PRIORITY

```cpp
// Adicionar em FCFSScheduler.hpp, SJNScheduler.hpp, PriorityScheduler.hpp
struct Statistics {
    double avg_wait_time{0.0};
    double avg_turnaround_time{0.0};
    double avg_cpu_utilization{0.0};
    double throughput{0.0};
    int total_context_switches{0};  // 0 para não-preemptivos
};

Statistics get_statistics() const;
```

#### 2.2 Coletar timestamps em PCB

```cpp
// Adicionar em PCB.hpp (se não existir)
struct PCB {
    // ... campos existentes ...
    
    // Timestamps para métricas
    uint64_t arrival_time;      // Tempo de chegada
    uint64_t start_time;        // Primeiro início
    uint64_t finish_time;       // Tempo de conclusão
    uint64_t total_wait_time;   // Tempo total esperando
    
    // Métricas calculadas
    uint64_t turnaround_time() const {
        return finish_time - arrival_time;
    }
    
    uint64_t response_time() const {
        return start_time - arrival_time;
    }
};
```

#### 2.3 Calcular métricas após execução

```cpp
Statistics FCFSScheduler::get_statistics() const {
    Statistics s;
    
    if (finished_list.empty()) return s;
    
    uint64_t total_wait = 0;
    uint64_t total_turnaround = 0;
    
    for (const auto& pcb : finished_list) {
        total_wait += pcb->total_wait_time;
        total_turnaround += pcb->turnaround_time();
    }
    
    s.avg_wait_time = total_wait / (double)finished_list.size();
    s.avg_turnaround_time = total_turnaround / (double)finished_list.size();
    s.throughput = finished_list.size() / total_execution_time;
    s.avg_cpu_utilization = calculate_cpu_utilization();
    s.total_context_switches = 0;  // FCFS não tem
    
    return s;
}
```

**IMPACTO:** Sem isso, não podemos comparar métricas detalhadas entre políticas!

---

### 3. ⚠️ GRÁFICOS PARA O ARTIGO (0%)

**O que FALTA:**
- ❌ Gráfico de tempo de execução vs núcleos
- ❌ Gráfico de speedup por política
- ❌ Gráfico de eficiência por política
- ❌ Gráfico de utilização de memória
- ❌ Gráfico comparativo de context switches

**SOLUÇÃO:** Usar os dados CSV para gerar gráficos no Excel, Python ou Gnuplot

**Exemplo com Python:**
```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('logs/multicore_comparative_results.csv')

# Gráfico de tempo vs núcleos
for policy in df['Policy'].unique():
    data = df[df['Policy'] == policy]
    plt.plot(data['Cores'], data['Time_ms'], label=policy, marker='o')

plt.xlabel('Número de Núcleos')
plt.ylabel('Tempo de Execução (ms)')
plt.title('Desempenho por Política de Escalonamento')
plt.legend()
plt.grid(True)
plt.savefig('desempenho_politicas.png', dpi=300)
```

---

## 📊 RESUMO DE PONTUAÇÃO

### Implementação (20 pontos)

#### Escalonamento (10 pontos)
- ✅ Round Robin preemptivo: **2.5 pts**
- ✅ FCFS não-preemptivo: **2.0 pts**
- ✅ SJN não-preemptivo: **2.0 pts**
- ✅ Priority preemptivo/não-preemptivo: **2.5 pts** (BÔNUS!)
- ⚠️ Métricas incompletas: **-1.0 pts**
- **SUBTOTAL: 8-9/10 pontos**

#### Gerenciamento de Memória (10 pontos)
- ✅ Memória segmentada: **3.0 pts**
- ✅ Cache L1/L2: **3.0 pts**
- ✅ Políticas FIFO + LRU: **3.0 pts**
- ✅ Thread-safe: **1.0 pts**
- **SUBTOTAL: 10/10 pontos** ✅

### Artigo IEEE (10 pontos)
- ❌ Não iniciado: **0/10 pontos**

### TOTAL ESTIMADO: **18-19/30 pontos** (sem artigo)
### TOTAL POSSÍVEL COM ARTIGO: **28-29/30 pontos**

---

## 🎯 PLANO DE AÇÃO PARA 06/12/2025

### ⏰ URGENTE (Próximos 2 dias - 25-27 Nov)

#### Tarefa 1: Adicionar métricas faltantes (4h)
1. ✅ Adicionar timestamps em PCB
2. ✅ Implementar `get_statistics()` em FCFS/SJN/PRIORITY
3. ✅ Atualizar teste comparativo para coletar métricas
4. ✅ Gerar novo CSV com métricas completas

#### Tarefa 2: Gerar gráficos (2h)
1. ✅ Script Python para processar CSV
2. ✅ Gerar gráficos de desempenho
3. ✅ Exportar em alta resolução (300 DPI)

### 📝 ALTA PRIORIDADE (28 Nov - 04 Dez)

#### Tarefa 3: Escrever artigo IEEE (16h)
**Distribuir entre equipe:**

**Membro 1:** Introdução + Referencial Teórico (4h)
- Contexto de sistemas operacionais
- Arquiteturas multicore
- Revisão de escalonadores

**Membro 2:** Metodologia + Implementação (4h)
- Descrição da arquitetura
- Diagramas de classes
- Pseudocódigos dos algoritmos

**Membro 3:** Resultados + Discussão (4h)
- Inserir gráficos
- Análise de desempenho
- Comparação entre políticas

**Membro 4:** Conclusão + Formatação (4h)
- Conclusões gerais
- Trabalhos futuros
- Revisão final
- Referências bibliográficas

### ✅ VERIFICAÇÃO FINAL (05 Dez)

- [ ] Código compilando sem warnings
- [ ] Todos testes passando
- [ ] Métricas completas coletadas
- [ ] Gráficos de alta qualidade
- [ ] Artigo IEEE formatado corretamente
- [ ] Referências bibliográficas completas
- [ ] Repositório GitHub atualizado
- [ ] README.md com instruções de compilação

---

## 📚 REFERÊNCIAS SUGERIDAS PARA O ARTIGO

### Livros

1. **Tanenbaum, A. S.; Bos, H.** (2015). *Modern Operating Systems*. 4th ed. Pearson.
   - Capítulos 2 (Processos), 3 (Memória), 6 (Deadlocks)

2. **Silberschatz, A.; Galvin, P. B.; Gagne, G.** (2018). *Operating System Concepts*. 10th ed. Wiley.
   - Capítulos 5 (CPU Scheduling), 9 (Virtual Memory)

3. **Patterson, D. A.; Hennessy, J. L.** (2017). *Computer Organization and Design: The Hardware/Software Interface*. 5th ed. Morgan Kaufmann.
   - Capítulos sobre pipeline, cache, multicore

### Artigos

4. **Gustafson, J. L.** (1988). "Reevaluating Amdahl's Law". *Communications of the ACM*, 31(5), 532-533.
   - Sobre speedup em sistemas paralelos

5. **Hennessy, J. L.; Patterson, D. A.** (2011). "Computer Architecture: A Quantitative Approach". 5th ed.
   - Métricas de desempenho, benchmarking

---

## 🎓 SUGESTÕES DE MELHORIA (OPCIONAL - BÔNUS)

### Funcionalidades extras que podem dar pontos adicionais:

1. **Visualização gráfica em tempo real**
   - Dashboard mostrando estado dos núcleos
   - Uso de memória ao vivo

2. **Mais políticas de escalonamento**
   - Multilevel Feedback Queue
   - Completely Fair Scheduler (CFS)

3. **Análise de starvation**
   - Detectar processos que nunca executam
   - Métricas de fairness

4. **Otimizações**
   - Cache coherence protocols
   - NUMA (Non-Uniform Memory Access)

5. **Testes de stress**
   - 100+ processos simultâneos
   - Workloads variados

---

## ✅ CHECKLIST FINAL

### Implementação
- [x] Arquitetura multicore funcional
- [x] 5 políticas de escalonamento
- [x] Gerenciamento de memória completo
- [x] 2 políticas de cache (FIFO + LRU)
- [x] Context switch preservando estado
- [ ] Métricas completas em todos escalonadores ⚠️
- [x] Testes comparativos funcionando
- [x] Baseline single-core
- [x] Logs e CSV gerados

### Documentação
- [x] Código comentado
- [x] README atualizado
- [ ] Artigo IEEE ❌
- [ ] Gráficos de desempenho ❌
- [ ] Diagramas de arquitetura ⚠️

### Entrega
- [ ] Repositório GitHub público
- [ ] Código compilando
- [ ] Artigo em PDF
- [ ] Apresentação (se necessário)

---

## 🚀 PRÓXIMOS PASSOS IMEDIATOS

### HOJE (25/11):
1. ✅ Adicionar struct Statistics em FCFS/SJN/PRIORITY
2. ✅ Implementar coleta de timestamps
3. ✅ Testar métricas

### AMANHÃ (26/11):
1. ✅ Gerar gráficos com dados atualizados
2. ✅ Começar escrever artigo (Introdução)

### ESTA SEMANA (27-29/11):
1. ✅ Referencial teórico completo
2. ✅ Metodologia e implementação
3. ✅ Primeira versão do artigo

### PRÓXIMA SEMANA (02-04/12):
1. ✅ Resultados e discussão
2. ✅ Conclusão
3. ✅ Revisão final

### DIA 05/12:
1. ✅ Verificação completa
2. ✅ Correções finais
3. ✅ Preparar entrega

---

## 🎯 CONCLUSÃO

### ✅ O QUE ESTÁ EXCELENTE:
- Arquitetura multicore robusta
- 5 políticas de escalonamento (requisito: 4)
- Gerenciamento de memória completo
- Testes comparativos detalhados
- Código bem estruturado e comentado

### ⚠️ O QUE PRECISA ATENÇÃO:
- Métricas detalhadas só no RoundRobin
- Gráficos ainda não gerados
- Diagramas de arquitetura faltando

### ❌ O QUE É CRÍTICO:
- **ARTIGO IEEE NÃO INICIADO (10 pontos em risco!)**

### 💪 PONTOS FORTES DO PROJETO:
- Preempção por prioridade funcionando perfeitamente
- LRU implementado (bônus!)
- 5 políticas testadas (requisito pedia 4)
- Testes estatísticos robustos (CV%, outliers)
- Código production-ready

---

**RESUMO:** Excelente implementação técnica (18-19/20 pontos), mas **URGENTE iniciar o artigo IEEE** para não perder 10 pontos!

**RECOMENDAÇÃO:** Dividir equipe:
- 2 pessoas: adicionar métricas + gerar gráficos (1 dia)
- 4 pessoas: escrever artigo IEEE em paralelo (1 semana)

**BOA SORTE! 🚀📄**
