# Priority Scheduler (Não-Preemptivo)

## Visão Geral

O **Priority Scheduler** executa processos baseado em sua prioridade. Maior número de prioridade = maior importância.

## Características

| Propriedade | Valor |
|-------------|-------|
| **Tipo** | Não-preemptivo |
| **Preempção** | Não |
| **Starvation** | Pode ocorrer |
| **Responsividade** | Baixa |
| **Overhead** | Mínimo |

## Como Funciona

```
Fila ordenada por prioridade:
    ┌────────┬────────┬────────┬────────┐
    │P3 (10) │P1 (7)  │P4 (5)  │P2 (3)  │
    └────────┴────────┴────────┴────────┘
         ↓
    Maior prioridade executa primeiro
    Processo NÃO é interrompido até terminar
```

O processo de maior prioridade sempre executa primeiro e **não é interrompido** por processos de maior prioridade que chegam posteriormente.

## Implementação

```cpp
class PriorityScheduler {
private:
    std::deque<PCB*> ready_queue;
    std::atomic<int> finished_count{0};
    std::atomic<int> total_count{0};
    
public:
    void add_process(PCB* process) {
        // Só incrementa total_count para processos NOVOS
        if (process->arrival_time == 0) {
            process->arrival_time = cpu_time::now_ns();
            total_count.fetch_add(1);
        }
        process->enter_ready_queue();
        ready_queue.push_back(process);
        sort_by_priority();  // Mantém ordenação
    }
    
    void sort_by_priority() {
        // Ordenação estável por prioridade DECRESCENTE
        std::stable_sort(ready_queue.begin(), ready_queue.end(),
            [](PCB* a, PCB* b) {
                return a->priority > b->priority;
            });
    }
    
    void schedule_cycle() {
        // 1. Coletar processos finalizados
        for (auto& core : cores) {
            PCB* process = core->get_current_process();
            if (process && (core->is_idle() || !core->is_thread_running())) {
                if (process->state == State::Finished) {
                    finished_count.fetch_add(1);
                }
                core->clear_current_process();
            }
        }
        
        // 2. Atribuir processos a núcleos DISPONÍVEIS
        for (auto& core : cores) {
            // Verificação atômica: idle E sem processo pendente
            if (core->is_available_for_new_process() && !ready_queue.empty()) {
                PCB* next = ready_queue.front();
                ready_queue.pop_front();
                core->execute_async(next);
            }
        }
    }
    
    bool all_finished() const {
        return finished_count.load() >= total_count.load() && total_count.load() > 0;
    }
};
```

### Detalhes de Implementação (v2.0)

A implementação atual inclui:

1. **Contadores Atômicos**: `finished_count` e `total_count` são `std::atomic<int>` para thread-safety.

2. **Ordenação Estável**: `std::stable_sort` mantém ordem FCFS para processos de mesma prioridade.

3. **Verificação de Disponibilidade**: Usa `is_available_for_new_process()` para evitar conflitos de atribuição.

4. **Coleta Antes de Atribuição**: Evita sobrescrita de processos não coletados.

5. **Incremento Único**: `total_count` só é incrementado para processos novos (arrival_time == 0).

## Uso via CLI

```bash
# Priority não-preemptivo
./simulador --policy PRIORITY --cores 2
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
