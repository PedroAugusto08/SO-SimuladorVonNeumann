# 🏆 Achievements - Progresso do Projeto

> **Última atualização:** 13/11/2025  
> **Prazo Final:** 06/12/2025  
> **Tempo Restante:** 23 dias

---

## 🎯 **ESCOPO DO GRUPO (IMPORTANTE!)**

### ✅ **O que ESTE GRUPO está implementando:**

| Responsável | Componente | Pontos | Status |
|-------------|------------|--------|--------|
| **Você (RR)** | Escalonamento Round Robin | 10 | 30% ⏳ |
| **Outro membro** | Gerenciamento de Memória (FIFO/LRU/Segmentação) | 10 | 0% ⭕ |
| **Todos** | Artigo IEEE | 10 | 0% ⭕ |
| **TOTAL** | - | **30** | **10%** |

> **⚠️ ATENÇÃO:** Gerenciamento de Memória vale **10 pontos completos**, não apenas 6!
> 
> **Breakdown Memória (10 pts):**
> - Segmentação Tanenbaum: 2 pts
> - Política FIFO: 3 pts
> - Política LRU: 3 pts
> - Métricas de memória: 2 pts

---

## 📊 Resumo Executivo

| Categoria | Progresso | Itens Completos | Total |
|-----------|-----------|-----------------|-------|
| 🏗️ Arquitetura Multicore | [███████████████░░░░░] 75% | 6/8 | 75% |
| ⚙️ Escalonamento Round Robin | [██████████████░░░░░░] 70% | 7/10 | 70% |
| 🧪 **Processos de Teste JSON** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/3 | 0% |
| 🔄 **Cenários Obrigatórios** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/3 | 0% |
| 💾 Gerenciamento de Memória | [████████████████░░░░] 80% | 4/5 | 80% |
| � **Políticas FIFO/LRU** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/6 | 0% |
| �🔒 Sincronização | [█████████████░░░░░░░] 65% | 3/5 | 65% |
| 📊 Métricas e Testes | [██████░░░░░░░░░░░░░░░] 30% | 2/7 | 30% |
| � **Sistema de Logs** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/4 | 0% |
| 🎯 **Baseline Single-Core** | [░░░░░░░░░░░░░░░░░░░░] 0% ⚠️ | 0/5 | 0% |
| �📄 Artigo IEEE | [░░░░░░░░░░░░░░░░░░░░░░░] 0% | 0/6 | 0% |
| **🎓 TOTAL GERAL** | [████████░░░░░░░░░░░░] 38% | 22/62 | **38%** |

**⚠️ Itens CRÍTICOS adicionados:** 5 etapas obrigatórias identificadas (26 tarefas)!

Nota: barras de progresso substituem imagens externas (progress-bar.dev) por indicadores inline unicode.

---

## 🎯 Etapas de Desenvolvimento

### ✅ ETAPA 1: Estrutura Básica Multicore (3 dias)
**Status:** ✅ **CONCLUÍDA**  
**Data de Conclusão:** 13/11/2025  
**Prazo Recomendado:** 15/11/2025

#### Achievements Completos:

- [x] **Core.hpp criado** ✅ (13/11/2025)
  - Classe `Core` com pipeline MIPS de 5 estágios
  - Execução assíncrona com `std::thread`
  - Cache L1 privada por núcleo
  - Estados: IDLE, BUSY, STOPPING
  - Arquivo: `src/cpu/Core.hpp` (107 linhas)

- [x] **Core.cpp implementado** ✅ (13/11/2025)
  - Construtor e destrutor com gerenciamento de threads
  - Método `execute_async()` para execução paralela
  - Método `wait_completion()` para sincronização
  - Loop de execução do pipeline
  - Arquivo: `src/cpu/Core.cpp` (161 linhas)

- [x] **Múltiplos núcleos funcionando** ✅ (13/11/2025)
  - 2 núcleos configurados (expansível)
  - Cada núcleo com thread independente
  - Cache L1 privada isolada
  - Teste: `./simulador` executa com 2 cores

- [x] **Integração com MemoryManager** ✅ (13/11/2025)
  - RAM compartilhada entre núcleos
  - Acesso sincronizado à memória principal
  - Verificado em `src/cpu/Core.cpp`

- [x] **Compilação bem-sucedida** ✅ (13/11/2025)
  - Makefile atualizado com `Core.cpp` e `RoundRobinScheduler.cpp`
  - Comando: `make simulador`
  - Sistema: WSL/GCC 13/C++17

- [x] **Conflito de nomes resolvido** ✅ (13/11/2025)
  - Função `Core()` renomeada para `CoreExecutionLoop()`
  - Ajustado em: `CONTROL_UNIT.hpp/cpp`, `test_cpu_metrics.cpp`

#### Pendências:

- [ ] ⏳ **Teste de carga com 4+ núcleos**
  - Prazo: 16/11/2025
  - Verificar escalabilidade
  - Criar `test_multicore.cpp`

- [ ] ⏳ **Documentação de API do Core**
  - Prazo: 17/11/2025
  - Adicionar exemplos de uso
  - Criar `docs/API_CORE.md`

---

### 🔄 ETAPA 2: Escalonador Round Robin (4 dias)
**Status:** 🟡 **EM ANDAMENTO** (70% completo)  
**Início:** 13/11/2025  
**Prazo Recomendado:** 19/11/2025

#### Achievements Completos:

- [x] **RoundRobinScheduler.hpp criado** ✅ (13/11/2025)
  - Fila global de prontos (FIFO)
  - Fila de processos bloqueados
  - Vetor de núcleos
  - Estrutura de estatísticas
  - Arquivo: `src/cpu/RoundRobinScheduler.hpp` (68 linhas)

- [x] **RoundRobinScheduler.cpp implementado** ✅ (13/11/2025)
  - Método `add_process()` - adiciona à fila
  - Método `schedule_cycle()` - ciclo de escalonamento
  - Método `assign_process_to_core()` - atribuição
  - Método `collect_finished_processes()` - coleta resultados
  - Arquivo: `src/cpu/RoundRobinScheduler.cpp` (164 linhas)

- [x] **PCB estendido com métricas RR** ✅ (13/11/2025)
  - `arrival_time` - chegada no sistema
  - `start_time` - primeira execução
  - `finish_time` - término
  - `total_wait_time` - tempo em espera
  - `context_switches` - trocas de contexto
  - `assigned_core` / `last_core` - rastreamento de núcleo
  - Arquivo: `src/cpu/PCB.hpp`

- [x] **Detecção de migração entre núcleos** ✅ (13/11/2025)
  - Implementado em `assign_process_to_core()`
  - Incrementa `context_switches` quando troca de núcleo

- [x] **Fila global de prontos (FIFO)** ✅ (13/11/2025)
  - `std::deque<PCB*> ready_queue`
  - Ordem: primeiro a chegar, primeiro a executar

- [x] **Tratamento de processos bloqueados** ✅ (13/11/2025)
  - `std::deque<PCB*> blocked_queue`
  - Método `handle_blocked_processes()`

- [x] **Quantum configurável** ✅ (13/11/2025)
  - Padrão: 100 ciclos
  - Configurável via construtor

#### Pendências:

- [ ] ⏳ **Integração completa com main.cpp**
  - Prazo: 16/11/2025 ⚠️
  - Atualmente o escalonador existe mas não está sendo usado no loop principal
  - Substituir lógica atual por `RoundRobinScheduler`
  - **CRÍTICO:** Necessário para testes de múltiplos processos

- [ ] ⏳ **Implementar preempção real por quantum**
  - Prazo: 17/11/2025
  - Interromper execução quando quantum expira
  - Atualmente depende de processo terminar ou bloquear

- [ ] ⏳ **Métricas agregadas funcionando**
  - Prazo: 18/11/2025
  - `get_statistics()` retorna valores calculados
  - Tempo médio de espera/turnaround
  - Taxa de throughput

---

