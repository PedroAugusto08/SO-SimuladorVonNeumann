# Arquitetura do Sistema

## Visão Geral

O simulador implementa uma arquitetura Von Neumann multicore com os seguintes componentes principais:

## 1. Núcleos de Processamento (Core)

Cada núcleo é uma instância da classe `Core` que executa processos de forma assíncrona.

### Estrutura do Core

```cpp
class Core {
private:
    int core_id;
    CoreState state;              // IDLE, BUSY, STOPPING
    MemoryManager* memManager;
    IOManager* ioManager;
    cache L1_cache;               // Cache privada
    std::thread execution_thread; // Thread de execução
    PCB* current_process;         // Processo atual

public:
    void execute_async(PCB* process);  // Execução assíncrona
    bool is_idle() const;
    PCB* get_current_process();
};
```

### Pipeline MIPS (5 estágios)

1. **IF** (Instruction Fetch): Busca instrução na memória
2. **ID** (Instruction Decode): Decodifica e lê registradores
3. **EX** (Execute): Executa operação na ULA
4. **MEM** (Memory Access): Acessa memória se necessário
5. **WB** (Write Back): Escreve resultado no registrador

## 2. Process Control Block (PCB)

O PCB armazena todas as informações de um processo:

```cpp
struct PCB {
    // Identificação
    int pid;
    std::string name;
    State state;           // Ready, Running, Blocked, Finished

    // Escalonamento
    int quantum;
    int priority;
    uint64_t arrival_time;
    uint64_t start_time;
    uint64_t finish_time;
    uint64_t total_wait_time;
    uint64_t context_switches;
    int assigned_core;
    uint64_t estimated_job_size;  // Para SJN

    // Memória
    uint64_t cache_hits;
    uint64_t cache_misses;

    // Programa
    uint32_t program_start_addr;
    uint32_t program_size;
    hw::REGISTER_BANK regBank;
};
```

## 3. Escalonadores

Todos os escalonadores herdam uma interface comum:

### FCFSScheduler

```cpp
class FCFSScheduler {
    std::deque<PCB*> ready_queue;      // Fila FIFO
    std::vector<std::unique_ptr<Core>> cores;

    void add_process(PCB* process);    // Adiciona ao final
    void schedule_cycle();             // Atribui a núcleos livres
    bool all_finished() const;
};
```

### RoundRobinScheduler

```cpp
class RoundRobinScheduler {
    std::deque<PCB*> ready_queue;
    int quantum;                        // Fatia de tempo

    void schedule_cycle();              // Com preempção
    bool has_pending_processes() const;
};
```

### SJNScheduler

```cpp
class SJNScheduler {
    std::deque<PCB*> ready_queue;       // Ordenada por job size

    void add_process(PCB* process);     // Inserção ordenada
};
```

### PriorityScheduler

```cpp
class PriorityScheduler {
    std::deque<PCB*> ready_queue;       // Ordenada por prioridade
    int quantum;                         // Para modo preemptivo
};
```

## 4. Gerenciamento de Memória

### MemoryManager

Coordena acesso à memória com sincronização thread-safe:

```cpp
class MemoryManager {
    MainMemory ram;
    SecondaryMemory disk;
    std::shared_mutex memory_mutex;

    uint32_t read(uint32_t address, PCB& process);
    void write(uint32_t address, uint32_t value, PCB& process);
};
```

### Cache L1

Cada núcleo tem sua própria cache:

```cpp
class cache {
    std::map<size_t, uint32_t> cache_data;
    std::queue<size_t> fifo_queue;        // Para política FIFO
    int capacity = 128;                    // Linhas de cache

    bool lookup(size_t address, uint32_t& value);
    void put(size_t address, uint32_t value);
};
```

## 5. Fluxo de Execução

```
1. main() carrega processos de JSON
2. Escalonador recebe processos via add_process()
3. Loop principal:
   a. schedule_cycle() atribui processos a núcleos livres
   b. Núcleos executam assincronamente
   c. Processos terminados são coletados
   d. Métricas são atualizadas
4. Ao final, estatísticas são exibidas/exportadas
```

## 6. Sincronização

O sistema usa mecanismos de sincronização C++17:

- `std::mutex`: Protege filas do escalonador
- `std::shared_mutex`: Protege memória compartilhada (leitura paralela)
- `std::atomic`: Contadores thread-safe no PCB
- `std::thread`: Execução paralela de núcleos

## 7. Diagrama de Classes

```
┌─────────────────────┐
│    main.cpp         │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────┐     ┌─────────────────────┐
│   Scheduler         │────▶│      Core           │
│ (FCFS/SJN/RR/Prio)  │     │                     │
└─────────┬───────────┘     └─────────┬───────────┘
          │                           │
          ▼                           ▼
┌─────────────────────┐     ┌─────────────────────┐
│       PCB           │     │     Pipeline        │
│                     │     │   (5 estágios)      │
└─────────────────────┘     └─────────┬───────────┘
                                      │
                                      ▼
                            ┌─────────────────────┐
                            │   MemoryManager     │
                            │  (Cache/RAM/Disk)   │
                            └─────────────────────┘
```
