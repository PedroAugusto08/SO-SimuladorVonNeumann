# Sistema de Memória

## Visão Geral

O simulador implementa uma **hierarquia de memória de 3 níveis** com cache privada por núcleo e memória principal compartilhada.

## Hierarquia

```
┌─────────────────────────────────────────────────────┐
│                     CPU (4 cores)                    │
├──────────┬──────────┬──────────┬───────────────────┤
│  Core 0  │  Core 1  │  Core 2  │     Core 3        │
│┌────────┐│┌────────┐│┌────────┐│  ┌────────┐       │
││Cache L1│││Cache L1│││Cache L1││  │Cache L1│       │
││128 lin ││ 128 lin ││ 128 lin ││  │128 lin │       │
│└────────┘│└────────┘│└────────┘│  └────────┘       │
└────┬─────┴────┬─────┴────┬─────┴────────┬──────────┘
     │          │          │              │
     └──────────┴────┬─────┴──────────────┘
                     │
              ┌──────┴──────┐
              │     RAM     │
              │  4096 bytes │
              │   (shared)  │
              └──────┬──────┘
                     │
              ┌──────┴──────┐
              │    Disco    │
              │   16 KB     │
              └─────────────┘
```

## Especificações

| Nível | Tamanho | Latência | Tipo |
|-------|---------|----------|------|
| **Cache L1** | 128 linhas × 4 bytes | 1 ciclo | Por núcleo |
| **RAM** | 4096 bytes | 10 ciclos | Compartilhada |
| **Disco** | 16384 bytes | 100 ciclos | Swap |

## Cache L1

Cada núcleo possui sua própria cache L1:

```cpp
struct CacheLine {
    bool valid;        // Linha válida?
    bool dirty;        // Modificada?
    int tag;           // Tag do endereço
    int data;          // Valor armazenado
    int access_time;   // Para políticas de substituição
};

class Cache {
    static const int CACHE_LINES = 128;
    CacheLine lines[CACHE_LINES];
    CachePolicy policy;  // FIFO ou LRU
};
```

### Políticas de Substituição

| Política | Comportamento |
|----------|---------------|
| **FIFO** | Remove a linha mais antiga |
| **LRU** | Remove a linha menos recentemente usada |

## Processo de Acesso

### Leitura

```
1. Core solicita endereço X
        │
        ▼
2. Verifica Cache L1
        │
   ┌────┴────┐
   │  HIT?   │
   └────┬────┘
        │
   ┌────┴────┬──────────────────┐
   │ SIM     │         NÃO     │
   ▼         │         ▼       │
3a. Retorna  │  3b. Busca na RAM
    dado     │         │
             │    ┌────┴────┐
             │    │ Em RAM? │
             │    └────┬────┘
             │   ┌─────┴─────┐
             │   │ SIM       │ NÃO
             │   ▼           ▼
             │  Carrega    Busca no Disco
             │  na cache   + carrega
             │       │
             └───────┴─────────┐
                               ▼
                4. Retorna dado
```

### Escrita (Write-Back)

```cpp
void write(int address, int value) {
    int index = address % CACHE_LINES;
    
    // Se linha vai ser substituída e está suja
    if (lines[index].valid && lines[index].dirty) {
        // Write-back para RAM
        ram->write(lines[index].tag * CACHE_LINES + index, 
                   lines[index].data);
    }
    
    // Escreve na cache
    lines[index].valid = true;
    lines[index].dirty = true;
    lines[index].tag = address / CACHE_LINES;
    lines[index].data = value;
}
```

## Sincronização Multi-Core

### Problema: Coerência de Cache

```
Core 0 lê endereço 100 → Cache 0: [100: 42]
Core 1 lê endereço 100 → Cache 1: [100: 42]
Core 0 escreve 100 = 99 → Cache 0: [100: 99]
Core 1 lê endereço 100 → Cache 1: [100: 42] ← INCONSISTENTE!
```

### Solução: Mutex em RAM Compartilhada

```cpp
class SharedMemoryManager {
    std::mutex ram_mutex;
    
    int read(int address) {
        std::lock_guard<std::mutex> lock(ram_mutex);
        return ram[address];
    }
    
    void write(int address, int value) {
        std::lock_guard<std::mutex> lock(ram_mutex);
        ram[address] = value;
    }
};
```

## Métricas de Cache

O sistema coleta:

```cpp
struct CacheMetrics {
    int hits;       // Acessos encontrados na cache
    int misses;     // Acessos que falharam
    int evictions;  // Substituições de linha
    
    double hit_rate() {
        return (double)hits / (hits + misses);
    }
};
```

## Uso via CLI

```bash
# Cache com política LRU
./simulador --cache-policy LRU --cores 2

# Cache com política FIFO
./simulador --cache-policy FIFO --cores 2
```

## Exemplo de Execução

**Acessos sequenciais (endereços 0-200):**

```
Endereço: 0   → Cache MISS  → Busca RAM → Cache: [0: valor]
Endereço: 5   → Cache MISS  → Busca RAM → Cache: [5: valor]
Endereço: 0   → Cache HIT   → Retorna direto
Endereço: 128 → Cache MISS  → Evict linha 0 → Cache: [128: valor]
Endereço: 0   → Cache MISS  → Evict linha 128 → Cache: [0: valor]
```

**Métricas resultantes:**
- Hits: 1
- Misses: 4
- Hit Rate: 20%

## Comparação de Políticas

| Cenário | FIFO | LRU |
|---------|------|-----|
| Acesso sequencial | Similar | Similar |
| Acesso com localidade | Ruim | **Melhor** |
| Overhead | Menor | Maior |
| Implementação | Simples | Mais complexa |

## Diagramas de Memória

### Mapeamento de Endereços

```
Endereço: 0x0542 (1346 decimal)
                ├── tag = 1346 / 128 = 10
                └── index = 1346 % 128 = 66

Cache Line 66:
┌───────┬───────┬───────┬───────────────┐
│ valid │ dirty │ tag=10│ data = valor  │
└───────┴───────┴───────┴───────────────┘
```

### Fluxo Completo

```
Instrução: MOV R0, [100]
                │
     ┌──────────┴──────────┐
     │    Memory Manager   │
     │   request(100,READ) │
     └──────────┬──────────┘
                │
     ┌──────────┴──────────┐
     │      Cache L1       │
     │  index = 100%128=100│
     │  tag = 100/128=0    │
     └──────────┬──────────┘
          ┌─────┴─────┐
          │   MISS    │
          └─────┬─────┘
     ┌──────────┴──────────┐
     │        RAM          │
     │    read(100)        │
     │    +10 ciclos       │
     └──────────┬──────────┘
                │
     ┌──────────┴──────────┐
     │   Atualiza Cache    │
     │ lines[100] = valor  │
     └──────────┬──────────┘
                │
     ┌──────────┴──────────┐
     │    Retorna dado     │
     │      R0 = valor     │
     └─────────────────────┘
```