### � ETAPA 2.5: Cenários de Execução (3 dias) ⚠️⚠️
**Status:** ⭕ **NÃO INICIADA** (0%) - **CRÍTICO!**  
**Prazo Recomendado:** 17/11/2025  
**Pontos em Jogo:** 4 pontos (2 + 2)

> **⚠️ OBRIGATÓRIO:** Especificação exige 2 cenários distintos!

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️ **Cenário 1: Round Robin Não-Preemptivo** (2 pts)
  - Prazo: 17/11/2025 ⚠️
  - Especificação: "Cenário não preemptivo: todas as tarefas são executadas até a conclusão, sem interrupções"
  - **O que fazer:**
    - Adicionar flag `--non-preemptive` ao simulador
    - Round Robin SEM quantum (processos até terminar)
    - Ordem FIFO mantida, mas sem interrupção
  - **Arquivos:** `src/main.cpp`, `RoundRobinScheduler.cpp`
  - **Teste:** Rodar 5 processos e verificar que não há preempção

- [ ] ⏳ **Cenário 2: Round Robin Preemptivo** (2 pts)
  - Prazo: 19/11/2025
  - Especificação: "Cenário preemptivo: todas as tarefas são passíveis de interrupção"
  - **O que fazer:**
    - Garantir quantum funciona (já implementado parcialmente)
    - Context switch preserva estado completo
    - Processo interrompido volta ao fim da fila
  - **Arquivos:** `src/cpu/Core.cpp`, `RoundRobinScheduler.cpp`
  - **Teste:** Verificar que processos são interrompidos no quantum

- [ ] ⚠️ **Comparação entre cenários**
  - Prazo: 20/11/2025
  - Executar mesmos processos em ambos cenários
  - Coletar métricas de cada um
  - Incluir comparação no artigo

---

### 🧪 ETAPA 2.6: Processos de Teste e Carga Inicial (2 dias) ⚠️⚠️
**Status:** ⭕ **NÃO INICIADA** (0%) - **BLOQUEADOR!**  
**Prazo Recomendado:** 15/11/2025  
**Importância:** CRÍTICA (sem isso, nada funciona!)

> **⚠️ BLOQUEADOR:** Sem processos JSON, não há como testar Round Robin!

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️⚠️ **Criar 5+ processos JSON variados**
  - Prazo: 15/11/2025 (2 DIAS!) ⚠️
  - **BLOQUEIO:** Sem isso, impossível testar escalonamento!
  - **O que criar:**
    - `processo1.json` - CPU-bound (muitas operações ALU, poucas memórias)
    - `processo2.json` - I/O-bound (muitos acessos à memória)
    - `processo3.json` - Balanceado (mix de CPU e memória)
    - `processo4.json` - Curto (100-200 instruções)
    - `processo5.json` - Longo (1000+ instruções)
  - **Pasta:** `src/tasks/`
  - **Formato:** Copiar estrutura de `tasks.json` existente

- [ ] ⚠️⚠️ **Implementar carga inicial completa**
  - Prazo: 16/11/2025 ⚠️
  - Especificação: "todos os programas pertencentes ao lote devem ser completamente carregados na memória principal ANTES do início da execução"
  - **O que fazer:**
    ```cpp
    // Em main.cpp
    vector<string> process_files = {
        "processo1.json", "processo2.json", 
        "processo3.json", "processo4.json", "processo5.json"
    };
    
    // PASSO 1: Carregar TODOS antes
    for (auto& file : process_files) {
        PCB* pcb = load_process_from_json(file);
        scheduler.add_process(pcb);
    }
    
    // PASSO 2: SÓ DEPOIS executar
    scheduler.run_until_completion();
    ```
  - **Arquivos:** `src/main.cpp`
  - **Teste:** Verificar que todos 5 processos são carregados antes da execução

- [ ] ⏳ **Documentar formato JSON**
  - Prazo: 17/11/2025
  - Criar `docs/FORMATO_JSON.md`
  - Explicar estrutura de processos
  - Exemplos de cada tipo

---

### �💾 ETAPA 3: Gerenciamento de Memória (4 dias)
**Status:** ✅ **QUASE COMPLETA** (80%)  
**Prazo Recomendado:** 23/11/2025

#### Achievements Completos:

- [x] **MemoryManager compartilhado** ✅ (Antes de 13/11)
  - RAM única para todos os núcleos
  - Arquivo: `src/memory/MemoryManager.hpp/cpp`

- [x] **Cache L1 privada por núcleo** ✅ (13/11/2025)
  - Cada núcleo tem `std::unique_ptr<Cache> L1_cache`
  - Implementado em `Core::Core()`
  - Arquivo: `src/cpu/Core.cpp` linha 18

- [x] **Contabilização de cache hits/misses** ✅ (13/11/2025)
  - `PCB::cache_hits` e `PCB::cache_misses`
  - Incrementados durante acesso à memória
  - Arquivo: `src/cpu/PCB.hpp`

- [x] **Sincronização de acesso à RAM** ✅ (Antes de 13/11)
  - `std::mutex` em `MemoryManager`
  - Acesso thread-safe

#### Pendências:

- [ ] ⏳ **Teste de contenção de memória**
  - Prazo: 20/11/2025
  - Múltiplos núcleos acessando RAM simultaneamente
  - Medir impacto de locks

---


### 💾 ETAPA 3.4: Segmentação Tanenbaum e Métricas de Memória (3 dias) ⚠️
**Status:** ⭕ **NÃO INICIADA** (0%) - **CRÍTICO!**  
**Prazo Recomendado:** 20/11/2025  
**Pontos em Jogo:** 4 pontos (2 + 2)  
**Responsável:** OUTRO MEMBRO DO GRUPO 🔴

> **⚠️ OBRIGATÓRIO:** Especificação exige:
> - "mapeamento de endereços segundo um modelo inspirado em Tanenbaum"
> - "A utilização de memória ao longo do tempo"

#### Contexto e Objetivo:

**O que é segmentação de memória (modelo Tanenbaum)?**
- Memória acessada por palavras de X bits
- **Parte dos bits** = endereço do bloco/segmento
- **Parte dos bits** = deslocamento interno (offset)
- Exemplo: palavra de 16 bits = 8 bits (bloco) + 8 bits (offset)

**Por que é importante?**
- Permite endereçamento virtual
- Facilita gerenciamento de cache  
- Base para políticas de substituição FIFO/LRU

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️ **Implementar segmentação de endereços Tanenbaum** (2 pts)
  - Prazo: 18/11/2025 ⚠️
  - Especificação: "parte dos bits representa o endereço do bloco e parte o deslocamento interno"
  - **O que fazer:**
    ```cpp
    class MemoryAddressing {
        static constexpr int WORD_BITS = 16;
        static constexpr int BLOCK_BITS = 8;    // 256 blocos
        static constexpr int OFFSET_BITS = 8;   // 256 bytes/bloco
    public:
        struct Address {
            uint16_t block;
            uint16_t offset;
        };
        
        static Address to_segmented(uint32_t linear) {
            Address addr;
            addr.block = (linear >> OFFSET_BITS) & ((1 << BLOCK_BITS) - 1);
            addr.offset = linear & ((1 << OFFSET_BITS) - 1);
            return addr;
        }
        
        static uint32_t to_linear(Address addr) {
            return (addr.block << OFFSET_BITS) | addr.offset;
        }
    };
    ```
  - **Arquivos:** Criar `src/memory/MemoryAddressing.hpp`, `.cpp`
  - **Integração:** Modificar `MemoryManager::access()` para usar segmentação
  - **Teste:** Endereço 300 = Bloco 1 + Offset 44

