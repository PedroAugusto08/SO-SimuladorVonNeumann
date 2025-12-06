# FAQ - Perguntas Frequentes

## Geral

### O que é este simulador?

É um simulador de sistema operacional multi-core baseado na arquitetura Von Neumann. Implementa escalonamento de processos, hierarquia de memória com cache, e coleta métricas de performance.

### Para que serve?

- Estudo de algoritmos de escalonamento
- Compreensão de hierarquia de memória
- Análise de performance de sistemas multi-core
- Trabalhos acadêmicos de Sistemas Operacionais

### Quais linguagens são suportadas?

O simulador é escrito em **C++17**. Os programas executados são definidos em JSON.

---

## Instalação

### Funciona no Windows?

Não nativamente. Use **WSL2** (Windows Subsystem for Linux) com Ubuntu.

```powershell
wsl --install -d Ubuntu
```

### Preciso de root/sudo?

Apenas para instalar dependências:
```bash
sudo apt install build-essential g++ make
```

### Qual versão do GCC é necessária?

GCC/G++ 9.0 ou superior (para suporte completo a C++17).

---

## Compilação

### Como compilar?

```bash
make simulador
```

### Como recompilar do zero?

```bash
make clean && make simulador
```

### Como rodar os testes?

```bash
# Teste de métricas completo
make test-metrics

# Teste single-core determinístico
make test-single-core
```

### Erro "undefined reference to pthread"?

Verifique se o Makefile inclui `-pthread` nas flags.

---

## Execução

### Qual o comando básico?

```bash
./bin/simulador --policy FCFS --cores 2 -p tasks.json process.json
```

### Quais políticas estão disponíveis?

| Política | Flag |
|----------|------|
| First Come First Served | `--policy FCFS` |
| Shortest Job Next | `--policy SJN` |
| Round Robin | `--policy RR` |
| Priority (não-preemptivo) | `--policy PRIORITY` |

> **Nota:** A política `PRIORITY_PREEMPT` foi temporariamente desabilitada.
| Priority Preemptivo | `--policy PRIORITY_PREEMPT` |

### Como usar Round Robin?

```bash
./simulador --policy RR --quantum 1000 --cores 2 -p tasks.json proc.json
```

### Posso executar múltiplos processos?

Sim! Use múltiplos `-p`:
```bash
./simulador --policy FCFS --cores 4 \
    -p tasks.json proc1.json \
    -p tasks.json proc2.json \
    -p tasks.json proc3.json
```

---

## Métricas

### Onde ficam as métricas?

Em `logs/metrics/detailed_metrics.csv`.

### O que significa cada métrica?

| Métrica | Significado |
|---------|-------------|
| Wait Time | Tempo na fila de prontos |
| Turnaround | Tempo total (chegada → término) |
| Throughput | Instruções por ciclo |

### Como analisar as métricas?

```python
import pandas as pd
df = pd.read_csv('logs/metrics/detailed_metrics.csv')
print(df.describe())
```

---

## Escalonamento

### Qual política é melhor?

Depende do cenário:

| Cenário | Melhor Política |
|---------|-----------------|
| Jobs de tamanho conhecido | SJN |
| Sistema interativo | Round Robin |
| Prioridades diferentes | Priority |
| Simplicidade | FCFS |

### O que é starvation?

Quando um processo nunca executa porque outros têm prioridade. Pode ocorrer com SJN e Priority.

### O que é quantum?

Fatia de tempo que cada processo recebe no Round Robin antes de ser preemptado.

---

## Memória

### Como funciona a cache?

Cada núcleo tem cache L1 privada. Acessos verificam cache antes de ir para RAM.

### FIFO vs LRU - qual usar?

**LRU** geralmente tem melhor hit rate para workloads com localidade temporal.

### O que é hit rate?

Percentual de acessos encontrados na cache:
$$Hit\ Rate = \frac{Hits}{Hits + Misses} \times 100\%$$

---

## Problemas Comuns

### Programa trava (deadlock)?

1. Verifique se todos os processos podem terminar
2. Use timeout nos testes
3. Veja [Troubleshooting](../uso/troubleshooting.md)

### Resultados inconsistentes?

Pode ser race condition. Compile com ThreadSanitizer:
```bash
g++ -fsanitize=thread ...
```

### Segmentation fault?

Compile com AddressSanitizer:
```bash
g++ -fsanitize=address -g ...
```

---

## Desenvolvimento

### Como adicionar nova política?

1. Criar classe herdando de `Scheduler`
2. Implementar `add_process()` e `schedule_cycle()`
3. Adicionar no factory de escalonadores

### Como adicionar nova instrução?

1. Adicionar opcode em `instruction_codes.hpp`
2. Implementar em `CONTROL_UNIT.cpp`
3. Adicionar no parser JSON

### Como contribuir?

1. Fork do repositório
2. Criar branch para feature
3. Implementar com testes
4. Abrir Pull Request

---

## Recursos

### Onde encontrar mais informação?

- [Arquitetura](../guia/arquitetura.md)
- [Escalonadores](../escalonadores/fcfs.md)
- [Bibliografia](bibliografia.md)

### Preciso de ajuda!

1. Verifique [Troubleshooting](../uso/troubleshooting.md)
2. Consulte a documentação
3. Abra issue no repositório
