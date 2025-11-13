# Requisitos Detalhados do Trabalho

## 📋 Especificação Funcional Completa

Esta seção detalha **todos** os requisitos que o simulador deve atender para pontuação máxima.

## 1️⃣ Arquitetura Multicore

### Requisitos Gerais

<div class="alert alert-info">
<strong>Flexibilidade:</strong> O número de núcleos (n) deve ser configurável pela equipe.
</div>

#### RF-ARCH-01: Múltiplos Núcleos
```cpp
// O simulador deve suportar n núcleos configuráveis
class MulticoreArchitecture {
    int num_cores;  // Configurável (2, 4, 8, etc.)
    std::vector<Core*> cores;
    
    MulticoreArchitecture(int n) : num_cores(n) {
        for (int i = 0; i < n; i++) {
            cores.push_back(new Core(i));
        }
    }
};
```

**Critério de Aceitação:**
- ✅ Número de núcleos configurável via argumento/config
- ✅ Cada núcleo independente com pipeline próprio
- ✅ Mínimo 2 núcleos, recomendado 4
- ✅ Todos núcleos funcionais simultaneamente

#### RF-ARCH-02: Memória Compartilhada
```cpp
class SharedMemory {
    MAIN_MEMORY* ram;        // Única RAM para todos
    SECONDARY_MEMORY* disk;  // Único disco para todos
    std::mutex access_lock;  // Controle de concorrência
    
public:
    uint32_t read(uint32_t addr, PCB& process) {
        std::lock_guard<std::mutex> lock(access_lock);
        // Leitura thread-safe
        return ram->ReadMem(addr);
    }
};
```

**Critério de Aceitação:**
- ✅ RAM única compartilhada
- ✅ Disco único compartilhado
- ✅ Acesso sincronizado (sem race conditions)
- ✅ Cache L1 privada por núcleo

#### RF-ARCH-03: Pipeline por Núcleo
**Cada núcleo mantém pipeline MIPS de 5 estágios:**

```
Core 0:  [IF] → [ID] → [EX] → [MEM] → [WB]
Core 1:  [IF] → [ID] → [EX] → [MEM] → [WB]
Core 2:  [IF] → [ID] → [EX] → [MEM] → [WB]
...
```

**Critério de Aceitação:**
- ✅ Pipeline completo por núcleo
- ✅ Execução paralela real (threads)
- ✅ Hazards tratados por núcleo

## 2️⃣ Escalonamento Round Robin

### Requisitos do Escalonador

#### RF-SCHED-01: Algoritmo Round Robin
```cpp
class RoundRobinScheduler {
private:
    std::deque<PCB*> ready_queue;  // Fila FIFO circular
    int quantum;                    // Quantum configurável
    
public:
    PCB* get_next_process() {
        if (ready_queue.empty()) return nullptr;
        
        PCB* process = ready_queue.front();
        ready_queue.pop_front();
        return process;
    }
    
    void requeue_process(PCB* process) {
        if (process->state == State::Ready) {
            ready_queue.push_back(process);  // Fim da fila
        }
    }
};
```

**Critério de Aceitação:**
- ✅ Fila circular (FIFO)
- ✅ Quantum configurável
- ✅ Preempção ao esgotar quantum
- ✅ Processo volta ao final da fila

#### RF-SCHED-02: Quantum de Tempo
```cpp
// O quantum deve ser configurável e respeitado
const int DEFAULT_QUANTUM = 100;  // ciclos

void execute_with_quantum(PCB* process, int quantum) {
    int cycles_executed = 0;
    
    while (cycles_executed < quantum && !process->finished) {
        // Executa 1 ciclo de pipeline
        execute_pipeline_cycle(process);
        cycles_executed++;
    }
    
    if (!process->finished) {
        // Quantum esgotado → context switch
        context_switch(process);
    }
}
```

