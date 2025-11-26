# Casos de Teste

## 🎯 Objetivo

Definir casos de teste específicos e detalhados para validar o comportamento do simulador multicore em diferentes cenários.

---

## 📋 Casos de Teste Unitários

### CT-U001: Criação de Core

**Descrição:** Verificar criação correta de um core.

**Pré-condições:** Sistema inicializado

**Passos:**
1. Criar um core com ID 0
2. Verificar ID do core
3. Verificar estado inicial (ocioso)

**Resultado Esperado:**
- Core criado com ID correto
- Estado inicial: ocioso
- Sem processos atribuídos

**Critérios de Sucesso:**
```cpp
EXPECT_EQ(core->getCoreId(), 0);
EXPECT_FALSE(core->isBusy());
EXPECT_EQ(core->getCurrentProcess(), nullptr);
```

---

### CT-U002: Adicionar Processo ao Scheduler

**Descrição:** Verificar adição de processo à fila.

**Pré-condições:** Scheduler criado

**Passos:**
1. Criar processo com PID 1
2. Adicionar ao scheduler
3. Verificar tamanho da fila

**Resultado Esperado:**
- Processo adicionado à fila
- Tamanho da fila = 1

**Critérios de Sucesso:**
```cpp
EXPECT_EQ(scheduler->getQueueSize(), 1);
EXPECT_TRUE(scheduler->hasProcess(1));
```

---

### CT-U003: Alocação de Segmento

**Descrição:** Verificar alocação de segmento de memória.

**Pré-condições:** 
- SegmentationManager criado
- Memória disponível suficiente

**Passos:**
1. Criar tabela de segmentos para PID 1
2. Alocar segmento CODE de 4096 bytes
3. Verificar alocação

**Resultado Esperado:**
- Segmento alocado com sucesso
- Memória disponível diminuída
- Segmento na tabela do processo

**Critérios de Sucesso:**
```cpp
EXPECT_TRUE(manager->allocateSegment(1, CODE, 4096));
EXPECT_LT(manager->getAvailableMemory(), initial_memory);
```

---

## 🔗 Casos de Teste de Integração

### CT-I001: Execução Multicore Básica

**Descrição:** Testar execução de processos em múltiplos cores.

**Pré-condições:**
- Sistema com 2 cores
- 4 processos disponíveis

**Passos:**
1. Inicializar sistema com 2 cores
2. Adicionar 4 processos
3. Iniciar execução
4. Aguardar conclusão
5. Verificar distribuição

**Resultado Esperado:**
- Todos os 4 processos executados
- Processos distribuídos entre os 2 cores
- Sem erros de sincronização

**Dados de Teste:**
```json
{
  "processes": [
    {"pid": 1, "burst_time": 100},
    {"pid": 2, "burst_time": 150},
    {"pid": 3, "burst_time": 120},
    {"pid": 4, "burst_time": 180}
  ]
}
```

**Critérios de Sucesso:**
```cpp
EXPECT_EQ(completed_processes, 4);
EXPECT_GT(core0_processes, 0);
EXPECT_GT(core1_processes, 0);
```

---

### CT-I002: Round Robin com Quantum

**Descrição:** Validar preempção por quantum no Round Robin.

**Pré-condições:**
- Scheduler RR com quantum = 10
- 3 processos com burst_time > quantum

**Passos:**
1. Configurar quantum = 10 ciclos
2. Adicionar 3 processos (burst_time = 50)
3. Executar e monitorar preempções
4. Verificar que cada processo executa por quantum

**Resultado Esperado:**
- Processos intercalados
- Cada execução limitada a quantum
- Ordem Round Robin mantida

**Critérios de Sucesso:**
```cpp
EXPECT_EQ(process1->quantum_count, 5); // 50/10
EXPECT_EQ(process2->quantum_count, 5);
EXPECT_EQ(process3->quantum_count, 5);
```

---

### CT-I003: Política LRU de Substituição

**Descrição:** Testar substituição de segmentos com LRU.

**Pré-condições:**
- Memória limitada (1MB)
- Política LRU ativa

