# Introdução

## Sobre o Projeto

O **Simulador Von Neumann Multicore** é um sistema que simula uma arquitetura de computador com múltiplos núcleos de processamento, implementando:

- Pipeline MIPS de 5 estágios (IF, ID, EX, MEM, WB)
- Múltiplas políticas de escalonamento de processos
- Hierarquia de memória com Cache L1, RAM e Disco
- Coleta de métricas de desempenho

## Objetivos

1. **Simular arquitetura multicore** com 1 a 6 núcleos
2. **Implementar escalonadores** FCFS, SJN, Round Robin e Priority
3. **Gerenciar memória** com cache privada por núcleo e memória compartilhada
4. **Coletar métricas** para análise comparativa de desempenho

## Componentes Principais

### CPU e Núcleos

Cada núcleo (`Core`) possui:
- Pipeline MIPS completo de 5 estágios
- Cache L1 privada (128 linhas)
- Banco de registradores próprio
- ULA (Unidade Lógica Aritmética)

```
┌─────────────────────────────────────────────────────────┐
│                   ARQUITETURA MULTICORE                 │
├─────────────────────────────────────────────────────────┤
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐    │
│  │ Core 0  │  │ Core 1  │  │ Core 2  │  │ Core n  │    │
│  │ ┌─────┐ │  │ ┌─────┐ │  │ ┌─────┐ │  │ ┌─────┐ │    │
│  │ │Cache│ │  │ │Cache│ │  │ │Cache│ │  │ │Cache│ │    │
│  │ └─────┘ │  │ └─────┘ │  │ └─────┘ │  │ └─────┘ │    │
│  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘    │
│       └───────────┬─────────────┬───────────┬─┘        │
│                   │   Barramento │           │          │
│       ┌───────────▼─────────────▼───────────▼─┐        │
│       │      MEMÓRIA PRINCIPAL (RAM)          │        │
│       └───────────────┬───────────────────────┘        │
│                       │                                 │
│       ┌───────────────▼───────────────────────┐        │
│       │    MEMÓRIA SECUNDÁRIA (DISCO)         │        │
│       └───────────────────────────────────────┘        │
│                                                         │
│  ┌───────────────────────────────────────────────────┐ │
│  │              ESCALONADOR DE PROCESSOS             │ │
│  │  ┌─────────────────────────────────────────────┐  │ │
│  │  │  Fila de Prontos │ P1 │ P2 │ P3 │ P4 │...  │  │ │
│  │  └─────────────────────────────────────────────┘  │ │
│  └───────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Escalonadores

O sistema suporta 5 políticas de escalonamento:

| Política | Tipo | Descrição |
|----------|------|-----------|
| **FCFS** | Não-preemptivo | First Come First Served - ordem de chegada |
| **SJN** | Não-preemptivo | Shortest Job Next - menor job primeiro |
| **Round Robin** | Preemptivo | Fatia de tempo (quantum) para cada processo |
| **Priority** | Não-preemptivo | Maior prioridade primeiro |
| **Priority Preemptivo** | Preemptivo | Prioridade com preempção |

### Gerenciamento de Memória

- **Cache L1**: Privada por núcleo, 128 linhas, política FIFO ou LRU
- **RAM**: Memória principal compartilhada entre núcleos
- **Disco**: Memória secundária para swap

## Métricas Coletadas

O sistema coleta automaticamente:

| Métrica | Descrição |
|---------|-----------|
| Tempo de Espera | Tempo na fila de prontos |
| Tempo de Turnaround | Tempo total no sistema |
| Tempo de Resposta | Tempo até primeira execução |
| Throughput | Processos completados por segundo |
| Utilização de CPU | Percentual de tempo ocupado |
| Cache Hit Rate | Taxa de acerto na cache |

## Tecnologias

- **Linguagem**: C++17
- **Threading**: std::thread, std::mutex, std::atomic
- **Build**: Makefile
- **Documentação**: Docsify + Markdown