- [ ] ⚠️⚠️ **Coletar métricas de utilização de memória** (2 pts)
  - Prazo: 20/11/2025 ⚠️
  - Especificação: "A utilização de memória ao longo do tempo"
  - **O que fazer:**
    ```cpp
    class MemoryMetrics {
        struct Snapshot {
            uint64_t timestamp;
            size_t bytes_used;
            double utilization_percent;
            int num_processes;
        };
        std::vector<Snapshot> history;
    public:
        void take_snapshot(uint64_t cycle, size_t used, int procs);
        void save_to_csv(const std::string& filename);
    };
    ```
  - **Saída:** `logs/memory_utilization.csv`
  - **Gráfico:** Para seção "Resultados" do artigo
  - **Arquivos:** Criar `src/memory/MemoryMetrics.hpp`, `.cpp`

- [ ] ⏳ **Integrar snapshots no loop principal**
  - Prazo: 20/11/2025
  - No `main.cpp`: chamar `mem_metrics.take_snapshot()` a cada 1000 ciclos
  - Salvar CSV no final da execução

- [ ] ⏳ **Criar gráfico Python**
  - Prazo: 21/11/2025
  - Script `scripts/plot_memory_utilization.py`
  - Usar no artigo (seção Resultados)

#### Divisão de Tarefas:
- Dia 1 (18/11): Implementar `MemoryAddressing` + testes
- Dia 2 (19/11): Implementar `MemoryMetrics` + integração
- Dia 3 (20/11): Gerar CSV, gráfico, validar

---

### � ETAPA 3.5: Políticas de Substituição FIFO e LRU (5 dias) ⚠️
**Status:** ⭕ **NÃO INICIADA** (0%) - **CRÍTICO!**  
**Prazo Recomendado:** 23/11/2025  
**Pontos em Jogo:** 6 pontos (3 + 3)  
**Responsável:** OUTRO MEMBRO DO GRUPO 🔴

> **⚠️ OBRIGATÓRIO:** Especificação exige "políticas de substituição (ex.: FIFO, LRU)"!  
> **📌 ATENÇÃO:** Esta etapa é de OUTRO membro, mas todos devem conhecer!

#### Contexto e Objetivo:

**O que são políticas de substituição?**
- Quando a cache está CHEIA e precisa buscar novo dado
- Precisa REMOVER algum bloco antigo (substituição)
- FIFO = remove o mais antigo (First In, First Out)
- LRU = remove o menos recentemente usado (Least Recently Used)

**Onde implementar?**
- Arquivos: `src/memory/CachePolicy.hpp/cpp` OU `src/memory/cache.cpp`
- Integração: `Cache::access()` chama política quando cache full

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️ **Implementar política FIFO** (3 pts)
  - Prazo: 21/11/2025 ⚠️
  - Especificação: "política de substituição FIFO"
  - **O que fazer:**
    ```cpp
    // Criar classe FIFOPolicy
    class FIFOPolicy : public ReplacementPolicy {
        std::queue<int> insertion_order; // Ordem de inserção
    public:
        int select_victim() override {
            int victim = insertion_order.front();
            insertion_order.pop();
            return victim; // Remove o mais antigo
        }
        
        void on_access(int block_id) override {
            insertion_order.push(block_id);
        }
    };
    ```
  - **Arquivos:** Criar `src/memory/FIFOPolicy.hpp`, `FIFOPolicy.cpp`
  - **Teste:** Cache com 4 blocos, inserir 5º e verificar que 1º foi removido

- [ ] ⚠️⚠️ **Implementar política LRU** (3 pts)
  - Prazo: 23/11/2025 ⚠️
  - Especificação: "política de substituição LRU"
  - **O que fazer:**
    ```cpp
    // Criar classe LRUPolicy
    class LRUPolicy : public ReplacementPolicy {
        std::map<int, uint64_t> last_access_time; // Timestamp de acesso
        uint64_t current_time = 0;
    public:
        int select_victim() override {
            // Encontra bloco com menor timestamp
            int victim = -1;
            uint64_t oldest_time = UINT64_MAX;
            for (auto& [block, time] : last_access_time) {
                if (time < oldest_time) {
                    oldest_time = time;
                    victim = block;
                }
            }
            return victim; // Remove menos usado
        }
        
        void on_access(int block_id) override {
            last_access_time[block_id] = current_time++;
        }
    };
    ```
  - **Arquivos:** Criar `src/memory/LRUPolicy.hpp`, `LRUPolicy.cpp`
  - **Teste:** Acessar blocos [1,2,3,4,1], cache cheio, inserir 5 → remove bloco 2

- [ ] ⚠️ **Integrar políticas ao Cache**
  - Prazo: 23/11/2025 ⚠️
  - Modificar `Cache` para aceitar política:
    ```cpp
    // Em cache.hpp
    class Cache {
        std::unique_ptr<ReplacementPolicy> policy;
    public:
        Cache(CachePolicyType type) {
            if (type == CachePolicyType::FIFO)
                policy = std::make_unique<FIFOPolicy>();
            else if (type == CachePolicyType::LRU)
                policy = std::make_unique<LRUPolicy>();
        }
        
        void access(int address) {
            if (is_full() && !is_hit(address)) {
                int victim = policy->select_victim();
                evict(victim);
            }
            // ... resto do código
            policy->on_access(address);
        }
    };
    ```
  - **Arquivos:** `src/memory/cache.hpp`, `cache.cpp`

- [ ] ⏳ **Adicionar flag de escolha de política**
  - Prazo: 24/11/2025
  - Permitir rodar simulador com diferentes políticas:
    ```bash
    ./simulador --cache-policy=FIFO
    ./simulador --cache-policy=LRU
    ```
  - Modificar `main.cpp` para aceitar argumento
  - **Arquivos:** `src/main.cpp`

- [ ] ⏳ **Testar comparação FIFO vs LRU**
  - Prazo: 25/11/2025
  - Rodar mesmos 5 processos com FIFO
  - Rodar mesmos 5 processos com LRU
  - Comparar:
    - Cache hit rate (%)
    - Tempo total de execução
    - Número de misses
  - Salvar resultados:
    - `logs/metrics_FIFO.csv`
    - `logs/metrics_LRU.csv`

- [ ] ⏳ **Documentar políticas**
  - Prazo: 26/11/2025
  - Criar `docs/POLITICAS_CACHE.md`
  - Explicar FIFO vs LRU
  - Mostrar exemplos de uso
  - Incluir gráficos comparativos no artigo

#### Recursos para Implementação:

**Referências Teóricas:**
- Tanenbaum, "Modern Operating Systems", Capítulo 3.4 (Page Replacement)
- Silberschatz, "Operating Systems Concepts", Capítulo 10 (Virtual Memory)

**Exemplos de Código:**
- Ver `src/memory/cachePolicy.cpp` (já existe estrutura básica)
- Ver `src/memory/cache.cpp` (classe Cache atual)

**Testes Sugeridos:**
```cpp
// Teste FIFO
Cache cache_fifo(4, CachePolicyType::FIFO); // 4 blocos
cache_fifo.access(1); // Miss
cache_fifo.access(2); // Miss
cache_fifo.access(3); // Miss
cache_fifo.access(4); // Miss
cache_fifo.access(5); // Miss → remove bloco 1 (mais antigo)
assert(!cache_fifo.contains(1)); // Bloco 1 foi removido

// Teste LRU
Cache cache_lru(4, CachePolicyType::LRU);
cache_lru.access(1); // Miss
cache_lru.access(2); // Miss
cache_lru.access(3); // Miss
cache_lru.access(4); // Miss
cache_lru.access(1); // Hit (atualiza timestamp)
cache_lru.access(5); // Miss → remove bloco 2 (menos usado)
assert(!cache_lru.contains(2)); // Bloco 2 foi removido
assert(cache_lru.contains(1));  // Bloco 1 ainda está (foi usado recentemente)
```

#### Divisão de Tarefas (Sugerida para responsável):

**Dia 1-2 (19-20 Nov):**
- [ ] Estudar políticas FIFO e LRU (teoria)
- [ ] Analisar código existente em `cachePolicy.cpp`
- [ ] Criar classes FIFOPolicy e LRUPolicy

