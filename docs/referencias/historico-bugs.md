# Histórico de Correções de Bugs

Este documento registra todos os bugs críticos identificados e corrigidos durante o desenvolvimento do simulador.

## Resumo

| Total de Bugs | Resolvidos | Taxa de Sucesso |
|---------------|------------|-----------------|
| 10 | 10 | 100% |

---

## Bug #1: Processos Lendo Memória Não Inicializada

**Data:** 15/11/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
Processos executando instrução END infinitamente, lendo valor `0xffffffff`.

### Causa Raiz
`Core.cpp` verificava variável local `endProgram` ao invés de `context.endProgram`.

### Correção
```cpp
// ANTES (bugado)
while (!endProgram && quantum_remaining > 0) { ... }

// DEPOIS (corrigido)
while (!context.endProgram && quantum_remaining > 0) { ... }
```

### Arquivo
`src/cpu/Core.cpp`, linha 106

---

## Bug #2: PC Sem Verificação de Bounds

**Data:** 15/11/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
PC ultrapassava tamanho do programa, lendo lixo de memória.

### Causa Raiz
`CONTROL_UNIT.cpp` não validava se PC < program_start_addr + program_size.

### Correção
```cpp
uint32_t program_end = context.program_start_addr + context.program_size;
if (context.pc >= program_end) {
    context.endProgram = true;
    return;
}
```

### Arquivo
`src/cpu/CONTROL_UNIT.cpp`, linhas 120-127

---

## Bug #3: Duplicação de Incremento em finished_count

**Data:** 16/11/2025  
**Severidade:** Alta  
**Status:** ✅ Resolvido

### Sintoma
`finished_count` sendo incrementado 2x por processo.

### Causa Raiz
Incremento duplicado em `classify_and_queue_process()` E em `collect_finished_processes()`.

### Correção
Removido incremento duplicado em `collect_finished_processes()`.

### Arquivo
`src/cpu/RoundRobinScheduler.cpp`

---

## Bug #4: Race Conditions em Contadores

**Data:** 16/11/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
`finished_count` e `total_count` dessincronizados entre threads.

### Causa Raiz
Contadores usando `int` normal sem sincronização.

### Correção
```cpp
// ANTES
int finished_count;
int total_count;

// DEPOIS
std::atomic<int> finished_count;
std::atomic<int> total_count;
```

### Arquivo
`src/cpu/RoundRobinScheduler.hpp`

---

## Bug #5: Deadlock em has_pending_processes()

**Data:** 16/11/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
Scheduler travava esperando mutex em método const.

### Causa Raiz
`has_pending_processes()` tentava adquirir lock em método const.

### Correção
```cpp
// Implementação lock-free
bool has_pending_processes() const {
    int finished = finished_count.load(std::memory_order_acquire);
    int total = total_count.load(std::memory_order_acquire);
    return finished < total;
}
```

### Arquivo
`src/cpu/RoundRobinScheduler.cpp`

---

## Bug #6: Processos Perdidos Durante Assignment

**Data:** 17/11/2025  
**Severidade:** Alta  
**Status:** ✅ Resolvido

### Sintoma
Processos desapareciam quando todos cores estavam ocupados.

### Causa Raiz
Loop de assignment terminava após `max_attempts`, perdendo processos na fila.

### Correção
```cpp
while (ready_queue.size() > 0 && idle_cores > 0) {
    int attempts = 0;
    while (attempts < max_attempts && ready_queue.size() > 0) {
        // tentar atribuir
        attempts++;
    }
}
```

### Arquivo
`src/cpu/RoundRobinScheduler.cpp`

---

## Bug #7: Coleta Após Assignment (Race Crítica)

**Data:** 17/11/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
CV de 70-140%, processos sobrescritos antes de serem coletados.

### Causa Raiz
`schedule_cycle()` atribuía novos processos ANTES de coletar os antigos.

