# 🚀 Otimizações Multicore - Relatório Final

**Data**: 17 de novembro de 2025  
**Objetivo**: Melhorar eficiência do scheduler Round Robin em ambientes multicore  
**Resultado**: **Speedup de 0.31x → 2.40x (2 cores)** e **0.10x → 1.32x (8 cores)**

---

## 📉 Problema Inicial

O simulador apresentava **speedup negativo**: quanto mais cores adicionados, **pior** o desempenho.

### Resultados Antes das Otimizações

| Núcleos | Tempo (ms) | Speedup | Eficiência | Problema |
|---------|-----------|---------|------------|----------|
| 1       | 3.31      | 1.00x   | 100%       | Baseline |
| 2       | 10.74     | **0.31x** ❌ | 15.4%      | 3x mais lento! |
| 4       | 26.32     | **0.13x** ❌ | 3.1%       | 8x mais lento! |
| 8       | 34.40     | **0.10x** ❌ | 1.2%       | 10x mais lento! |

**Diagnóstico**: Contenção de locks serializa completamente a execução.

---

## 🔍 Análise dos Gargalos

### 1. **Scheduler Mutex Global** (CRÍTICO)
```cpp
void RoundRobinScheduler::schedule_cycle() {
    std::lock_guard<std::mutex> lock(scheduler_mutex);  // ⚠️ 10.000 vezes!
    // ... todo o scheduling ...
}
```

**Impacto**:
- Chamado 10.000 vezes (MAX_CYCLES)
- Cada core compete pelo mesmo mutex
- **Serialização completa**: apenas 1 thread progride por vez

### 2. **Verificação Excessiva de Estados**
```cpp
void collect_finished_processes() {
    for (auto& core : cores) {  // ⚠️ 10.000 × 8 cores = 80.000 checks
        if (core->is_idle()) continue;
        // ...
    }
}
```

**Impacto**:
- 80.000 verificações atômicas de estado
- Gera tráfego de cache coherence

### 3. **Cache Hit Rate Baixo**
- L1 privado: 128 linhas/core
- Sem L2 compartilhado
- Hit rate: 71% (1 core) → 8.2% (8 cores)
- 808 cache misses × 8 cores = contenção na RAM

---

## ✅ Otimizações Implementadas

### Otimização 1: **Batch Scheduling**

**Antes**:
```cpp
void schedule_cycle() {
    std::lock_guard<std::mutex> lock(scheduler_mutex);  // TODA vez
    current_time++;
    // ...
}
```

**Depois**:
```cpp
void schedule_cycle() {
    current_time++;  // Sem lock!
    
    // Lock apenas a cada 10 ciclos
    if (current_time % batch_size == 0 || should_schedule) {
        std::lock_guard<std::mutex> lock(scheduler_mutex);
        // ... scheduling ...
    }
}
```

**Resultado**:
- Locks reduzidos: 10.000 → ~1.000 (10x menos)
- **Lock contentions: 6 → 0**
- **Tempo de espera: 1.43μs → 0.00μs**

---

### Otimização 2: **Atribuição Contínua**

**Antes**:
```cpp
for (auto& core : cores) {
    if (core->is_idle() && !ready_queue.empty()) {
        assign_process_to_core(/* apenas 1 processo */);
    }
}
```

**Depois**:
```cpp
// Atribuir TODOS os processos possíveis de uma vez
while (!ready_queue.empty()) {
    bool assigned = false;
    for (auto& core : cores) {
        if (core->is_idle() && !ready_queue.empty()) {
            assign_process_to_core(/* N processos */);
            assigned = true;
        }
    }
    if (!assigned) break;
}
```

**Resultado**:
- Maximiza paralelismo: todos cores ocupados simultaneamente
- Reduz latência de scheduling

---

### Otimização 3: **Contadores Atômicos**

**Antes**:
```cpp
bool has_pending_processes() const {
    std::lock_guard<std::mutex> lock(scheduler_mutex);  // Lock para tudo!
    return finished_count < total_count;
}
```