**Passos:**
1. Alocar segmentos até preencher 80% da memória
2. Acessar segmentos em ordem: S1, S2, S3, S1, S2
3. Alocar novo segmento grande (requer substituição)
4. Verificar que S3 foi removido (LRU)

**Resultado Esperado:**
- S3 removido (menos recentemente usado)
- S1 e S2 permanecem em memória
- Novo segmento alocado

**Critérios de Sucesso:**
```cpp
EXPECT_FALSE(s3->in_memory);
EXPECT_TRUE(s1->in_memory);
EXPECT_TRUE(s2->in_memory);
```

---

## 🎭 Casos de Teste de Sistema

### CT-S001: Simulação Completa (Carga Baixa)

**Descrição:** Executar simulação completa com carga baixa.

**Configuração:**
- Cores: 4
- Processos: 10
- Quantum: 20
- Memória: 10MB
- Política: LRU

**Processos:**
```json
{
  "processes": [
    {"pid": 1, "arrival": 0, "burst": 100, "memory": "512KB"},
    {"pid": 2, "arrival": 10, "burst": 150, "memory": "256KB"},
    {"pid": 3, "arrival": 20, "burst": 200, "memory": "1MB"},
    {"pid": 4, "arrival": 30, "burst": 80, "memory": "128KB"},
    {"pid": 5, "arrival": 40, "burst": 120, "memory": "512KB"},
    {"pid": 6, "arrival": 50, "burst": 90, "memory": "256KB"},
    {"pid": 7, "arrival": 60, "burst": 110, "memory": "512KB"},
    {"pid": 8, "arrival": 70, "burst": 160, "memory": "1MB"},
    {"pid": 9, "arrival": 80, "burst": 130, "memory": "256KB"},
    {"pid": 10, "arrival": 90, "burst": 140, "memory": "512KB"}
  ]
}
```

**Métricas Esperadas:**
- Throughput: > 5 proc/s
- Utilização média de CPU: > 70%
- Taxa de acerto de memória: > 85%
- Tempo médio de turnaround: < 300ms

---

### CT-S002: Simulação Completa (Carga Alta)

**Descrição:** Testar sistema sob alta carga.

**Configuração:**
- Cores: 8
- Processos: 100
- Quantum: 10
- Memória: 50MB
- Política: FIFO

**Características:**
- Processos variados (burst: 50-500ms)
- Chegada: uniforme
- Memória: 128KB - 5MB

**Métricas Esperadas:**
- Throughput: > 20 proc/s
- Utilização média de CPU: > 85%
- Fairness index: > 0.8
- Sem deadlocks

---

### CT-S003: Teste de Stress

**Descrição:** Testar limites do sistema.

**Configuração:**
- Cores: 16
- Processos: 1000
- Quantum: 5
- Memória: 100MB

**Objetivos:**
- Verificar estabilidade
- Identificar memory leaks
- Avaliar escalabilidade
- Medir overhead

**Critérios de Sucesso:**
- Todos os processos completam
- Sem crashes
- Sem memory leaks (valgrind)
- Tempo total < 10 minutos

---

## 🐛 Casos de Teste de Erro

### CT-E001: Segmentation Fault

**Descrição:** Verificar tratamento de acesso inválido à memória.

**Passos:**
1. Alocar segmento de 4096 bytes
2. Tentar acessar offset 10000 (inválido)
3. Capturar exceção

**Resultado Esperado:**
- Exceção `std::runtime_error` lançada
- Mensagem: "Segmentation fault"
- Processo não afeta sistema

**Critérios de Sucesso:**
```cpp
EXPECT_THROW(
    manager->accessSegment(1, 0, 10000),
    std::runtime_error
);
```

---

### CT-E002: Memória Insuficiente

**Descrição:** Testar comportamento com memória esgotada.

**Passos:**
1. Preencher memória até 100%
2. Tentar alocar novo segmento
3. Verificar falha ou swap

**Resultado Esperado:**
- Alocação falha ou
- Segmento antigo é removido (swap)