**Valores Recomendados:**
| Cenário | Quantum | Justificativa |
|---------|---------|---------------|
| Interativo | 50-100 ciclos | Baixa latência |
| Batch | 200-500 ciclos | Alto throughput |
| Misto | 100-200 ciclos | Balanceado |

**Critério de Aceitação:**
- ✅ Quantum respeitado rigorosamente
- ✅ Configurável via arquivo/argumento
- ✅ Métricas de quantum expirado coletadas

#### RF-SCHED-03: Context Switch
```cpp
struct ContextSwitchManager {
    void save_context(PCB* process, Core& core) {
        // Salvar todos registradores
        process->saved_registers = core.register_bank;
        process->saved_pc = core.register_bank.pc.read();
        process->saved_state = core.pipeline_state;
        
        // Métricas
        process->context_switches++;
        process->last_switch_time = get_current_cycle();
    }
    
    void restore_context(PCB* process, Core& core) {
        // Restaurar todos registradores
        core.register_bank = process->saved_registers;
        core.register_bank.pc.write(process->saved_pc);
        core.pipeline_state = process->saved_state;
        
        // Cache warmup pode ser necessário
        core.cache->invalidate();  // Ou política mais sofisticada
    }
};
```

**Overhead do Context Switch:**
- Salvar/restaurar registradores: ~10-20 ciclos
- Invalidação de cache: custo variável
- Total estimado: ~30-50 ciclos

**Critério de Aceitação:**
- ✅ Estado completo salvo/restaurado
- ✅ Overhead contabilizado nas métricas
- ✅ Sem perda de dados entre trocas
- ✅ Context switches contados

#### RF-SCHED-04: Distribuição entre Núcleos

**Estratégia 1: Fila Global (Recomendado)**
```cpp
// Todos núcleos compartilham uma fila
class GlobalQueueScheduler {
    std::deque<PCB*> global_ready_queue;
    std::mutex queue_mutex;
    
    PCB* get_next_for_core(int core_id) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        
        if (global_ready_queue.empty()) return nullptr;
        
        PCB* process = global_ready_queue.front();
        global_ready_queue.pop_front();
        return process;
    }
};
```

**Estratégia 2: Fila por Núcleo (Alternativa)**
```cpp
// Cada núcleo tem sua própria fila
class PerCoreQueueScheduler {
    std::vector<std::deque<PCB*>> per_core_queues;
    
    void distribute_process(PCB* process) {
        // Load balancing: menor fila
        int target_core = find_core_with_shortest_queue();
        per_core_queues[target_core].push_back(process);
    }
};
```

**Critério de Aceitação:**
- ✅ Distribuição justa entre núcleos
- ✅ Balanceamento de carga
- ✅ Sem inanição (starvation)
- ✅ Sincronização adequada

#### RF-SCHED-05: Estados de Processo
```cpp
enum class State {
    New,        // Recém-criado
    Ready,      // Pronto para executar
    Running,    // Executando em um núcleo
    Blocked,    // Esperando I/O
    Finished    // Terminado
};

// Transições permitidas
void validate_state_transition(State from, State to) {
    // New → Ready
    // Ready → Running
    // Running → Ready (quantum expirado)
    // Running → Blocked (espera I/O)
    // Running → Finished (terminou)
    // Blocked → Ready (I/O completou)
}
```

**Diagrama de Estados:**
```
    ┌─────┐
    │ New │
    └──┬──┘
       ↓
    ┌──────┐     ┌─────────┐     ┌──────────┐
    │Ready │ ←──→│ Running │ ───→│ Finished │
    └──┬───┘     └────┬────┘     └──────────┘
       ↑              ↓
       │         ┌────────┐
       └─────────│Blocked │
                 └────────┘
```

## 3️⃣ Gerenciamento de Memória

### Requisitos de Memória