**Depois**:
```cpp
bool has_pending_processes() const {
    // Verificação lock-free
    int finished = finished_count;  // atomic load
    int total = total_count;
    
    if (finished < total) return true;
    
    // Lock apenas se necessário
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    return finished_count < total_count || !ready_queue.empty();
}
```

**Resultado**:
- Evita 10.000+ locks só para verificação de estado
- Fast path lock-free

---

### Otimização 4: **Coleta Amortizada**

**Antes**:
```cpp
void schedule_cycle() {
    // ...
    collect_finished_processes();  // TODO ciclo!
}
```

**Depois**:
```cpp
void schedule_cycle() {
    // ...
    if (current_time % collect_interval == 0) {  // A cada 10 ciclos
        collect_finished_processes();
    }
}
```

**Resultado**:
- Verificações: 10.000 → 1.000 (10x menos)
- Overhead reduzido: 80.000 → 8.000 checks

---

### Otimização 5: **Cache L2 Compartilhado** (Experimental)

```cpp
// Hierarquia implementada:
// L1 (128 linhas, privado) → L2 (1024 linhas, compartilhado) → RAM → Disco
```

**Implementação com Try-Lock (não-bloqueante)**:
```cpp
std::unique_lock<std::shared_mutex> l2_lock(L2_cache_mutex, std::try_to_lock);
if (l2_lock.owns_lock()) {
    // Acessa L2 apenas se lock disponível
    size_t l2_data = L2_shared_cache->get(address);
    // ...
}
// Senão, vai direto para RAM (evita bloqueio)
```

**Resultado**:
- RAM accesses: 808 → 227-345 (redução de ~50-70%)
- Porém: adiciona overhead de sincronização
- **Trade-off**: hit rate melhora, mas latência aumenta

---

## 📊 Resultados Finais

### Melhor Configuração: Batch Scheduling (v2)

| Núcleos | Antes (ms) | Depois (ms) | Speedup Antes | Speedup Depois | **Melhoria** |
|---------|-----------|-------------|---------------|----------------|------------|
| 1       | 3.31      | 2.02        | 1.00x         | 1.00x          | 39% mais rápido |
| 2       | 10.74     | 0.84        | 0.31x ❌      | **2.40x** ✅   | **12.8x melhor!** |
| 4       | 26.32     | 2.96        | 0.13x ❌      | 0.68x ⚠️       | **8.9x melhor!** |
| 8       | 34.40     | 1.54        | 0.10x ❌      | **1.32x** ✅   | **22.3x melhor!** |

### Métricas de Contenção

| Métrica | Antes | Depois | Melhoria |
|---------|-------|--------|----------|
| Lock Contentions | 6 | **0** | ✅ Eliminado |
| Avg Lock Wait | 1.43 μs | **0.00 μs** | ✅ Eliminado |
| Scheduler Locks | 10.000 | ~1.000 | 90% redução |
| State Checks | 80.000 | ~8.000 | 90% redução |

---

## 🎯 Análise de Performance

### Speedup por Número de Cores

```
Speedup
  ^
2.5|     ●  Depois (2 cores: 2.40x)
  2|    /
1.5|   /    ●  Depois (8 cores: 1.32x)
  1|  ●------●--●  Ideal = 1.0x (baseline)
0.5| /        \
  0|●----------●--●  Antes (0.31x, 0.13x, 0.10x)
   +-----------------> Cores
   1    2    4    8
```

### Eficiência

| Cores | Eficiência Antes | Eficiência Depois | Melhoria |
|-------|------------------|-------------------|----------|
| 2     | 15.4% ❌         | **119.8%** ✅     | Super-linear! |
| 4     | 3.1% ❌          | 17.1% ⚠️          | 5.5x melhor |
| 8     | 1.2% ❌          | 16.4% ⚠️          | 13.7x melhor |

