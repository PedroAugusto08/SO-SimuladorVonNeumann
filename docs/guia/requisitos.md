# Requisitos do Trabalho

## Informações Gerais

- **Disciplina**: Sistemas Operacionais - CEFET-MG Campus V
- **Professor**: Michel Pires da Silva
- **Valor**: 30 pontos (20 Implementação + 10 Artigo)

## Distribuição de Pontos

| Componente | Pontos | Status |
|------------|--------|--------|
| Escalonamento | 10 | ✅ Completo |
| Gerenciamento de Memória | 10 | ✅ Completo |
| Artigo IEEE | 10 | ⏳ Pendente |
| **Total** | **30** | |

## Requisitos Funcionais

### 1. Arquitetura Multicore (2 pts)

- [x] Suporte a múltiplos núcleos (1-6)
- [x] Cache L1 privada por núcleo
- [x] Memória compartilhada entre núcleos
- [x] Execução paralela com threads

### 2. Escalonamento (8 pts)

- [x] **FCFS** - First Come First Served (não-preemptivo)
- [x] **SJN** - Shortest Job Next (não-preemptivo)
- [x] **Round Robin** - Preemptivo com quantum configurável
- [x] **Priority** - Por prioridade (não-preemptivo e preemptivo)
- [x] Fila de processos thread-safe
- [x] Context switch funcional
- [x] Distribuição de processos entre núcleos

### 3. Gerenciamento de Memória (10 pts)

- [x] Cache L1 por núcleo (128 linhas)
- [x] Memória principal (RAM) compartilhada
- [x] Memória secundária (Disco)
- [x] Política de substituição FIFO
- [x] Política de substituição LRU
- [x] Sincronização de acesso (mutexes)

### 4. Métricas (obrigatório)

- [x] Tempo médio de espera
- [x] Tempo médio de turnaround
- [x] Tempo médio de resposta
- [x] Throughput (processos/segundo)
- [x] Utilização de CPU
- [x] Taxa de cache hit/miss
- [x] Exportação para CSV

### 5. Testes e Validação

- [x] Baseline single-core
- [x] Testes multicore (2, 4, 6 núcleos)
- [x] Comparação entre políticas
- [x] Cálculo de speedup e eficiência

## Requisitos Não-Funcionais

### Código

- [x] C++17 ou superior
- [x] Thread-safe
- [x] Compilável com GCC/G++
- [x] Makefile funcional

### Documentação

- [x] README com instruções
- [x] Documentação técnica
- [ ] Artigo no formato IEEE

## Cenários de Teste Obrigatórios

### Cenário 1: Não-Preemptivo

Processos executam até conclusão sem interrupção.

```bash
./simulador --policy FCFS --cores 2
./simulador --policy SJN --cores 2
```

### Cenário 2: Preemptivo

Processos são interrompidos por quantum.

```bash
./simulador --policy RR --cores 2 --quantum 1000
./simulador --policy PRIORITY --cores 2 --quantum 1000
```

### Cenário 3: Comparativo

Comparar todas as políticas com mesmo workload.

```bash
./test_metrics_complete
./test_multicore_comparative
```

## Métricas de Avaliação

### Speedup

$$S = \frac{T_{1\ core}}{T_{n\ cores}}$$

### Eficiência

$$E = \frac{S}{n} \times 100\%$$

### Throughput

$$Throughput = \frac{n_{processos}}{T_{total}}$$

### Tempo de Turnaround

$$T_{turnaround} = T_{fim} - T_{chegada}$$

### Tempo de Espera

$$T_{espera} = T_{turnaround} - T_{execução}$$
