# Round Robin

## Visão Geral

O **Round Robin** é um algoritmo preemptivo que distribui tempo de CPU igualmente entre processos usando uma fatia de tempo fixa (quantum).

## Características

| Propriedade | Valor |
|-------------|-------|
| **Tipo** | Preemptivo |
| **Estrutura** | Fila circular FIFO |
| **Quantum** | Configurável (padrão: 1000 ciclos) |
| **Starvation** | Não ocorre |
| **Fairness** | Excelente |

## Como Funciona

```
Fila Circular:
         ┌───┐
    ┌───▶│ P1│───┐
    │    └───┘   │
  ┌───┐        ┌───┐
  │ P4│        │ P2│
  └───┘        └───┘
    │    ┌───┐   │
    └────│ P3│◀──┘
         └───┘
         
Cada processo executa por no máximo 'quantum' ciclos
```

1. Processo do início da fila é executado
2. Executa por até `quantum` ciclos
3. Se não terminou → volta para o **final** da fila
4. Próximo processo é executado
5. Repete ciclicamente

## Implementação

```cpp
class RoundRobinScheduler {
private:
    std::deque<PCB*> ready_queue;
    int quantum;
    
public:
    RoundRobinScheduler(int num_cores, MemoryManager* mem, 
                        IOManager* io, int q)
        : quantum(q) {
        // Inicializar núcleos
    }
    
    void schedule_cycle() {
        // 1. Coletar processos terminados/preemptados
        collect_finished_processes();
        
        // 2. Atribuir processos a núcleos livres
        for (auto& core : cores) {
            if (core->is_idle() && !ready_queue.empty()) {
                PCB* next = ready_queue.front();
                ready_queue.pop_front();
                next->quantum = quantum;
                core->execute_async(next);
            }
        }
    }
    
    void handle_preemption(PCB* process) {
        if (process->state != State::Finished) {
            process->context_switches++;
            ready_queue.push_back(process);  // Volta para fila
        }
    }
};
```

## Uso via CLI

```bash
# Round Robin com quantum padrão (1000 ciclos)
./simulador --policy RR --cores 2

# Round Robin com quantum personalizado
./simulador --policy RR --cores 4 --quantum 500

# Round Robin com múltiplos processos
./simulador --policy RR --cores 2 --quantum 1000 \
    -p tasks.json process1.json \
    -p tasks.json process2.json
```

## Escolha do Quantum

O valor do quantum afeta diretamente o comportamento:

| Quantum | Características |
|---------|-----------------|
| **Muito pequeno** (<100) | Alto overhead de context switch |
| **Ideal** (500-2000) | Bom equilíbrio |
| **Muito grande** (>10000) | Comporta-se como FCFS |

### Regra prática

$$quantum \approx 10 \times context\_switch\_time$$

## Context Switch

A cada preempção, o sistema:

1. **Salva** estado do processo atual
   - Registradores
   - Program Counter (PC)
   - Flags de estado

2. **Restaura** estado do próximo processo
   - Carrega registradores
   - Restaura PC
   - Atualiza núcleo

```cpp
void context_switch(PCB* old_proc, PCB* new_proc, Core& core) {
    // Salvar
    old_proc->regBank = core.registers;
    old_proc->pc = core.registers.pc.read();
    
    // Restaurar
    core.registers = new_proc->regBank;
    core.registers.pc.write(new_proc->pc);
    
    // Métricas
    old_proc->context_switches++;
}
```

## Métricas Esperadas

### Vantagens

- ✅ **Fairness**: Todos processos recebem CPU igualmente
- ✅ **Responsivo**: Bom tempo de resposta
- ✅ **Sem starvation**: Todo processo executa regularmente
- ✅ **Previsível**: Tempo máximo de espera = (n-1) × quantum

### Desvantagens

- ❌ **Overhead**: Context switches frequentes
- ❌ **Throughput menor**: Tempo gasto em trocas
- ❌ **Quantum crítico**: Valor inadequado degrada performance

## Exemplo de Execução

**Configuração:**
- Quantum = 4 ciclos
- P1: 8 ciclos, P2: 5 ciclos, P3: 3 ciclos

**Timeline:**
```
     P1    P2    P3    P1    P2    P1
    ├────┼────┼───┼────┼─┼────┤
    0    4    8   11   15 16   20
    
P1: [0-4] → fila → [11-15] → fila → [16-20] FIM
P2: [4-8] → fila → [15-16] FIM
P3: [8-11] FIM
```

**Métricas:**
| Processo | Burst | Turnaround | Espera |
|----------|-------|------------|--------|
| P1 | 8 | 20 | 12 |
| P2 | 5 | 16 | 11 |
| P3 | 3 | 11 | 8 |
| **Média** | | **15.67** | **10.33** |

## Comparação com Outras Políticas

| Métrica | FCFS | SJN | Round Robin |
|---------|------|-----|-------------|
| Fairness | Médio | Ruim | **Excelente** |
| Responsividade | Ruim | Ruim | **Boa** |
| Overhead | Mínimo | Mínimo | Alto |
| Throughput | Bom | **Excelente** | Médio |

## Quando Usar

✅ **Recomendado para:**
- Sistemas de time-sharing
- Ambientes interativos
- Quando fairness é prioridade
- Múltiplos usuários compartilhando CPU

❌ **Não recomendado para:**
- Sistemas real-time
- Quando overhead é crítico
- Workloads com processos muito curtos