**Dia 3 (21 Nov):**
- [ ] Implementar e testar FIFO
- [ ] Criar testes unitários

**Dia 4 (22 Nov):**
- [ ] Implementar e testar LRU
- [ ] Criar testes unitários

**Dia 5 (23 Nov):**
- [ ] Integrar ao Cache principal
- [ ] Testar com processos completos
- [ ] Comparar resultados FIFO vs LRU

---

### �🔒 ETAPA 4: Sincronização e Concorrência (3 dias)
**Status:** 🟡 **PARCIAL** (65%)  
**Prazo Recomendado:** 26/11/2025

#### Achievements Completos:

- [x] **Mutexes na fila de prontos** ✅ (13/11/2025)
  - `std::mutex scheduler_mutex` em RoundRobinScheduler
  - `std::lock_guard` em métodos críticos

- [x] **Sincronização de threads Core** ✅ (13/11/2025)
  - `std::thread execution_thread`
  - `wait_completion()` com `join()`

- [x] **Atomic operations no PCB** ✅ (13/11/2025)
  - Todos os contadores são `std::atomic<uint64_t>`
  - Thread-safe sem locks

#### Pendências:

- [ ] ⏳ **Teste de race conditions**
  - Prazo: 22/11/2025
  - Usar ThreadSanitizer (`-fsanitize=thread`)
  - Verificar deadlocks

- [ ] ⏳ **Documentar estratégia de locks**
  - Prazo: 24/11/2025
  - Criar `docs/SINCRONIZACAO.md`
  - Explicar ordem de aquisição de mutexes

---

### 📊 ETAPA 5: Métricas e Validação (5 dias)
**Status:** ⭕ **NÃO INICIADA** (30%)  
**Prazo Recomendado:** 01/12/2025 ⚠️

#### Achievements Completos:

- [x] **Estrutura de métricas no PCB** ✅ (13/11/2025)
  - Campos: `arrival_time`, `start_time`, `finish_time`, etc.
  - Métodos: `get_turnaround_time()`, `get_wait_time()`

- [x] **Estrutura Statistics no Scheduler** ✅ (13/11/2025)
  - `avg_wait_time`, `avg_turnaround_time`, etc.
  - Método `get_statistics()`

#### Pendências (CRÍTICAS):

- [ ] ⏳ **Implementar coleta de métricas em arquivo**
  - Prazo: 20/11/2025
  - Salvar em `logs/metrics.csv` ou `.json`
  - Colunas: PID, tempo_espera, turnaround, núcleo, etc.

- [ ] ⏳ **Calcular Speedup**
  - Prazo: 24/11/2025
  - Fórmula: $Speedup = \frac{T_{single}}{T_{multi}}$
  - Gerar gráfico de speedup vs. número de núcleos

- [ ] ⏳ **Criar gráficos de utilização de CPU**
  - Prazo: 26/11/2025
  - Usar Python/matplotlib ou gnuplot
  - Gráfico: tempo x utilização por núcleo

---

### 📝 ETAPA 5.5: Sistema de Logs e Rastreamento (2 dias) ⚠️
**Status:** ⭕ **NÃO INICIADA** (0%) - **CRÍTICO!**  
**Prazo Recomendado:** 20/11/2025  
**Pontos em Jogo:** 1 ponto

> **⚠️ OBRIGATÓRIO:** Especificação exige "LOG DETALHADO das execuções"!

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️ **Implementar arquivo de log principal** (1 pt)
  - Prazo: 18/11/2025 ⚠️
  - Especificação: "LOG DETALHADO registrando o que ocorreu em cada instante"
  - **O que fazer:**
    ```cpp
    // Criar classe Logger
    class ExecutionLogger {
        std::ofstream log_file;
        std::mutex log_mutex;
    public:
        void log_event(int core_id, int pid, string event, uint64_t timestamp);
    };
    
    // Eventos a registrar:
    // - Processo carregado na memória
    // - Processo iniciou execução (core X)
    // - Context switch (de PID X para Y)
    // - Processo terminou
    // - Acesso à memória (cache hit/miss)
    ```
  - **Arquivos:** Criar `src/logger/ExecutionLogger.hpp`, `ExecutionLogger.cpp`
  - **Saída:** `logs/execution_YYYYMMDD_HHMMSS.log`

- [ ] ⚠️ **Integrar log em todos pontos críticos**
  - Prazo: 19/11/2025 ⚠️
  - Core::execute_process() → log início/fim
  - RoundRobinScheduler::schedule() → log alocação
  - RoundRobinScheduler::context_switch() → log troca
  - MemoryManager::access() → log cache hit/miss
  - **Arquivos:** `Core.cpp`, `RoundRobinScheduler.cpp`, `MemoryManager.cpp`

- [ ] ⏳ **Criar arquivo de métricas CSV**
  - Prazo: 20/11/2025
  - Formato: `PID,arrival,start,finish,wait,turnaround,core_id`
  - **Saída:** `logs/metrics.csv`
  - Facilita importação em Python/Excel para gráficos

- [ ] ⏳ **Documentar formato dos logs**
  - Prazo: 21/11/2025
  - Criar `docs/FORMATO_LOGS.md`
  - Exemplo de cada tipo de evento

---

### 🎯 ETAPA 5.6: Baseline e Comparação Single-Core (3 dias) ⚠️⚠️
**Status:** ⭕ **NÃO INICIADA** (0%) - **CRÍTICO!**  
**Prazo Recomendado:** 26/11/2025  
**Pontos em Jogo:** 2 pontos

> **⚠️ OBRIGATÓRIO:** Especificação exige "Comparação com baseline single-core"!

#### Pendências CRÍTICAS:

- [ ] ⚠️⚠️ **Implementar modo single-core** (2 pts)
  - Prazo: 24/11/2025 ⚠️
  - Especificação: "comparação com baseline (execução single-core)"
  - **O que fazer:**
    ```cpp
    // Adicionar flag ao simulador
    // main.cpp:
    bool single_core_mode = false;
    if (argc > 1 && strcmp(argv[1], "--single-core") == 0) {
        single_core_mode = true;
    }
    
    // Criar apenas 1 core ao invés de N
    int num_cores = single_core_mode ? 1 : 2;
    ```
  - **Arquivos:** `src/main.cpp`
  - **Teste:** Rodar com `./simulador --single-core`

- [ ] ⚠️ **Executar bateria de testes comparativa**
  - Prazo: 25/11/2025 ⚠️
  - Rodar mesmos 5 processos em:
    - 1 core (baseline)
    - 2 cores (atual)
    - 4 cores (se implementado)
  - Salvar resultados separados:
    - `logs/metrics_1core.csv`
    - `logs/metrics_2cores.csv`
    - `logs/metrics_4cores.csv`

- [ ] ⚠️ **Calcular Speedup e Efficiency**
  - Prazo: 26/11/2025 ⚠️
  - Fórmulas:
    - $Speedup = \frac{T_{1core}}{T_{Ncores}}$
    - $Efficiency = \frac{Speedup}{N} \times 100\%$
  - Criar tabela comparativa para o artigo
  - **Script:** Criar `scripts/calculate_speedup.py`

- [ ] ⏳ **Gerar gráficos comparativos**
  - Prazo: 26/11/2025
  - Gráfico 1: Tempo total (1 vs 2 vs 4 cores)
  - Gráfico 2: Speedup por número de cores
  - Gráfico 3: Utilização de CPU por core
  - **Script:** `scripts/plot_comparison.py`

- [ ] ⏳ **Documentar resultados**
  - Prazo: 27/11/2025
  - Criar `docs/RESULTADOS_BASELINE.md`
  - Incluir tabelas e gráficos
  - Análise: Round Robin escala bem? Por quê?

---

### 📄 ETAPA 6: Artigo IEEE (5 dias)
**Status:** ⭕ **NÃO INICIADA** (0%)  
**Prazo Recomendado:** 06/12/2025 (DATA LIMITE) ⚠️⚠️

