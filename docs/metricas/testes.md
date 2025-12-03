# Testes

## Visão Geral

O projeto inclui testes abrangentes para validar todas as funcionalidades do simulador.

## Estrutura de Testes

```
test/
├── test_cpu_metrics.cpp          # Métricas de CPU
├── test_deep_inspection.cpp      # Inspeção detalhada
├── test_metrics_complete.cpp     # Métricas completas
├── test_multicore_comparative.cpp # Comparação multi-core
├── test_multicore_throughput.cpp  # Throughput multi-core
├── test_multicore.cpp            # Testes multi-core
├── test_priority_preemptive.cpp  # Priority preemptivo
├── test_race_debug.cpp           # Debug de race conditions
└── test_verify_execution.cpp     # Verificação de execução
```

## Compilação

```bash
# Compilar todos os testes
make test

# Compilar teste específico
make test_multicore
make test_metrics
make test_priority
```

## Testes Disponíveis

### test_multicore.cpp

Testa execução multi-core básica:
- Inicialização de múltiplos núcleos
- Distribuição de processos
- Sincronização entre núcleos

```bash
./test_multicore
```

### test_multicore_comparative.cpp

Compara performance entre políticas de escalonamento:
- FCFS vs SJN vs Round Robin vs Priority
- Métricas: throughput, latência, fairness
- Executa com múltiplas configurações de núcleos

```bash
./test_multicore_comparative
```

**Saída típica:**
```
=== Comparativo de Políticas (4 cores) ===
Política         | Throughput | Avg Wait | Fairness
-----------------+------------+----------+---------
FCFS             | 1.23       | 450      | 0.85
SJN              | 1.45       | 320      | 0.72
Round Robin      | 1.18       | 380      | 0.95
Priority         | 1.35       | 350      | 0.68
```

### test_priority_preemptive.cpp

Testa escalonamento com prioridade preemptiva:
- Preempção quando processo de maior prioridade chega
- Validação de quantum
- Context switches corretos

```bash
./test_priority_preemptive
```

### test_metrics_complete.cpp

Valida coleta de métricas:
- Wait time calculado corretamente
- Turnaround time correto
- Contagem de instruções
- Exportação para CSV

```bash
./test_metrics_complete
```

### test_race_debug.cpp

Debug de condições de corrida:
- Acesso concorrente à memória
- Sincronização de processos
- Deadlock detection

```bash
./test_race_debug
```

## Executando Testes

### Todos os Testes

```bash
make test
./run_all_tests.sh
```

### Teste Individual

```bash
make test_multicore
./test_multicore
```

### Com Valgrind (Memory Check)

```bash
valgrind --leak-check=full ./test_multicore
```

## Escrevendo Novos Testes

### Template

```cpp
#include <iostream>
#include <cassert>
#include "cpu/Scheduler.hpp"
#include "memory/MemoryManager.hpp"

void test_feature() {
    std::cout << "Testing feature... ";
    
    // Setup
    MemoryManager mem;
    FCFSScheduler scheduler(2, &mem, nullptr);
    
    // Test
    PCB process;
    process.pid = 1;
    scheduler.add_process(&process);
    
    // Assert
    assert(scheduler.ready_queue_size() == 1);
    
    std::cout << "PASSED" << std::endl;
}

int main() {
    test_feature();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
```

### Adicionando ao Makefile

```makefile
test_my_feature: test/test_my_feature.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
```

## Validação de Resultados

### Verificações Automáticas

1. **Todas instruções executadas**: Nenhum processo fica incompleto
2. **Sem deadlocks**: Simulador sempre termina
3. **Métricas consistentes**: Soma dos tempos bate com total
4. **Memória limpa**: Sem leaks (verificado com Valgrind)

### Critérios de Sucesso

```cpp
void validate_execution() {
    // Todos processos terminaram?
    assert(finished_processes == total_processes);
    
    // Métricas fazem sentido?
    for (auto& proc : processes) {
        assert(proc.turnaround >= proc.execution_time);
        assert(proc.wait_time >= 0);
        assert(proc.instructions_executed > 0);
    }
    
    // Consistência temporal
    assert(total_cycles >= max_end_time);
}
```

## CI/CD

Para integração contínua:

```yaml
# .github/workflows/test.yml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Build
        run: make all
        
      - name: Run Tests
        run: make test && ./run_all_tests.sh
        
      - name: Memory Check
        run: valgrind --error-exitcode=1 ./test_multicore
```

## Troubleshooting

### Teste falha com assertion

1. Verificar se PCB está inicializado corretamente
2. Checar se scheduler tem processos
3. Verificar ordem de execução

### Teste trava (deadlock)

1. Verificar locks aninhados
2. Checar condições de término
3. Adicionar timeouts

### Memória corrompida

1. Executar com AddressSanitizer:
   ```bash
   g++ -fsanitize=address -g test_multicore.cpp -o test
   ./test
   ```

2. Verificar acessos out-of-bounds
3. Checar uso após free
