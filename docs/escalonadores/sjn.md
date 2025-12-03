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
    
public:
    void add_process(PCB* process) {
        // Inserção ordenada por estimated_job_size (menor primeiro)
        auto it = std::find_if(ready_queue.begin(), ready_queue.end(),
            [&](PCB* p) { 
                return process->estimated_job_size < p->estimated_job_size; 
            });
        ready_queue.insert(it, process);
    }
    
    void schedule_cycle() {
        for (auto& core : cores) {
            if (core->is_idle() && !ready_queue.empty()) {
                PCB* next = ready_queue.front();  // Menor job
                ready_queue.pop_front();
                core->execute_async(next);
            }
        }
    }
};
```

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
