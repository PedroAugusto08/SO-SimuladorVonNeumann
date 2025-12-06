# Histórico de Correções de Bugs

Este documento registra todos os bugs críticos identificados e corrigidos durante o desenvolvimento do simulador.

## Resumo

| Total de Bugs | Resolvidos | Taxa de Sucesso |
|---------------|------------|-----------------|
| 15 | 15 | 100% |

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

---

## Bug #11: Spin-Wait em all_finished() nos Schedulers Não-RR

**Data:** 03/12/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
FCFS, SJN e Priority executavam em 3000-9000ms enquanto Round Robin executava em ~100ms.

```
Round Robin: ~100ms ✓
FCFS:        ~3200ms (32x mais lento!)
SJN:         ~3400ms
Priority:    ~3500ms
```

### Causa Raiz
Os métodos `all_finished()` de FCFS, SJN e Priority tinham verificações complexas que causavam busy-wait:

```cpp
// ANTES (bugado) - verificação complexa causava spin-loop
bool all_finished() const {
    return ready_queue.empty() && 
           blocked_list.empty() && 
           std::all_of(cores.begin(), cores.end(), [](auto& c) {
               return c->get_current_process() == nullptr;
           });
}
```

Enquanto Round Robin usava verificação simples baseada em contadores:

```cpp
// Round Robin (eficiente)
bool has_pending_processes() const {
    return finished_count.load() < total_count.load();
}
```

### Correção
Simplificamos `all_finished()` para usar contadores atômicos:

```cpp
bool all_finished() const {
    int finished = finished_count.load();
    int total = total_count.load();
    
    if (finished >= total && total > 0) {
        for (const auto& core : cores) {
            if (core->get_current_process() != nullptr) return false;
        }
        for (const auto& core : cores) {
            if (core->is_thread_running()) return false;
        }
        return true;
    }
    return false;
}
```

### Arquivos Modificados
- `src/cpu/FCFSScheduler.cpp`
- `src/cpu/SJNScheduler.cpp`
- `src/cpu/PriorityScheduler.cpp`

---

## Bug #12: Contadores Não-Atômicos em PriorityScheduler

**Data:** 03/12/2025  
**Severidade:** Alta  
**Status:** ✅ Resolvido

### Sintoma
Race conditions intermitentes no PriorityScheduler causando CV >100%.

### Causa Raiz
`finished_count` e `total_count` eram `int` normal em PriorityScheduler.

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
`src/cpu/PriorityScheduler.hpp`

---

## Bug #13: Duplicação de total_count em add_process()

**Data:** 03/12/2025  
**Severidade:** Alta  
**Status:** ✅ Resolvido

### Sintoma
Processos sendo contados múltiplas vezes, `total_count` crescendo indefinidamente.

### Causa Raiz
`add_process()` incrementava `total_count` toda vez que era chamado, mesmo para processos preemptados retornando à fila.

### Correção
```cpp
void add_process(PCB* process) {
    if (process->arrival_time == 0) {  // Só incrementa para processos NOVOS
        process->arrival_time = cpu_time::now_ns();
        total_count++;
    }
    process->enter_ready_queue();
    ready_queue.push_back(process);
}
```

### Arquivos Modificados
- `src/cpu/FCFSScheduler.cpp`
- `src/cpu/SJNScheduler.cpp`
- `src/cpu/PriorityScheduler.cpp`

---

## Bug #14: Race Condition em get_current_process()

**Data:** 03/12/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
Falhas intermitentes (CV ~137%) em diferentes combinações de scheduler/cores.

### Causa Raiz
`get_current_process()` não usava lock enquanto `clear_current_process()` usava, criando race condition:

```cpp
// ANTES - leitura sem proteção
PCB* get_current_process() const { 
    return current_process;  // ← Race condition!
}

void clear_current_process() {
    std::lock_guard<std::mutex> lock(core_mutex);
    current_process = nullptr;
}
```

### Correção
```cpp
// DEPOIS - leitura protegida
PCB* get_current_process() const { 
    std::lock_guard<std::mutex> lock(core_mutex);
    return current_process; 
}
```

### Arquivo
`src/cpu/Core.hpp`

---

## Bug #15: Atribuição de Processo a Core com Processo Pendente

**Data:** 03/12/2025  
**Severidade:** Crítica  
**Status:** ✅ Resolvido

### Sintoma
Dois processos tentando usar o mesmo core, causando timeout intermitente.

### Causa Raiz
Schedulers verificavam apenas `is_idle()` para atribuir novos processos:

```cpp
// ANTES - verificação incompleta
if (core->is_idle() && !ready_queue.empty()) {
    core->execute_async(process);  // Pode sobrescrever processo não coletado!
}
```

O problema ocorria porque:
1. Core termina execução e seta `state = IDLE`
2. Scheduler vê `is_idle() == true`
3. Scheduler atribui novo processo ANTES de coletar o anterior
4. Dois processos tentam usar o mesmo core

### Correção
Criamos método `is_available_for_new_process()` que verifica atomicamente:

```cpp
// Novo método no Core.hpp
bool is_available_for_new_process() const {
    std::lock_guard<std::mutex> lock(core_mutex);
    return state.load() == CoreState::IDLE && current_process == nullptr;
}

// Nos schedulers - usar novo método
if (core->is_available_for_new_process() && !ready_queue.empty()) {
    core->execute_async(process);  // Seguro!
}
```

### Arquivos Modificados
- `src/cpu/Core.hpp` (novo método)
- `src/cpu/FCFSScheduler.cpp`
- `src/cpu/SJNScheduler.cpp`
- `src/cpu/PriorityScheduler.cpp`