#### RF-MEM-01: Modelo de Segmentação
```cpp
struct MemorySegment {
    uint32_t segment_id;      // ID único do segmento
    uint32_t base_address;    // Endereço físico base
    uint32_t limit;           // Tamanho do segmento
    uint32_t logical_base;    // Base do espaço lógico
    int owner_pid;            // Processo dono
    
    // Permissões
    bool readable;
    bool writable;
    bool executable;
};

class SegmentTable {
    std::vector<MemorySegment> segments;
    
    uint32_t translate(uint32_t logical_addr, int pid) {
        // Encontra segmento do processo
        MemorySegment* seg = find_segment(pid, logical_addr);
        
        if (!seg) throw SegmentationFault();
        
        uint32_t offset = logical_addr - seg->logical_base;
        
        if (offset >= seg->limit) {
            throw SegmentBoundsException();
        }
        
        return seg->base_address + offset;
    }
};
```

**Critério de Aceitação:**
- ✅ Tabela de segmentos implementada
- ✅ Tradução lógico → físico funcional
- ✅ Verificação de limites (bounds checking)
- ✅ Tratamento de erros de segmentação

#### RF-MEM-02: Endereçamento por Palavra
```cpp
// Conforme Tanenbaum: endereços em palavras de x bits

const int WORD_SIZE_BITS = 32;  // 32 bits = 4 bytes
const int BYTE_OFFSET_BITS = 2; // log2(4) = 2

uint32_t byte_to_word_address(uint32_t byte_addr) {
    return byte_addr >> BYTE_OFFSET_BITS;  // Divide por 4
}

uint32_t word_to_byte_address(uint32_t word_addr) {
    return word_addr << BYTE_OFFSET_BITS;  // Multiplica por 4
}

// Estrutura do endereço
struct Address {
    uint32_t block;   // Bits 31-2
    uint32_t offset;  // Bits 1-0
    
    static Address parse(uint32_t addr) {
        return {
            .block = addr >> BYTE_OFFSET_BITS,
            .offset = addr & 0x3  // Últimos 2 bits
        };
    }
};
```

**Critério de Aceitação:**
- ✅ Endereços em palavras de 32 bits
- ✅ Offset interno ao bloco tratado
- ✅ Alinhamento correto

#### RF-MEM-03: Políticas de Substituição

**FIFO (First-In-First-Out)**
```cpp
class FIFOReplacementPolicy {
    std::queue<uint32_t> insertion_order;
    
    uint32_t select_victim() {
        if (insertion_order.empty()) 
            throw NoVictimAvailable();
        
        uint32_t victim = insertion_order.front();
        insertion_order.pop();
        return victim;
    }
    
    void on_page_load(uint32_t page_addr) {
        insertion_order.push(page_addr);
    }
};
```

**LRU (Least Recently Used)**
```cpp
class LRUReplacementPolicy {
    std::list<uint32_t> access_history;  // Mais recente no início
    std::unordered_map<uint32_t, 
        std::list<uint32_t>::iterator> address_to_iter;
    
    uint32_t select_victim() {
        if (access_history.empty()) 
            throw NoVictimAvailable();
        
        uint32_t victim = access_history.back();
        access_history.pop_back();
        address_to_iter.erase(victim);
        return victim;
    }
    
    void on_page_access(uint32_t page_addr) {
        // Remove da posição atual
        auto it = address_to_iter.find(page_addr);
        if (it != address_to_iter.end()) {
            access_history.erase(it->second);
        }
        
        // Insere no início (mais recente)
        access_history.push_front(page_addr);
        address_to_iter[page_addr] = access_history.begin();
    }
};
```

**Critério de Aceitação:**
- ✅ Implementar FIFO **OU** LRU (escolha da equipe)
- ✅ Vítima selecionada corretamente
- ✅ Métricas de page faults coletadas
- ✅ Comparação de desempenho entre políticas (bonus)

#### RF-MEM-04: Hierarquia de Memória