#### Template e Estrutura:

- [ ] ⏳ **Baixar template IEEE LaTeX**
  - Prazo: 27/11/2025
  - Usar modelo de conferência IEEE
  - Link: https://www.ieee.org/conferences/publishing/templates.html

- [ ] ⏳ **Escrever Abstract**
  - Prazo: 28/11/2025
  - 150-200 palavras
  - Resumir: problema, solução, resultados

- [ ] ⏳ **Escrever Introdução**
  - Prazo: 29/11/2025
  - Contexto: Arquitetura Von Neumann
  - Problema: Bottleneck, necessidade de multicore
  - Solução: Simulador com Round Robin

- [ ] ⏳ **Escrever Metodologia**
  - Prazo: 01/12/2025
  - Descrever arquitetura do simulador
  - Algoritmo Round Robin
  - Estrutura de dados

- [ ] ⏳ **Escrever Resultados e Discussão**
  - Prazo: 03/12/2025
  - Apresentar métricas coletadas
  - Gráficos de speedup, utilização, tempo de espera
  - Comparar com baseline

- [ ] ⏳ **Escrever Conclusão e Referências**
  - Prazo: 04/12/2025
  - Resumir contribuições
  - Trabalhos futuros
  - Mínimo 10 referências bibliográficas

---

## 📅 Cronograma Visual (ATUALIZADO)

```
Novembro 2025          |  Dezembro 2025
-----------------------+----------------
13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 01 02 03 04 05 06
✅ ⏳ ⚠️ ⚠️ ⚠️ ⚠️ ⚠️ ⚠️ ⚠️ ⚠️ 🟡 ⚠️ ⚠️ ⚠️ 📄 📄 📄 📄 📄 📄 📄 📄 📄 🏁
│  │  │  │  │  │  │  │  │     │     │     │     │     │        │
│  │  │  │  │  │  │  │  │     │     │     │     │     │        └─ ENTREGA FINAL
│  │  │  │  │  │  │  │  │     │     │     │     │     └────────── Revisão Artigo
│  │  │  │  │  │  │  │  │     │     │     │     └───────────────── Resultados Artigo
│  │  │  │  │  │  │  │  │     │     │     └─────────────────────── Gráficos
│  │  │  │  │  │  │  │  │     │     └───────────────────────────── Baseline 1-core ⚠️
│  │  │  │  │  │  │  │  │     └─────────────────────────────────── FIFO/LRU (outro membro) 🟡
│  │  │  │  │  │  │  │  └───────────────────────────────────────── Testes Sincronização
│  │  │  │  │  │  │  └──────────────────────────────────────────── Logs Integrados ⚠️
│  │  │  │  │  │  └─────────────────────────────────────────────── Comparação Cenários
│  │  │  │  │  └────────────────────────────────────────────────── Cenário Preemptivo ⏳
│  │  │  │  └───────────────────────────────────────────────────── Cenário Não-Preemptivo ⚠️⚠️
│  │  │  └──────────────────────────────────────────────────────── Carga Inicial Completa
│  │  └─────────────────────────────────────────────────────────── 5+ JSONs Processos ⚠️⚠️⚠️
│  └────────────────────────────────────────────────────────────── Integração Core+RR
└───────────────────────────────────────────────────────────────── Hoje ✅ (13 Nov)

📊 NOVA ESTRUTURA DE ETAPAS:
┌──────────────────────────────────────────────────────────────┐
│ ETAPA 1: ✅ Multicore (75%) - 13/11                          │
│ ETAPA 2: 🟡 Round Robin (70%) - 14/11                        │
│ ETAPA 2.5: ⚠️ Cenários Obrigatórios (0%) - 17-19/11 [4 pts] │
│ ETAPA 2.6: ⚠️⚠️ Processos JSON (0%) - 15-16/11 [BLOQUEIO]   │
│ ETAPA 3: 🟡 Memória (80%) - 16-20/11                         │
│ ETAPA 3.4: 🔴 Segmentação+Métricas (0%) - 18-20/11 [4 pts]  │
│ ETAPA 3.5: 🔴 FIFO/LRU (0%) - 19-23/11 [6 pts] (outro mbr)  │
│ ETAPA 4: 🟡 Sincronização (65%) - 21-24/11                   │
│ ETAPA 5: ⏳ Métricas (30%) - 22-26/11                        │
│ ETAPA 5.5: ⚠️ Logs (0%) - 18-20/11 [1 pt]                   │
│ ETAPA 5.6: ⚠️⚠️ Baseline (0%) - 24-26/11 [2 pts]            │
│ ETAPA 6: ⭕ Artigo IEEE (0%) - 27/11-06/12 [10 pts]          │
└──────────────────────────────────────────────────────────────┘

⚠️⚠️⚠️ DEADLINES CRÍTICOS (próximos 7 dias):
• 15 Nov (SEX): 5+ processos JSON criados [BLOQUEADOR!]
• 16 Nov (SÁB): Carga inicial batch implementada
• 17 Nov (DOM): Cenário não-preemptivo funcionando [2 pts]
• 18-20 Nov: Segmentação Tanenbaum + Métricas memória [4 pts]
• 19-23 Nov: FIFO/LRU (outro membro) [6 pts]
```

---

## ⚠️ **ITENS CRÍTICOS FALTANDO (PODEM PERDER PONTOS)**

> **📌 REFERÊNCIA:** Veja detalhes completos de implementação nas **ETAPAS 2.5, 2.6, 5.5 e 5.6** acima!

### 🔴 **URGENTE - Esta Semana (14-17 Nov):**

#### 1. **5+ Processos JSON** [BLOQUEADOR!!!] ⚠️⚠️⚠️
   - **Especificação:** Criar "carga inicial com processos para testar"
   - **📂 VER:** ETAPA 2.6 (linha ~160)
   - **O que fazer:**
     - Criar `processo1.json` (CPU-bound)
     - Criar `processo2.json` (I/O-bound)  
     - Criar `processo3.json` (balanceado)
     - Criar `processo4.json` (curto, 100-200 inst)
     - Criar `processo5.json` (longo, 1000+ inst)
   - **Prazo:** 15/11/2025 (AMANHÃ!)
   - **Risco:** Sem isso, NADA pode ser testado!

#### 2. **Cenário Não-Preemptivo** (-2 pts se não fizer) ⚠️⚠️
   - **Especificação:** "Cenário não preemptivo: todas as tarefas são executadas até a conclusão, sem interrupções"
   - **📂 VER:** ETAPA 2.5 (linha ~80)
   - **O que fazer:**
     - Criar flag `--non-preemptive` ou similar
     - Round Robin SEM quantum
     - Processos executam até terminar
   - **Prazo:** 17/11/2025
   - **Arquivo:** `src/main.cpp` + `RoundRobinScheduler`

#### 2. **Criar 5+ Processos JSON** (bloqueador para testes) ⚠️⚠️
   - **Especificação:** "Ler do disco um lote inicial de programas previamente definido"
   - **O que fazer:**
     - `processo1.json` - CPU-bound
     - `processo2.json` - I/O-bound
     - Criar `processo5.json` (longo, 1000+ inst)
   - **Prazo:** 15/11/2025 (AMANHÃ!)
   - **Risco:** Sem isso, NADA pode ser testado!
   - **📂 VER:** ETAPA 2.6 (detalhes completos)

#### 2. **Cenário Não-Preemptivo** (-2 pts se não fizer) ⚠️⚠️
   - **Especificação:** "Cenário não preemptivo: todas as tarefas são executadas até a conclusão, sem interrupções"
   - **📂 VER:** ETAPA 2.5 (linha ~80)
   - **O que fazer:**
     - Adicionar flag `--non-preemptive` ao simulador
     - Round Robin SEM quantum (processos até terminar)
     - Ordem FIFO mantida, mas sem interrupção
   - **Prazo:** 17/11/2025
   - **📂 VER:** ETAPA 2.5 (detalhes completos)