**Critérios de Sucesso:**
```cpp
bool success = manager->allocateSegment(1, HEAP, 10*1024*1024);
if (!success) {
    // Falha esperada
    EXPECT_EQ(manager->getAvailableMemory(), 0);
} else {
    // Swap ocorreu
    EXPECT_GT(manager->getSwapCount(), 0);
}
```

---

### CT-E003: Deadlock Detection

**Descrição:** Verificar detecção de deadlock.

**Passos:**
1. Criar situação propícia a deadlock
2. Thread 1: lock(A), aguarda, lock(B)
3. Thread 2: lock(B), aguarda, lock(A)
4. Usar timeout para detectar

**Resultado Esperado:**
- Deadlock detectado
- Sistema não trava
- Erro reportado

---

## 📊 Casos de Teste de Performance

### CT-P001: Speedup Multicore

**Descrição:** Medir speedup com diferentes números de cores.

**Configuração:**
- Testar com: 1, 2, 4, 8 cores
- Mesmo conjunto de 50 processos
- Medir tempo total

**Métricas:**
```
Cores | Tempo | Speedup | Eficiência
------|-------|---------|------------
  1   | 10.0s |  1.0x   |   100%
  2   |  5.5s |  1.8x   |    90%
  4   |  3.0s |  3.3x   |    82%
  8   |  1.8s |  5.5x   |    69%
```

**Critérios de Sucesso:**
- Speedup aumenta com cores
- Eficiência > 60%
- Speedup < número de cores (overhead esperado)

---

### CT-P002: Comparação FIFO vs LRU

**Descrição:** Comparar políticas de substituição.

**Configuração:**
- Mesmo workload
- Memória limitada
- Executar com FIFO e LRU

**Métricas Esperadas:**
| Métrica | FIFO | LRU |
|---------|------|-----|
| Hit Rate | 75% | 85% |
| Swaps | 150 | 100 |
| Tempo Total | 12s | 10s |

**Critérios de Sucesso:**
- LRU tem melhor hit rate
- LRU tem menos swaps
- LRU é mais rápido

---

## 🔄 Casos de Teste de Regressão

### CT-R001: Funcionalidades Base

**Descrição:** Garantir que código base não foi quebrado.

**Testes:**
- Pipeline de 5 estágios funciona
- Banco de registradores intacto
- ULA opera corretamente
- Memória principal funcional
- Cache L1 operacional

**Critérios de Sucesso:**
- Todos os testes base passam
- Comportamento idêntico ao original

---

## 📝 Template de Caso de Teste

```markdown
### CT-XXX: [Nome do Caso]

**Descrição:** [O que está sendo testado]

**Pré-condições:**
- [Condição 1]
- [Condição 2]

**Passos:**
1. [Passo 1]
2. [Passo 2]
3. [Passo 3]

**Dados de Entrada:**
[JSON, valores, etc.]

**Resultado Esperado:**
[O que deve acontecer]

**Critérios de Sucesso:**
```cpp
[Código de verificação]
```

**Prioridade:** [Alta/Média/Baixa]

**Status:** [Pendente/Passando/Falhando]
```

---

## 📊 Matriz de Rastreabilidade

| Caso | Requisito | Componente | Prioridade | Status |
|------|-----------|------------|------------|--------|
| CT-U001 | REQ-001 | Core | Alta | ✅ |
| CT-U002 | REQ-002 | Scheduler | Alta | ✅ |
| CT-U003 | REQ-003 | Memory | Alta | ✅ |
| CT-I001 | REQ-004 | MultiCore | Alta | 🟡 |
| CT-I002 | REQ-005 | RR | Alta | 🟡 |
| CT-S001 | REQ-ALL | Sistema | Média | ⏳ |

**Legenda:**
- ✅ Passando
- 🟡 Em Progresso
- ❌ Falhando
- ⏳ Pendente

---

## 🔗 Próximos Passos

- ➡️ [Debugging](14-debugging.md)
- ➡️ [Estrutura do Artigo IEEE](15-estrutura-artigo.md)

---

## 📚 Referências

- IEEE Std 829-2008 (Test Documentation)
- MYERS, G. J. The Art of Software Testing
