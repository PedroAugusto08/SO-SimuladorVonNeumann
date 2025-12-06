# Políticas de Cache

## FIFO - First In First Out

### Descrição

Remove a linha que está há mais tempo na cache, independentemente de uso recente.

### Implementação

```cpp
class FIFOCache {
    std::queue<int> insertion_order;  // Ordem de inserção
    
    int select_victim() {
        int oldest = insertion_order.front();
        insertion_order.pop();
        return oldest;
    }
    
    void insert(int line_index, int data) {
        if (cache_full()) {
            int victim = select_victim();
            evict(victim);
        }
        
        lines[line_index] = data;
        insertion_order.push(line_index);
    }
};
```

### Características

| Aspecto | Valor |
|---------|-------|
| Overhead | Baixo |
| Complexidade | O(1) |
| Localidade temporal | Não aproveita |
| Anomalia de Bélády | Pode ocorrer |

## LRU - Least Recently Used

### Descrição

Remove a linha que não é acessada há mais tempo. Aproveita o princípio de localidade temporal.

### Implementação

```cpp
class LRUCache {
    int global_time = 0;
    
    void access(int line_index) {
        lines[line_index].last_access = ++global_time;
    }
    
    int select_victim() {
        int oldest_time = INT_MAX;
        int victim = 0;
        
        for (int i = 0; i < CACHE_LINES; i++) {
            if (lines[i].valid && lines[i].last_access < oldest_time) {
                oldest_time = lines[i].last_access;
                victim = i;
            }
        }
        return victim;
    }
};
```

### Características

| Aspecto | Valor |
|---------|-------|
| Overhead | Médio |
| Complexidade | O(n) busca de vítima |
| Localidade temporal | **Aproveita bem** |
| Hit rate típico | Maior que FIFO |

## Comparação

### Cenário 1: Acesso Sequencial

**Sequência:** 0, 1, 2, 3, 4, 5, 6, 7 (cache com 4 linhas)

| Acesso | FIFO | LRU |
|--------|------|-----|
| 0 | MISS | MISS |
| 1 | MISS | MISS |
| 2 | MISS | MISS |
| 3 | MISS | MISS |
| 4 | MISS (evict 0) | MISS (evict 0) |
| 5 | MISS (evict 1) | MISS (evict 1) |

**Resultado:** Ambos iguais (100% miss para streaming)

### Cenário 2: Acesso com Localidade

**Sequência:** 0, 1, 2, 0, 1, 3, 0, 1, 4 (cache com 3 linhas)

**FIFO:**
```
0 → [0]        MISS
1 → [0,1]      MISS
2 → [0,1,2]    MISS
0 → [0,1,2]    HIT
1 → [0,1,2]    HIT
3 → [1,2,3]    MISS (evict 0 - mais antigo)
0 → [2,3,0]    MISS (evict 1)
1 → [3,0,1]    MISS (evict 2)
4 → [0,1,4]    MISS (evict 3)

Hits: 2, Misses: 7, Rate: 22%
```

**LRU:**
```
0 → [0]        MISS
1 → [0,1]      MISS
2 → [0,1,2]    MISS
0 → [1,2,0]    HIT (atualiza tempo)
1 → [2,0,1]    HIT (atualiza tempo)
3 → [0,1,3]    MISS (evict 2 - menos recente)
0 → [1,3,0]    HIT
1 → [3,0,1]    HIT
4 → [0,1,4]    MISS (evict 3)

Hits: 4, Misses: 5, Rate: 44%
```

**LRU é MELHOR neste cenário!**

### Cenário 3: Working Set

**Programa com loop acessando mesmos dados:**
```cpp
for (int i = 0; i < 1000; i++) {
    access(A);  // Endereço A
    access(B);  // Endereço B
    access(C);  // Endereço C
}
```

| Política | Hit Rate |
|----------|----------|
| FIFO | ~67% |
| LRU | ~99% |

**LRU mantém A, B, C na cache por serem sempre "recentes".**

## Anomalia de Bélády

### Descrição

Fenômeno onde **aumentar o tamanho da cache piora o hit rate** (só ocorre com FIFO).

### Exemplo

**Sequência:** 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5

**Com 3 linhas (FIFO):** 9 page faults
**Com 4 linhas (FIFO):** 10 page faults ← PIOR!

### Por que não afeta LRU?

LRU é uma **política de pilha** (stack algorithm). Em políticas de pilha, o conjunto de páginas na cache com N frames é sempre subconjunto do conjunto com N+1 frames.

## Uso no Simulador

```bash
# Executar com FIFO
./simulador --cache-policy FIFO --cores 2 -p tasks.json process.json

# Executar com LRU
./simulador --cache-policy LRU --cores 2 -p tasks.json process.json

# Comparar métricas
./simulador --policy FCFS --cores 2 --cache-policy FIFO > fifo.log
./simulador --policy FCFS --cores 2 --cache-policy LRU > lru.log
```

## Métricas Coletadas

```cpp
struct CacheStats {
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;
    uint64_t writebacks;
    
    void print() {
        double rate = (double)hits / (hits + misses) * 100;
        printf("Hit Rate: %.2f%%\n", rate);
        printf("Hits: %lu, Misses: %lu\n", hits, misses);
        printf("Evictions: %lu, Writebacks: %lu\n", evictions, writebacks);
    }
};
```

## Recomendações

| Cenário | Política Recomendada |
|---------|---------------------|
| Workload desconhecido | LRU |
| Acesso sequencial (streaming) | FIFO |
| Loops intensivos | LRU |
| Memória limitada para overhead | FIFO |
| Máxima performance | LRU |