#### 3. **Carga Inicial Completa** (requisito obrigatório) ⚠️
   - **Especificação:** "todos os programas pertencentes ao lote devem ser completamente carregados na memória principal ANTES do início da execução"
   - **📂 VER:** ETAPA 2.6 (linha ~160)
   - **O que fazer:**
     ```cpp
     // Carregar TODOS antes
     for (auto& file : process_files) {
         scheduler.add_process(load(file));
     }
     // SÓ DEPOIS executar
     scheduler.run();
     ```
   - **Prazo:** 16/11/2025
   - **Arquivo:** `src/main.cpp`

### 🟡 **IMPORTANTE - Semana 18-26 Nov:**

#### 4. **Arquivos de Log** (-1 pt se não fizer) ⚠️
   - **Especificação:** "todas as execuções devem gerar arquivos de log contendo métricas"
   - **📂 VER:** ETAPA 5.5 (linha ~370)
   - **O que fazer:**
     - Criar classe `ExecutionLogger`
     - Log de cada evento (início, fim, context switch, cache hit/miss)
     - Salvar em `logs/execution_{timestamp}.log`
     - Gerar também `metrics.csv` para análise
   - **Prazo:** 20/11/2025
   - **📂 VER:** ETAPA 5.5 (detalhes completos)

#### 5. **Baseline Single-Core** (-2 pts se não fizer) ⚠️⚠️
   - **Especificação:** "deve-se utilizar como baseline a arquitetura single-core previamente desenvolvida"
   - **📂 VER:** ETAPA 5.6 (linha ~415)
   - **O que fazer:**
     - Adicionar flag `--single-core`
     - Rodar mesmos 5 processos em 1 core
     - Comparar com 2 e 4 núcleos
     - Calcular Speedup e Efficiency
   - **Prazo:** 24-26/11/2025
   - **Necessário para artigo!**
   - **📂 VER:** ETAPA 5.6 (detalhes completos)

#### 6. **Políticas FIFO e LRU** (-6 pts se não fizer) 🔴🔴
   - **Especificação:** "políticas de substituição de cache (ex.: FIFO, LRU)"
   - **📂 VER:** ETAPA 3.5 (linha ~290)
   - **Responsável:** OUTRO MEMBRO DO GRUPO!
   - **O que fazer:**
     - Implementar `FIFOPolicy` (remove bloco mais antigo)
     - Implementar `LRUPolicy` (remove bloco menos usado)
     - Integrar ao `Cache::access()`
     - Testar comparação FIFO vs LRU
   - **Prazo:** 21-23/11/2025
   - **⚠️ ATENÇÃO:** Sem isso, perde 6 pontos (20% da nota)!
   - **📂 VER:** ETAPA 3.5 (guia completo com código, testes e divisão de tarefas)

---

## 🎯 Próximos 5 Dias Críticos

### 📌 Hoje - 13/11/2025 (Quarta)
- [x] ✅ Compilar e validar `Core` + `RoundRobinScheduler`
- [x] ✅ Verificar execução básica do simulador

### 📌 14/11/2025 (Quinta) - **DIA CRUCIAL**
- [ ] ⚠️ **Integrar RoundRobinScheduler ao main.cpp**
  - Modificar loop principal
  - Usar `scheduler.schedule_cycle()`
  - Remover lógica antiga de execução única

### 📌 15/11/2025 (Sexta)
- [ ] ⚠️ **Criar 5 processos de teste (JSON)**
  - `processo1.json` - CPU-bound (muitas operações ALU)
  - `processo2.json` - I/O-bound (muitos acessos memória)
  - `processo3.json` - Misto
  - `processo4.json` - Curto (poucos ciclos)
  - `processo5.json` - Longo (muitos ciclos)

### 📌 16-17/11/2025 (Fim de Semana)
- [ ] Rodar testes com múltiplos processos
- [ ] Implementar logging de métricas em arquivo
- [ ] Validar que Round Robin está funcionando

### 📌 18/11/2025 (Segunda)
- [ ] Executar baseline (1 núcleo)
- [ ] Executar multicore (2, 4 núcleos)
- [ ] Coletar dados para o artigo

---

## ⚠️ Itens de Alta Prioridade

### 🔴 CRÍTICO (Sem isso, trabalho não funciona)

1. **Integrar RoundRobinScheduler ao main.cpp** ⏰ **14/11**
   - Atualmente o escalonador existe mas não é usado
   - `main.cpp` ainda executa um único processo
   - **Ação:** Substituir loop por `scheduler.schedule_cycle()`

2. **Criar processos de teste (JSON)** ⏰ **15/11**
   - Apenas `tasks.json` existe
   - Precisa de múltiplos processos para testar RR
   - **Ação:** Criar `src/tasks/processo{1-5}.json`

3. **Implementar coleta de métricas** ⏰ **20/11**
   - Métricas calculadas mas não salvas
   - Artigo precisa de dados
   - **Ação:** Criar `logs/metrics.csv` com resultados

### 🟡 IMPORTANTE (Impacta qualidade)

4. **Preempção real por quantum** ⏰ **17/11**
   - Atualmente processo só para quando termina
   - Quantum não interrompe execução
   - **Ação:** Adicionar verificação de quantum no pipeline

5. **Testes de carga multicore** ⏰ **19/11**
   - Testar com 2, 4, 8 núcleos
   - Verificar escalabilidade
   - **Ação:** Criar `test_scalability.cpp`

6. **Gráficos para o artigo** ⏰ **26/11**
   - Speedup, utilização, tempo de espera
   - **Ação:** Script Python com matplotlib

### 🟢 DESEJÁVEL (Melhora apresentação)

7. **Documentação de API**
8. **Testes de race conditions**
9. **README atualizado com instruções de execução**

---

## 📈 Indicadores de Progresso

### Por Categoria:

#### 🏗️ Arquitetura Multicore (75% - Quase Pronto)
```
[████████████████░░░░] 6/8 completos
```
- ✅ Classe Core
- ✅ Cache L1 privada
- ✅ Threads assíncronas
- ✅ Pipeline MIPS por núcleo
- ✅ MemoryManager compartilhado
- ✅ Compilação funcionando
- ⏳ Teste com 4+ núcleos
- ⏳ Documentação de API

#### ⚙️ Escalonamento Round Robin (70% - Em Andamento)
```
[██████████████░░░░░░] 7/10 completos
```
- ✅ Classe RoundRobinScheduler
- ✅ Fila global FIFO
- ✅ Fila de bloqueados
- ✅ PCB estendido
- ✅ Detecção de migração
- ✅ Quantum configurável
- ✅ Estrutura de estatísticas
- ⏳ Integração com main.cpp ⚠️
- ⏳ Preempção real
- ⏳ Métricas calculadas

#### 💾 Gerenciamento de Memória (80% - Funcional)
```
[████████████████░░░░] 4/5 completos
```
- ✅ MemoryManager compartilhado
- ✅ Cache L1 privada
- ✅ Contadores de hits/misses
- ✅ Sincronização
- ⏳ Teste de contenção

#### 🔒 Sincronização (65% - Parcial)
```
[█████████████░░░░░░░] 3/5 completos
```
- ✅ Mutexes no scheduler
- ✅ Threads sincronizadas
- ✅ Atomic operations
- ⏳ Teste de race conditions
- ⏳ Documentação

#### 📊 Métricas e Testes (30% - Atrasado ⚠️)
```
[██████░░░░░░░░░░░░░░] 2/7 completos
```
- ✅ Estrutura de métricas
- ✅ Métodos de cálculo
- ⏳ Processos de teste JSON ⚠️
- ⏳ Logging em arquivo ⚠️
- ⏳ Baseline single-core
- ⏳ Speedup
- ⏳ Gráficos

