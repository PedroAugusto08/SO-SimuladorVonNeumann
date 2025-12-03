# SJN - Shortest Job Next

## Visão Geral

O **SJN (Shortest Job Next)**, também conhecido como **SJF (Shortest Job First)**, é um algoritmo que prioriza processos com menor tempo de execução estimado.

## Características

| Propriedade | Valor |
|-------------|-------|
| **Tipo** | Não-preemptivo |
| **Estrutura** | Fila ordenada por job size |
| **Complexidade** | O(n) inserção, O(1) remoção |
| **Starvation** | ⚠️ Pode ocorrer |
| **Overhead** | Baixo |

## Como Funciona

```
Fila de Prontos (ordenada por tamanho):
    ┌────┬────┬────┬─────┬─────┐
    │P3  │P1  │P4  │P2   │P5   │
    │50  │100 │150 │200  │500  │ → Tamanho estimado
    └────┴────┴────┴─────┴─────┘
      ↓
    Menor job é executado primeiro
```

1. Processo chega → insere na posição **ordenada** por job size
2. Escalonador pega o **menor** processo da fila
3. Processo executa até **terminar**
4. Próximo menor processo é executado

## Implementação

```cpp
class SJNScheduler {
private:
    std::deque<PCB*> ready_queue;  // Ordenada por job size
    std::atomic<int> finished_count{0};
    std::atomic<int> total_count{0};
    
public:
    void add_process(PCB* process) {
        // Só incrementa total_count para processos NOVOS
        if (process->arrival_time == 0) {
            process->arrival_time = cpu_time::now_ns();
            total_count++;
        }
        process->enter_ready_queue();
        ready_queue.push_back(process);
        sort_by_job_size();  // Mantém ordenação
    }
    
    void sort_by_job_size() {
        std::stable_sort(ready_queue.begin(), ready_queue.end(),
            [](PCB* a, PCB* b) {
                return a->estimated_job_size < b->estimated_job_size;
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
                PCB* next = ready_queue.front();  // Menor job
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

1. **Contadores Atômicos**: Evita race conditions entre threads.

2. **Ordenação Estável**: `std::stable_sort` mantém ordem FCFS para jobs de mesmo tamanho.

3. **Verificação de Disponibilidade**: Usa `is_available_for_new_process()` para evitar conflitos.

4. **Coleta Antes de Atribuição**: Processos finalizados são coletados primeiro.

## Uso via CLI

```bash
# SJN com 2 núcleos
./simulador --policy SJN --cores 2

# SJN com 4 núcleos
./simulador --policy SJN --cores 4 \
    -p tasks_small.json process1.json \
    -p tasks_large.json process2.json
```

## Estimativa de Job Size

O sistema usa `program_size` como estimativa:

```cpp
pcb->estimated_job_size = pcb->program_size;
```

**Limitações desta abordagem:**
- Não considera loops (podem executar N vezes)
- Não considera branches
- Não considera I/O wait

**Melhorias possíveis:**
- Histórico de execuções anteriores
- Análise estática do código
- Heurísticas baseadas em tipo de instrução

## Métricas Esperadas

### Vantagens

- ✅ **Menor tempo de espera médio** (ótimo teórico)
- ✅ **Favorece processos curtos** (responsivo)
- ✅ **Alto throughput**
- ✅ **Baixo overhead**

### Desvantagens

- ❌ **Starvation**: Processos longos podem esperar indefinidamente
- ❌ **Requer estimativa**: Precisa conhecer tempo de execução antecipadamente
- ❌ **Não é justo**: Processos longos são penalizados

## Exemplo de Execução

**Processos:**
- P1: 200 ciclos (chega t=0)
- P2: 50 ciclos (chega t=10)
- P3: 100 ciclos (chega t=20)

**Ordem FCFS:**
```
P1 ─────────────────────> P2 ─────> P3 ──────────>
0                       200      250           350
Espera média: (0 + 190 + 230) / 3 = 140
```

**Ordem SJN:**
```
P2 ─────> P3 ──────────> P1 ─────────────────────>
0        50            150                     350
Espera média: (0 + 30 + 130) / 3 = 53.3  ← MELHOR!
```

## Problema de Starvation

**Cenário:**
- 1 processo longo (5000 ciclos)
- Processos curtos chegando continuamente (100 ciclos cada)

```
Fila: [100, 100, 100, 5000, 100, 100, ...]
       ↑    ↑    ↑         ↑    ↑
      Exec Exec Exec      Nunca executa!
```

**Solução: Aging (envelhecimento)**
```cpp
// A cada ciclo, incrementar prioridade de processos esperando
for (auto& pcb : ready_queue) {
    pcb->wait_cycles++;
    if (pcb->wait_cycles > THRESHOLD) {
        pcb->priority_boost = true;  // Aumenta prioridade
    }
}
```

## Comparação com Outras Políticas

| Métrica | FCFS | Round Robin | SJN |
|---------|------|-------------|-----|
| Tempo médio de espera | Médio | Médio | **Mínimo** |
| Starvation | Não | Não | **Sim** |
| Fairness | Médio | **Excelente** | Ruim |
| Throughput | Bom | Médio | **Excelente** |

## Quando Usar

✅ **Recomendado para:**
- Sistemas batch com jobs de tamanhos conhecidos
- Quando minimizar tempo de espera é prioridade
- Workloads previsíveis

❌ **Não recomendado para:**
- Sistemas interativos
- Quando fairness é importante
- Workloads com jobs de tamanhos muito variados
