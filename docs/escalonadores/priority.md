# Priority Scheduler

## Visão Geral

O **Priority Scheduler** executa processos baseado em sua prioridade. Maior número de prioridade = maior importância.

## Modos de Operação

O sistema suporta dois modos:

| Modo | Preempção | Quantum |
|------|-----------|---------|
| **Priority** | Não | Infinito |
| **Priority Preemptivo** | Sim | Configurável |

## Características

| Propriedade | Non-Preemptive | Preemptive |
|-------------|----------------|------------|
| **Tipo** | Não-preemptivo | Preemptivo |
| **Starvation** | Pode ocorrer | Pode ocorrer |
| **Responsividade** | Baixa | Alta |
| **Overhead** | Mínimo | Médio |

## Como Funciona

### Priority (Não-Preemptivo)

```
Fila ordenada por prioridade:
    ┌────────┬────────┬────────┬────────┐
    │P3 (10) │P1 (7)  │P4 (5)  │P2 (3)  │
    └────────┴────────┴────────┴────────┘
         ↓
    Maior prioridade executa primeiro
    Processo NÃO é interrompido
```

### Priority Preemptivo

```
Se processo de maior prioridade chega:
    ┌───────┐
    │P1 (7) │ executando
    └───────┘
         ↓ P3 (10) chega
    ┌────────┐  ┌───────┐
    │P3 (10) │  │P1 (7) │ → volta para fila
    └────────┘  └───────┘
    Preempção!
```

## Implementação

```cpp
class PriorityScheduler {
private:
    std::deque<PCB*> ready_queue;
    int quantum;  // 999999 = não-preemptivo
    
public:
    void add_process(PCB* process) {
        // Inserção ordenada por prioridade (maior primeiro)
        auto it = std::find_if(ready_queue.begin(), ready_queue.end(),
            [&](PCB* p) { 
                return process->priority > p->priority; 
            });
        ready_queue.insert(it, process);
    }
    
    void schedule_cycle() {
        for (auto& core : cores) {
            if (core->is_idle() && !ready_queue.empty()) {
                PCB* next = ready_queue.front();
                ready_queue.pop_front();
                next->quantum = quantum;
                core->execute_async(next);
            }
        }
    }
};
```

## Uso via CLI

```bash
# Priority não-preemptivo
./simulador --policy PRIORITY --cores 2

# Priority preemptivo com quantum
./simulador --policy PRIORITY_PREEMPT --cores 2 --quantum 1000
```

## Definição de Prioridades

No PCB, a prioridade é definida como:

```cpp
pcb->priority = 10;  // Alta prioridade
pcb->priority = 5;   // Média prioridade
pcb->priority = 1;   // Baixa prioridade
```

### Convenção

| Prioridade | Significado | Uso típico |
|------------|-------------|------------|
| 10 | Crítica | Sistema, tempo-real |
| 7-9 | Alta | Interativos |
| 4-6 | Média | Batch normal |
| 1-3 | Baixa | Background |

## Problema de Starvation

Processos de baixa prioridade podem nunca executar:

```
Fila: [P1:10, P2:10, P3:10, P4:1, ...]
                              ↑
                        Nunca executa se P1-P3
                        continuam chegando
```

### Solução: Priority Aging

```cpp
void apply_aging() {
    for (auto& pcb : ready_queue) {
        pcb->wait_cycles++;
        
        // A cada 1000 ciclos de espera, aumenta prioridade
        if (pcb->wait_cycles % 1000 == 0) {
            pcb->priority = std::min(10, pcb->priority + 1);
        }
    }
}
```

## Métricas Esperadas

### Vantagens

- ✅ **Controle fino**: Pode priorizar processos importantes
- ✅ **Flexível**: Adapta-se a diferentes necessidades
- ✅ **Eficiente para tempo-real**: Processos críticos executam primeiro

### Desvantagens

- ❌ **Starvation**: Processos de baixa prioridade podem esperar muito
- ❌ **Inversão de prioridade**: Problema em sistemas com recursos compartilhados
- ❌ **Configuração complexa**: Definir prioridades adequadas é difícil

## Exemplo de Execução

**Processos:**
| Processo | Prioridade | Burst |
|----------|------------|-------|
| P1 | 3 | 100 |
| P2 | 7 | 50 |
| P3 | 5 | 80 |

**Ordem de execução (1 core, não-preemptivo):**
```
P2 (pri=7) → P3 (pri=5) → P1 (pri=3)
0          50          130         230
```

## Inversão de Prioridade

**Problema clássico:**
1. P3 (baixa) adquire lock de recurso
2. P1 (alta) precisa do recurso → bloqueia
3. P2 (média) executa indefinidamente
4. P1 (alta) fica esperando P2 (média) terminar!

**Soluções:**
- **Priority Inheritance**: P3 herda prioridade de P1 temporariamente
- **Priority Ceiling**: Recurso tem prioridade máxima dos usuários

## Comparação com Outras Políticas

| Métrica | FCFS | Round Robin | Priority |
|---------|------|-------------|----------|
| Controle | Nenhum | Nenhum | **Total** |
| Fairness | Médio | Excelente | Ruim |
| Starvation | Não | Não | **Sim** |
| Overhead | Mínimo | Alto | Baixo |

## Quando Usar

✅ **Recomendado para:**
- Sistemas tempo-real
- Quando há processos claramente mais importantes
- Servidores com diferentes níveis de serviço

❌ **Não recomendado para:**
- Sistemas com muitos processos de mesma prioridade
- Quando fairness é importante
- Sistemas sem mecanismo de aging