### Correção
```cpp
void RoundRobinScheduler::schedule_cycle() {
    collect_finished_processes();  // PRIMEIRO!
    // ... assignment depois
}
```

### Arquivo
`src/cpu/RoundRobinScheduler.cpp`

---

## Bug #8: Urgent-Collect Não Implementado

**Data:** 17/11/2025  
**Severidade:** Alta  
**Status:** ✅ Resolvido

### Sintoma
Processo antigo sobrescrito por novo durante assignment.

### Causa Raiz
Assignment não verificava se core tinha processo antigo antes de atribuir novo.

### Correção
```cpp
PCB* old_process = core->get_current_process();
if (old_process != nullptr) {
    classify_and_queue_process(old_process, core);
    core->clear_current_process();
}
core->execute_async(process);
```

### Arquivo
`src/cpu/RoundRobinScheduler.cpp`

---

## Bug #9: idle_cores Não Incrementado

**Data:** 18/11/2025  
**Severidade:** Alta  
**Status:** ✅ Resolvido

### Sintoma
`has_pending_processes()` retornava true infinitamente.

### Causa Raiz
Urgent-collect limpava core mas não incrementava contador `idle_cores`.

### Correção
```cpp
core->clear_current_process();
idle_cores.fetch_add(1);  // Adicionar esta linha
```

### Arquivo
`src/cpu/RoundRobinScheduler.cpp`, linha 131

---

## Bug #10: Discrepância de Timestamp no Round Robin

**Data:** 25/11/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
Métricas do Round Robin ordens de magnitude diferentes das outras políticas.

```csv
FCFS: 5,031,974 nanoseconds
Round Robin: 898 ciclos  ← ERRADO!
```

### Causa Raiz
Round Robin usava `current_time` (contador de ciclos) ao invés de `std::chrono`.

```cpp
// FCFS/SJN/PRIORITY (correto)
process->finish_time = std::chrono::steady_clock::now().time_since_epoch().count();

// Round Robin (bugado)
process->finish_time = current_time;  // Contador de ciclos!
```

### Correção
Padronizado todos os timestamps para usar `std::chrono::steady_clock`:

```cpp
// arrival_time
if (process->arrival_time == 0) {
    process->arrival_time = std::chrono::steady_clock::now().time_since_epoch().count();
}

// start_time
process->start_time = std::chrono::steady_clock::now().time_since_epoch().count();

// finish_time
process->finish_time = std::chrono::steady_clock::now().time_since_epoch().count();
```

### Arquivos Modificados
- `src/cpu/RoundRobinScheduler.cpp` (5 localizações)
- `src/cpu/RoundRobinScheduler.hpp` (adicionado avg_response_time)

---

## Impacto das Correções

### Antes (14/11/2025)
| Métrica | Valor |
|---------|-------|
| Taxa de Sucesso | 64% |
| CV (Variabilidade) | 70-140% |
| Processos Finalizando | 6-7/8 |
| Speedup | 0.37x-0.80x |

### Depois (25/11/2025)
| Métrica | Valor |
|---------|-------|
| Taxa de Sucesso | **100%** |
| CV (Variabilidade) | **<5%** |
| Processos Finalizando | **8/8** |
| Speedup | **1.10x-1.26x** |

---

## Lições Aprendidas

### 1. Sincronização em Sistemas Assíncronos
- Contadores devem ser `std::atomic`
- Ordem de operações é crítica (coletar ANTES de atribuir)
- Lock-free quando possível para evitar deadlocks

### 2. Consistência de Unidades
- Todas métricas de tempo devem usar mesma unidade
- Padronizar em `std::chrono` para precisão
- Validar com comparação entre componentes

### 3. Testes de Stress
- Executar múltiplas iterações (50+) para detectar race conditions
- Medir CV para avaliar estabilidade
- Bugs intermitentes requerem testes repetidos

### 4. Debugging Sistemático
- Isolar variável suspeita
- Buscar todas ocorrências (grep)
- Comparar implementações corretas vs incorretas
- Validar correção com testes