---

## Impacto das Correções (Atualizado)

### Antes (02/12/2025)
| Métrica | Round Robin | FCFS/SJN/Priority |
|---------|-------------|-------------------|
| Tempo Execução | ~100ms | 3000-9000ms |
| CV (Variabilidade) | <5% | 70-140% |
| Taxa de Sucesso | 100% | ~33% (1 de 3 falhas) |

### Depois (03/12/2025)
| Métrica | Todas as Políticas |
|---------|--------------------|
| Tempo Execução | **~110-125ms** |
| CV (Variabilidade) | **<6%** |
| Taxa de Sucesso | **100%** |
| Speedup | **1.07x-1.12x** |

---

## Resumo das Mudanças por Arquivo

### `src/cpu/Core.hpp`
- Adicionado lock em `get_current_process()`
- Novo método `is_available_for_new_process()`

### `src/cpu/FCFSScheduler.cpp`
- Simplificado `all_finished()` para usar contadores
- `add_process()` só incrementa `total_count` para processos novos
- Adicionado yield/sleep para reduzir busy-wait
- Segunda passagem de coleta após yield
- Usa `is_available_for_new_process()` para atribuição

### `src/cpu/SJNScheduler.cpp`
- Mesmas correções do FCFS
- Mantém ordenação por tamanho de job

### `src/cpu/PriorityScheduler.cpp`
- Mesmas correções do FCFS
- Mantém ordenação por prioridade

### `src/cpu/PriorityScheduler.hpp`
- `finished_count` e `total_count` agora são `std::atomic<int>`

---

## Bug #16: Processos Bloqueados Perdidos em Urgent Collect (Non-Preemptive Schedulers)

**Data:** Junho 2025

### Descrição do Problema
Os escalonadores não-preemptivos (FCFS, SJN, PRIORITY) apresentavam falha esporádica onde o sistema atingia o limite de ciclos antes de concluir todos os processos. A mensagem de erro era:
```
Warning: Reached maximum cycles (200000), exiting main loop
Warning: Not all processes finished! Remaining: X
```

### Causa Raiz
Na seção "urgent collect" dos escalonadores, quando um core ficava ocioso mas ainda tinha um processo antigo associado (devido a race conditions), o código só tratava os estados **Finished** e **Ready**, mas ignorava processos no estado **Blocked**.

**Código problemático:**
```cpp
if (old_process->state == State::Finished) {
    finished_count++;
    finished_processes.insert(old_process->pid);
} else if (old_process->state == State::Ready) {
    ready_queue.push_back(old_process);
}
// Estado Blocked NÃO era tratado → processo perdido!
```

Quando um processo em estado Blocked era encontrado na urgent collect, ele simplesmente era descartado, nunca mais retornando à fila de prontos após completar sua operação de I/O.

### Solução Implementada
Adicionado tratamento específico para o estado **Blocked** na seção urgent collect:

```cpp
if (old_process->state == State::Finished) {
    finished_count++;
    finished_processes.insert(old_process->pid);
} else if (old_process->state == State::Ready) {
    ready_queue.push_back(old_process);
} else if (old_process->state == State::Blocked) {
    // NOVO: Registra processo bloqueado corretamente
    ioManager->registerProcessWaitingForIO(old_process);
    blocked_list.push_back(old_process);
}
```

### Arquivos Modificados
- `src/cpu/FCFSScheduler.cpp` - Linha ~185-200 (urgent collect section)
- `src/cpu/SJNScheduler.cpp` - Mesma correção
- `src/cpu/PriorityScheduler.cpp` - Mesma correção

### Verificação
Após a correção, todos os testes passaram com sucesso para todas as configurações de cores (1, 2, 4, 6):
```
=== 4 CORES ===
RR:       Success=true, CPU 100%, Throughput 3062
FCFS:     Success=true, CPU 100%, Throughput 7663
SJN:      Success=true, CPU 100%, Throughput 4541
PRIORITY: Success=true, CPU 100%, Throughput 5618
```

---

## Análise: Utilização de CPU e Variações de Tempo

### Por que a Utilização de CPU diminui com mais cores?

Com **6 cores** processando apenas **8 processos** simultâneos:
- **Subocupação natural:** 8 processos / 6 cores = 1.33 processos/core
- **Contenção de recursos:** Mais cores disputando acesso a memória, I/O e estruturas compartilhadas
- **Overhead de sincronização:** Mais locks, mais context switches
- **Lei de Amdahl:** Parte sequencial do código limita ganho de paralelismo

**Resultados observados (6 cores):**
| Política | CPU Util | Explicação |
|----------|----------|------------|
| RR | 79.5% | Quantum expira mesmo com poucos processos |
| FCFS | 87.2% | Mais eficiente para poucos processos |
| SJN | 89.9% | Ordenação ajuda na eficiência |
| PRIORITY | 52.9% | Maior overhead de ordenação por prioridade |

### Por que há variação no tempo de execução?

1. **Race Conditions:** Threads competem por recursos, ordem de execução não determinística
2. **Cold Start:** Primeira execução sofre com cache misses e JIT warmup
3. **Scheduler do SO:** Linux kernel pode migrar threads entre CPUs físicas
4. **Contenção de Memória:** False sharing em cache lines compartilhadas

**Solução implementada:** Adicionado **warmup run** em `test_metrics.cpp`:
```cpp
// Warmup - descarta primeira execução
run_policy("RR", num_cores, workloads);
// Agora coleta métricas reais
```

Isso elimina o efeito de cold start e produz medições mais consistentes.
