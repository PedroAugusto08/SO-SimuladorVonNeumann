# Simulador Von Neumann Multicore

## Visão Geral

Simulador de sistema operacional multi-core baseado na arquitetura Von Neumann, implementado em C++17 com suporte a múltiplas políticas de escalonamento e hierarquia de memória com cache.

## Características

| Componente | Descrição |
|------------|-----------|
| **Núcleos** | 1-8 núcleos configuráveis |
| **Escalonadores** | FCFS, SJN, Round Robin, Priority, Priority Preemptivo |
| **Memória** | Cache L1 por núcleo + RAM compartilhada + Disco |
| **Cache** | 128 linhas por núcleo, políticas FIFO/LRU |
| **Métricas** | Wait time, turnaround, throughput, cache hit rate |

## Início Rápido

### Requisitos

- Linux (Ubuntu 20.04+) ou WSL2
- GCC 9.0+ com suporte a C++17
- Make

### Instalação

```bash
# Instalar dependências
sudo apt install build-essential g++ make

# Compilar
make

# Executar teste
./simulador --policy FCFS --cores 2 \
    -p examples/programs/tasks.json examples/processes/process1.json
```

## Uso Básico

```bash
# FCFS com 2 núcleos
./simulador --policy FCFS --cores 2 -p tasks.json process.json

# Round Robin com quantum de 1000
./simulador --policy RR --cores 4 --quantum 1000 -p tasks.json process.json

# SJN (Shortest Job Next)
./simulador --policy SJN --cores 2 -p tasks.json process.json

# Priority com cache LRU
./simulador --policy PRIORITY --cores 4 --cache-policy LRU -p tasks.json process.json
```

## Documentação

| Seção | Descrição |
|-------|-----------|
| [Introdução](guia/introducao.md) | Visão geral do sistema |
| [Arquitetura](guia/arquitetura.md) | Estrutura e componentes |
| [Escalonadores](escalonadores/fcfs.md) | Políticas disponíveis |
| [Memória](memoria/visao-geral.md) | Hierarquia e cache |
| [Métricas](metricas/metricas.md) | Sistema de coleta |
| [Instalação](uso/instalacao.md) | Guia de setup |
| [Comandos](uso/comandos.md) | Uso via CLI |
| [FAQ](referencias/faq.md) | Perguntas frequentes |

## Estrutura do Projeto

```
SO-SimuladorVonNeumann/
├── src/
│   ├── cpu/          # CPU, cores, escalonadores
│   ├── memory/       # RAM, cache, gerenciador
│   ├── IO/           # I/O manager
│   └── main.cpp      # Ponto de entrada
├── test/             # Testes automatizados
├── examples/         # Programas e processos exemplo
├── docs/             # Documentação
└── Makefile
```

## Compilação

```bash
make              # Compila simulador
make test         # Compila testes
make clean        # Limpa objetos
make run          # Compila e executa
```

## Licença

Este projeto foi desenvolvido para fins acadêmicos.

---

📚 [Documentação Completa](guia/introducao.md) | 🐛 [Troubleshooting](uso/troubleshooting.md) | ❓ [FAQ](referencias/faq.md)