#### 📄 Artigo IEEE (0% - Não iniciado ⚠️⚠️)
```
[░░░░░░░░░░░░░░░░░░░░] 0/6 completos
```
- ⏳ Template LaTeX
- ⏳ Abstract
- ⏳ Introdução
- ⏳ Metodologia
- ⏳ Resultados
- ⏳ Conclusão

---

## 🎓 Critérios de Avaliação (30 pontos)

> **Fonte:** Especificação oficial - Michel Pires (CEFET-MG, 2025)  
> **Escopo do grupo:** Round Robin + Infraestrutura multicore

### 1️⃣ Implementação - Escalonamento Round Robin (10 pontos)

- [x] ✅ **Arquitetura multicore funcional** (2 pts)
  - Múltiplos núcleos operando
  - Memória compartilhada
  - Cache L1 privada por núcleo

- [x] ✅ **Fila global Round Robin (FIFO)** (1 pt)
  - Implementada em `RoundRobinScheduler`

- [ ] ⚠️ **Cenário 1: Round Robin Não-Preemptivo** (2 pts)
  - Especificação: "Cenário não preemptivo: todas as tarefas são executadas até a conclusão, sem interrupções"
  - Processos executam até terminar (sem quantum)
  - **FALTANDO - Prioridade Alta**

- [ ] ⏳ **Cenário 2: Round Robin Preemptivo** (2 pts)
  - Especificação: "Cenário preemptivo: todas as tarefas são passíveis de interrupção, sendo executadas conforme um quantum"
  - Quantum configurável
  - Context switch funcional

- [ ] ⚠️ **Baseline single-core** (2 pts)
  - Especificação: "deve-se utilizar como baseline a arquitetura single-core previamente desenvolvida"
  - Executar com 1 núcleo
  - Comparar com 2, 4 núcleos
  - **FALTANDO - Obrigatório**

- [ ] ⏳ **Arquivos de log de métricas** (1 pt)
  - Especificação: "todas as execuções devem gerar arquivos de log contendo métricas de desempenho"
  - Tempo médio de espera/retorno
  - Utilização da CPU
  - Throughput

**Pontuação Estimada Atual: 3/10** �

---

### 2️⃣ Implementação - Gerenciamento de Memória (10 pontos)

> **⚠️ NOTA:** Especificação exige políticas de substituição. 
> Se seu grupo NÃO implementará FIFO/LRU, estes pontos serão perdidos ou 
> devem ser implementados por outro membro da equipe.

- [x] ✅ **Infraestrutura básica** (4 pts)
  - Memória compartilhada entre núcleos
  - Cache L1 privada por núcleo
  - Sincronização (mutexes)
  - Contabilização de hits/misses

- [ ] ⚠️⚠️ **Segmentação de memória (modelo Tanenbaum)** (2 pts)
  - Especificação: "mapeamento de endereços segundo um modelo inspirado em Tanenbaum"
  - Endereçamento: parte bits = bloco, parte = offset
  - **FALTANDO - Se não for feito, perde 2 pts**

- [ ] ⚠️⚠️ **Política FIFO** (2 pts)
  - Especificação: "O gerenciamento de memória deve prever políticas de substituição (ex.: FIFO, LRU)"
  - **FALTANDO - Se não for feito, perde 2 pts**

- [ ] ⚠️⚠️ **Política LRU** (2 pts)
  - Especificação: exige políticas de substituição
  - **FALTANDO - Se não for feito, perde 2 pts**

**Pontuação Estimada Atual: 4/10** 🔴  
**Pontuação Possível (sem FIFO/LRU): 6/10** �

---

### 3️⃣ Artigo Científico IEEE (10 pontos)

> **Especificação:** "Artigo científico descrevendo o trabalho desenvolvido, no formato IEEE Conference Template"

- [ ] ⏳ **Estrutura IEEE correta** (1 pt)
  - Template LaTeX ou Word IEEE
  - Formatação adequada

- [ ] ⏳ **Resumo/Abstract** (1 pt)
  - 150-200 palavras
  - Problema, solução, resultados

- [ ] ⏳ **Introdução** (1 pt)
  - Contexto e motivação
  - Objetivos do trabalho

- [ ] ⏳ **Referencial Teórico** (2 pts)
  - Arquiteturas multicore
  - Escalonamento Round Robin
  - Gerenciamento de memória
  - Mínimo 10 referências

- [ ] ⏳ **Metodologia e Implementação** (2 pts)
  - Pseudocódigos
  - Diagramas da arquitetura
  - Fluxogramas

- [ ] ⏳ **Resultados e Discussão** (2 pts)
  - Gráficos comparativos
  - Tabelas de métricas
  - Comparação baseline vs multicore
  - Análise de desempenho

- [ ] ⏳ **Conclusão e Trabalhos Futuros** (1 pt)
  - Síntese das contribuições
  - Limitações e melhorias futuras

**Pontuação Estimada Atual: 0/10** ⭕

---

### **TOTAL AJUSTADO:**

| Componente | Atual | Com FIFO/LRU | Máximo |
|------------|-------|--------------|--------|
| Escalonamento | **3/10** 🔴 | **10/10** ✅ | 10 |
| Memória | **4/10** 🔴 | **10/10** ✅ | 10 |
| Artigo | **0/10** ⭕ | **10/10** ✅ | 10 |
| **TOTAL** | **7/30** (23%) | **30/30** (100%) ✅ | **30** |

> ⚠️ **CRÍTICO:** FIFO/LRU vale 6 pontos! Ver **ETAPA 3.5** para guia completo.
> 
> **Divisão Grupo:**
> 1. ✅ **Você:** Round Robin (10 pts) - 70% completo
> 2. 🔴 **Outro membro:** FIFO/LRU (6 pts) - **VER ETAPA 3.5**
> 3. 👥 **Todos:** Artigo IEEE (10 pts) - iniciar 27/11
> 
> **ETAPA 3.5** contém:
> - 📝 Código completo de FIFOPolicy e LRUPolicy
> - 🧪 Testes unitários prontos
> - 📅 Divisão de tarefas por dia (19-23/11)
> - 🔗 Integração com Cache existente

---

## 🚨 Riscos e Mitigação

| Risco | Probabilidade | Impacto | Mitigação |
|-------|---------------|---------|-----------|
| **Não implementar cenário não-preemptivo** | � Alta | 🔴 Alto (-2 pts) | Prioridade máxima - fazer até 17/11 |
| **Não testar baseline single-core** | 🔴 Alta | 🔴 Alto (-2 pts) | Reservar 24-26/11 para testes |
| **Não gerar arquivos de log** | 🟡 Média | 🔴 Alto (-1 pt) | Implementar até 20/11 |
| **Não implementar FIFO/LRU** | 🟡 Média | 🔴 CRÍTICO (-6 pts) | **VER ETAPA 3.5** - Outro membro deve implementar |
| **Artigo incompleto/atrasado** | � Média | � Alto (-10 pts) | Começar em 27/11, dividir seções |
| **Cenário preemptivo mal implementado** | � Baixa | 🟡 Médio (-2 pts) | Testar quantum funciona corretamente |

---

## 📚 Recursos Disponíveis

### Documentação Interna:
- ✅ `docs/08-round-robin.md` - Guia completo de Round Robin (684 linhas)
- ✅ `docs/COMPILACAO_SUCESSO.md` - Resumo de implementação
- ✅ `docs/WSL_QUICKSTART.md` - Guia de compilação
- ✅ `docs/02-requisitos.md` - Requisitos detalhados
- ✅ `docs/04-roadmap.md` - Roadmap original

### Código Implementado:
- ✅ `src/cpu/Core.hpp/cpp` - Núcleo de processamento
- ✅ `src/cpu/RoundRobinScheduler.hpp/cpp` - Escalonador
- ✅ `src/cpu/PCB.hpp` - PCB estendido
- ✅ `src/memory/MemoryManager.hpp/cpp` - Gerenciador de memória
- ✅ `Makefile` - Build configurado