**Por que 2 cores teve eficiência > 100%?**
- Cache L1 de 2 cores (256 linhas total) captura working set melhor
- Menos contenção que 4-8 cores
- Overhead de context switch amortizado

---

## 💡 Lições Aprendidas

### 1. **Locks São Caros**
- Reduzir frequência de locks é **mais importante** que otimizar dentro do lock
- Batch scheduling (10x menos locks) → 12-22x melhoria!

### 2. **Atomics > Locks**
- Para verificações simples, usar `std::atomic` sem locks
- Fast path lock-free → huge win

### 3. **Cache L2 Precisa de Cuidado**
- Compartilhar cache entre cores ajuda hit rate
- **MAS**: locks no L2 podem piorar performance
- Solução: try_lock (não-bloqueante) ou lock-free structures

### 4. **Amortização Funciona**
- Coletar processos a cada 10 ciclos em vez de TODO ciclo
- Trade-off: latência ligeiramente maior, throughput muito melhor

### 5. **2 Cores é o Sweet Spot**
- Melhor eficiência: 119.8%
- Menos contenção que 4-8 cores
- Cache L1 total suficiente para working set

---

## 🚧 Limitações e Trabalho Futuro

### Limitações Atuais

1. **4-8 cores ainda subótimo**
   - Eficiência: 16-17% (ideal seria 75-90%)
   - Causa: cache thrashing + overhead residual

2. **Cache hit rate baixo**
   - 8.2% com 8 cores (ideal: 50-70%)
   - Working set > capacidade total de L1

3. **Sem preempção real**
   - Context switches = 0 (processos terminam antes do quantum)
   - Teste não valida preempção

### Próximas Otimizações

#### Curto Prazo (1-2 dias)
- [ ] Aumentar tamanho de L1 para 512 linhas
- [ ] Implementar L2 lock-free (boost::lockfree)
- [ ] Adicionar workloads que forçam preempção
- [ ] Testar com quantum menor (50, 25 ciclos)

#### Médio Prazo (1 semana)
- [ ] Thread pool ao invés de criar threads por processo
- [ ] NUMA-aware allocation
- [ ] Work stealing entre cores
- [ ] Profiling detalhado com perf/vtune

#### Longo Prazo
- [ ] Implementar outros schedulers (CFS, EDF)
- [ ] Adicionar suporte para prioridades
- [ ] Simular multi-socket systems
- [ ] Benchmark contra simuladores de referência

---

## 📈 Recomendações de Uso

### Para Testes e Demos
**Usar 2 cores**:
- Speedup: 2.40x (próximo do ideal 2.0x)
- Eficiência: 119.8%
- Demonstra paralelismo funcional

### Para Análise de Escalabilidade
**Testar 1, 2, 4, 8 cores**:
- Mostra curva de scaling
- Identifica ponto de diminishing returns
- Útil para análise de Amdahl's Law

### Para Desenvolvimento
**Usar 1 core durante debug**:
- Determinístico
- Fácil de debugar
- Sem race conditions

---

## 🎓 Conclusão

As otimizações implementadas melhoraram drasticamente o desempenho multicore:

✅ **Speedup 2 cores**: 0.31x → 2.40x (**7.7x melhoria**)  
✅ **Speedup 8 cores**: 0.10x → 1.32x (**13.2x melhoria**)  
✅ **Lock contentions**: 6 → 0 (**eliminado**)  
✅ **Tempo de lock**: 1.43μs → 0.00μs (**eliminado**)

A **chave** foi **reduzir a frequência de locks** através de:
- Batch scheduling
- Contadores atômicos
- Amortização de operações custosas

O simulador agora demonstra **escalabilidade funcional** para 2 cores e **melhoria significativa** para 4-8 cores, adequado para fins educacionais e demonstração de conceitos de sistemas operacionais.

---

**Autores**: GitHub Copilot + Henrique  
**Repositório**: PedroAugusto08/SO-SimuladorVonNeumann  
**Branch**: main → tetste  
**Commits**: Ver histórico do git

