# FCFS - First Come First Served

## Visão Geral

O **FCFS (First Come First Served)** é o algoritmo de escalonamento mais simples. Processos são executados na ordem em que chegam à fila de prontos.

## Características

| Propriedade | Valor |
|-------------|-------|
| **Tipo** | Não-preemptivo |
| **Estrutura** | Fila FIFO |
| **Complexidade** | O(1) inserção, O(1) remoção |
| **Starvation** | Não ocorre |
| **Overhead** | Mínimo |

## Como Funciona

```
Fila de Prontos (FIFO):
    ┌───┬───┬───┬───┬───┐
    │ P1│ P2│ P3│ P4│ P5│ → Entrada
    └───┴───┴───┴───┴───┘
      ↓
    Núcleo executa até conclusão
```

1. Processo chega → vai para o **final** da fila
2. Escalonador pega processo do **início** da fila
3. Processo executa até **terminar** (sem preempção)
4. Próximo processo da fila é executado

## Implementação

```cpp
class FCFSScheduler {
private:
    std::deque<PCB*> ready_queue;
    std::vector<std::unique_ptr<Core>> cores;
    
public:
    void add_process(PCB* process) {
        ready_queue.push_back(process);  // Insere no final
    }
    
    void schedule_cycle() {
        // Atribuir processos a núcleos livres
        for (auto& core : cores) {
            if (core->is_idle() && !ready_queue.empty()) {
                PCB* next = ready_queue.front();  // Pega do início
                ready_queue.pop_front();
                core->execute_async(next);
            }
        }
        
        // Coletar processos terminados
        collect_finished_processes();
    }
};
```

## Uso via CLI

```bash
# FCFS com 2 núcleos
./simulador --policy FCFS --cores 2

# FCFS com 4 núcleos e múltiplos processos
./simulador --policy FCFS --cores 4 \
    -p tasks.json process1.json \
    -p tasks.json process2.json
```

## Métricas Esperadas

### Vantagens

- ✅ **Simplicidade**: Fácil de implementar e entender
- ✅ **Determinismo**: Ordem de execução previsível
- ✅ **Sem starvation**: Todo processo eventualmente executa
- ✅ **Baixo overhead**: Sem context switches desnecessários

### Desvantagens

- ❌ **Convoy Effect**: Processos curtos esperam processos longos
- ❌ **Tempo de espera alto**: Processos podem esperar muito
- ❌ **Não responsivo**: Sem garantia de tempo de resposta

## Exemplo de Execução

**Processos:**
- P1: 100 ciclos (chega t=0)
- P2: 50 ciclos (chega t=10)
- P3: 200 ciclos (chega t=20)

**Ordem de execução (1 core):**
```
P1 ────────────> P2 ─────> P3 ────────────────────>
0              100      150                      350
```

**Métricas calculadas:**
| Processo | Chegada | Início | Fim | Espera | Turnaround |
|----------|---------|--------|-----|--------|------------|
| P1 | 0 | 0 | 100 | 0 | 100 |
| P2 | 10 | 100 | 150 | 90 | 140 |
| P3 | 20 | 150 | 350 | 130 | 330 |
| **Média** | | | | **73.3** | **190** |

## Comparação com Outras Políticas

| Métrica | FCFS | Round Robin | SJN |
|---------|------|-------------|-----|
| Tempo médio de espera | Médio | Médio | **Melhor** |
| Responsividade | Ruim | **Boa** | Ruim |
| Overhead | **Mínimo** | Alto | Mínimo |
| Fairness | Médio | **Excelente** | Ruim |

## Quando Usar

✅ **Recomendado para:**
- Sistemas batch (processamento em lote)
- Workloads com processos de tamanho similar
- Quando simplicidade é prioridade

❌ **Não recomendado para:**
- Sistemas interativos
- Quando há processos com tempos muito diferentes
- Quando tempo de resposta é crítico