### Ferramentas:
- ✅ GCC 13 (C++17)
- ✅ WSL Ubuntu
- ✅ Make
- ⏳ Python + matplotlib (para gráficos)
- ⏳ LaTeX (para artigo)

---

## 🎯 Ações Imediatas (Próximas 48h)

### Amanhã - 14/11/2025:
1. ⚠️ **Modificar `src/main.cpp`**
   - Instanciar `RoundRobinScheduler`
   - Loop: `while (scheduler.has_pending_processes()) { scheduler.schedule_cycle(); }`
   - Remover execução direta do processo

2. ⚠️ **Testar execução básica**
   - Compilar: `make simulador`
   - Executar: `./simulador`
   - Verificar que escalonador está funcionando

### 15/11/2025:
3. ⚠️ **Criar processos de teste**
   - Copiar `src/tasks/tasks.json` como template
   - Criar variações de carga
   - Documentar em `docs/EXEMPLOS_JSON.md`

4. ⚠️ **Modificar main.cpp para carregar múltiplos processos**
   - Loop para carregar `processo1.json`, `processo2.json`, etc.
   - `scheduler.add_process(pcb)` para cada um

---

## 📞 Divisão de Tarefas (Sugerida)

### Pessoa 1 - Escalonador:
- [ ] Integrar RoundRobinScheduler ao main.cpp
- [ ] Implementar preempção por quantum
- [ ] Testar com múltiplos processos

### Pessoa 2 - Testes:
- [ ] Criar 5 processos JSON
- [ ] Executar baseline (1 núcleo)
- [ ] Executar multicore (2, 4 núcleos)
- [ ] Coletar dados

### Pessoa 3 - Métricas:
- [ ] Implementar logging em arquivo
- [ ] Calcular speedup
- [ ] Criar gráficos (Python)

### Pessoa 4 - Artigo:
- [ ] Baixar template IEEE
- [ ] Escrever Abstract + Introdução
- [ ] Escrever Metodologia
- [ ] Compilar resultados + conclusão

---

## ✅ Como Marcar um Achievement

Quando completar uma tarefa:

1. **Mudar `[ ]` para `[x]`**
2. **Adicionar data de conclusão: `✅ (DD/MM/2025)`**
3. **Atualizar percentuais no topo**
4. **Commitar a mudança:**
   ```bash
   git add docs/ACHIEVEMENTS.md
   git commit -m "Achievement: [nome da tarefa]"
   ```

---

## 🏁 Definição de "Pronto"

Uma etapa está completa quando:

- ✅ Código compila sem erros
- ✅ Testes básicos passam
- ✅ Documentação atualizada
- ✅ Commit feito no repositório
- ✅ Revisado por outro membro da equipe

---

## 📋 **CHECKLIST FINAL - Requisitos do Professor**

> **Use esta lista para garantir que NADA foi esquecido!**

### ✅ **Implementação - Escalonamento Round Robin (10 pts):**

- [x] ✅ Arquitetura multicore funcional (2 pts)
- [x] ✅ Fila FIFO de processos (1 pt)
- [ ] ⚠️ **Cenário não-preemptivo** (2 pts) - **FALTA**
- [ ] ⏳ **Cenário preemptivo com quantum** (2 pts) - **EM ANDAMENTO**
- [ ] ⚠️ **Baseline single-core comparativo** (2 pts) - **FALTA**
- [ ] ⏳ **Arquivos de log gerados** (1 pt) - **FALTA**

**Status:** 3/10 pts (30%) - **7 pontos em risco!**

---

### ✅ **Implementação - Gerenciamento de Memória (10 pts):**

#### Sua Parte (Infraestrutura - 4 pts):
- [x] ✅ Memória compartilhada entre núcleos
- [x] ✅ Cache L1 privada por núcleo
- [x] ✅ Sincronização thread-safe
- [x] ✅ Contabilização hits/misses

#### Outro Membro (Políticas - 6 pts):
- [ ] ⏳ Segmentação modelo Tanenbaum (2 pts)
- [ ] ⏳ Política FIFO (2 pts)
- [ ] ⏳ Política LRU (2 pts)

**Status:** 4/10 pts (40%) - **6 pontos em risco se ninguém fizer!**

---

### ✅ **Artigo Científico IEEE (10 pts):**

- [ ] ⏳ Template IEEE baixado e configurado
- [ ] ⏳ Resumo/Abstract escrito
- [ ] ⏳ Introdução escrita
- [ ] ⏳ Referencial Teórico (mín. 10 refs)
- [ ] ⏳ Metodologia e Implementação
- [ ] ⏳ Resultados com gráficos comparativos
- [ ] ⏳ Conclusão e Trabalhos Futuros

**Status:** 0/10 pts (0%) - **10 pontos em risco!**

---

### ✅ **Requisitos Técnicos Específicos:**

#### Carga de Processos:
- [ ] ⚠️ **Todos processos carregados ANTES da execução**
- [ ] ⚠️ **Sem novos processos durante execução**
- [ ] ⚠️ **Mínimo 5 processos JSON diferentes**

#### Cenários Experimentais:
- [ ] ⚠️ **Cenário 1: Não-preemptivo implementado**
- [ ] ⏳ **Cenário 2: Preemptivo implementado**
- [ ] ⏳ Context switch funcional entre cenários

#### Baseline e Comparação:
- [ ] ⚠️ **Executar com 1 núcleo (baseline)**
- [ ] ⏳ Executar com 2 núcleos
- [ ] ⏳ Executar com 4 núcleos
- [ ] ⏳ Calcular speedup
- [ ] ⏳ Gerar gráficos comparativos

#### Métricas Obrigatórias:
- [ ] ⏳ Tempo médio de espera
- [ ] ⏳ Tempo médio de retorno
- [ ] ⏳ Utilização média da CPU
- [ ] ⏳ Eficiência por núcleo
- [ ] ⏳ Throughput total

#### Logs e Documentação:
- [ ] ⚠️ **Arquivos de log gerados automaticamente**
- [ ] ⏳ Logs com formato estruturado (CSV/JSON)
- [ ] ⏳ Código comentado e publicado no GitHub
- [ ] ⏳ README com instruções de compilação

---

## 🎯 **RESUMO DE RISCO POR PONTOS:**

| Item | Pontos | Status | Prazo | Risco |
|------|--------|--------|-------|-------|
| Cenário não-preemptivo | 2 | ⚠️ FALTA | 17/11 | 🔴 ALTO |
| Baseline single-core | 2 | ⚠️ FALTA | 24/11 | 🔴 ALTO |
| Cenário preemptivo | 2 | ⏳ 70% | 19/11 | 🟡 MÉDIO |
| Logs de métricas | 1 | ⏳ 0% | 20/11 | 🟡 MÉDIO |
| Arquitetura multicore | 2 | ✅ FEITO | - | 🟢 SEGURO |
| Fila FIFO processos | 1 | ✅ FEITO | - | 🟢 SEGURO |
| **TOTAL ESCALONAMENTO** | **10** | **30%** | - | **🔴 7 pts em risco** |
| | | | | |
| Infraestrutura memória | 4 | ✅ FEITO | - | 🟢 SEGURO |
| FIFO/LRU (outro membro) | 6 | ⏳ 0% | 23/11 | 🔴 ALTO |
| **TOTAL MEMÓRIA** | **10** | **40%** | - | **🔴 6 pts em risco** |
| | | | | |
| Artigo IEEE | 10 | ⏳ 0% | 06/12 | 🔴 ALTO |
| **TOTAL ARTIGO** | **10** | **0%** | - | **🔴 10 pts em risco** |
| | | | | |
| **TOTAL GERAL** | **30** | **23%** | - | **🔴🔴🔴 23 pts em risco!** |

---

**Última atualização:** 13/11/2025 23:59  
**Próxima revisão:** 16/11/2025

---

> 💡 **Dica:** Mantenha este arquivo aberto durante o desenvolvimento para visualizar progresso em tempo real!