```
┌─────────────┬──────────────┬──────────────┬──────────────┐
│ Nível       │ Tamanho      │ Latência     │ Compartilh.  │
├─────────────┼──────────────┼──────────────┼──────────────┤
│ Cache L1    │ 64-256 KB    │ 1 ciclo      │ Por núcleo   │
│ RAM         │ 1-4 MB       │ 10 ciclos    │ Global       │
│ Disco       │ 10-100 MB    │ 100 ciclos   │ Global       │
└─────────────┴──────────────┴──────────────┴──────────────┘
```

**Critério de Aceitação:**
- ✅ 3 níveis de hierarquia
- ✅ Latências diferenciadas
- ✅ Cache privada, RAM/disco compartilhados
- ✅ Write-back implementado

## 4️⃣ Carga de Processos

### Requisitos de Carregamento

#### RF-LOAD-01: Lote Inicial
```cpp
class ProcessLoader {
    std::vector<std::unique_ptr<PCB>> load_batch(
        const std::string& batch_file) {
        
        std::vector<std::unique_ptr<PCB>> processes;
        
        // Lê todos processos do arquivo JSON/config
        json batch = load_json(batch_file);
        
        for (auto& proc_config : batch["processes"]) {
            auto pcb = create_process_from_config(proc_config);
            
            // Carrega programa na memória
            load_program_to_memory(pcb.get());
            
            processes.push_back(std::move(pcb));
        }
        
        return processes;
    }
};
```

**Formato do Lote (JSON):**
```json
{
  "batch_name": "test_batch_1",
  "processes": [
    {
      "pid": 1,
      "name": "process_A",
      "program_file": "tasks/program_a.json",
      "priority": 1,
      "quantum": 100
    },
    {
      "pid": 2,
      "name": "process_B",
      "program_file": "tasks/program_b.json",
      "priority": 1,
      "quantum": 100
    }
  ]
}
```

**Critério de Aceitação:**
- ✅ Todos processos carregados **antes** de iniciar
- ✅ Proibido chegada dinâmica durante execução
- ✅ Validação de memória suficiente
- ✅ Erros de carregamento tratados

## 5️⃣ Métricas e Instrumentação

### Requisitos de Medição

#### RF-METRIC-01: Métricas por Processo
```cpp
struct ProcessMetrics {
    // Tempos
    uint64_t arrival_time;        // Quando entrou no sistema
    uint64_t start_time;          // Primeira vez executando
    uint64_t finish_time;         // Quando terminou
    uint64_t wait_time;           // Tempo em ready
    uint64_t execution_time;      // Tempo executando
    uint64_t turnaround_time;     // Tempo total
    
    // Escalonamento
    uint64_t context_switches;    // Quantas trocas
    uint64_t quantum_expirations; // Quantas vezes expirou quantum
    uint64_t core_migrations;     // Mudanças de núcleo
    
    // Memória
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t page_faults;
    uint64_t memory_accesses;
    
    // Calculadas
    double cache_hit_rate() {
        return (double)cache_hits / (cache_hits + cache_misses);
    }
    
    uint64_t get_wait_time() {
        return turnaround_time - execution_time;
    }
};
```

**Critério de Aceitação:**
- ✅ Todas métricas coletadas
- ✅ Valores precisos (não estimados)
- ✅ Exportação para arquivo CSV
- ✅ Logs detalhados disponíveis

#### RF-METRIC-02: Métricas Globais
```cpp
struct SystemMetrics {
    int num_cores;
    int num_processes;
    uint64_t total_cycles;
    
    // Agregadas
    double avg_wait_time;
    double avg_turnaround_time;
    double avg_cpu_utilization;
    double throughput;
    
    // Por núcleo
    std::vector<double> per_core_utilization;
    std::vector<uint64_t> per_core_cycles;
    
    void calculate() {
        // Tempo médio de espera
        avg_wait_time = sum(all_wait_times) / num_processes;
        
        // Throughput (processos/ciclo)
        throughput = (double)num_processes / total_cycles;
        
        // Utilização da CPU
        uint64_t busy_cycles = sum(all_execution_times);
        avg_cpu_utilization = (double)busy_cycles / 
                              (total_cycles * num_cores);
    }
};
```

**Critério de Aceitação:**
- ✅ Médias calculadas corretamente
- ✅ Utilização por núcleo rastreada
- ✅ Throughput preciso
- ✅ Comparação single vs multi

## 6️⃣ Comparação com Baseline

### Requisitos de Análise

#### RF-COMP-01: Baseline Single-Core
```cpp
// O código atual já é a baseline
class SingleCoreBaseline {
    void run_baseline(std::vector<PCB*> processes) {
        // Executa tudo em 1 núcleo sequencialmente
        for (auto* p : processes) {
            execute_process_to_completion(p);
        }
        
        collect_metrics();
    }
};
```

**Critério de Aceitação:**
- ✅ Mesmos processos executados em ambos
- ✅ Mesmas condições iniciais
- ✅ Métricas comparáveis coletadas

#### RF-COMP-02: Speedup e Eficiência
```cpp
struct PerformanceComparison {
    double speedup;
    double efficiency;
    double overhead;
    
    void calculate(double T_single, double T_multi, int cores) {
        // Speedup: quantas vezes mais rápido
        speedup = T_single / T_multi;
        
        // Eficiência: speedup por núcleo
        efficiency = speedup / cores;
        
        // Overhead: custo de sincronização
        overhead = (T_multi * cores - T_single) / T_single;
    }
};
```

**Speedup Ideal vs Real:**
```
Speedup = T_single / T_multi

Ideal (Lei de Amdahl):
S(n) = 1 / (f_serial + (1-f_serial)/n)

Onde:
- n = número de núcleos
- f_serial = fração serial do código
```

**Critério de Aceitação:**
- ✅ Speedup calculado e reportado
- ✅ Eficiência analisada
- ✅ Overhead quantificado
- ✅ Comparação visual (gráficos)

## 7️⃣ Requisitos Não-Funcionais

### RNF-01: Desempenho
- ✅ Simulação deve executar em < 5 minutos para lote típico
- ✅ Overhead de sincronização < 10% do tempo total
- ✅ Utilização de CPU multicore > 70%

### RNF-02: Confiabilidade
- ✅ Sem race conditions
- ✅ Sem deadlocks
- ✅ Resultados determinísticos (mesma entrada → mesma saída)

### RNF-03: Manutenibilidade
- ✅ Código modular e bem documentado
- ✅ Separação clara de responsabilidades
- ✅ Fácil adicionar novos escalonadores/políticas

### RNF-04: Portabilidade
- ✅ Compila em Linux (Docker/WSL)
- ✅ CMake para build
- ✅ Dependências mínimas (C++17, pthreads)

## 📊 Checklist de Requisitos

Use esta checklist para validar o progresso:

### Arquitetura
- [ ] Múltiplos núcleos configuráveis
- [ ] Memória compartilhada thread-safe
- [ ] Cache L1 privada por núcleo
- [ ] Pipeline MIPS completo por núcleo

### Escalonamento
- [ ] Round Robin implementado
- [ ] Quantum configurável e respeitado
- [ ] Context switch completo
- [ ] Distribuição entre núcleos

### Memória
- [ ] Segmentação de memória
- [ ] Tradução de endereços
- [ ] Política de substituição (FIFO ou LRU)
- [ ] Hierarquia de 3 níveis

### Métricas
- [ ] Tempos de espera e retorno
- [ ] Utilização de CPU
- [ ] Context switches
- [ ] Cache hits/misses
- [ ] Speedup calculado

### Entrega
- [ ] Código no GitHub público
- [ ] Artigo formato IEEE
- [ ] Comparação com baseline
- [ ] Instruções de compilação

## 📖 Próximos Passos

Agora vamos analisar a arquitetura atual em detalhes:

➡️ [**Arquitetura Atual do Simulador**](03-arquitetura-atual.md)

