# Simulador Von Neumann Multicore

## Visão Geral

Simulador de sistema operacional multi-core baseado na arquitetura Von Neumann, implementado em C++17 com suporte a múltiplas políticas de escalonamento e hierarquia de memória com cache.

> **Última atualização:** 06/12/2025

## Características

| Componente | Descrição |
|------------|-----------|
| **Núcleos** | 1-8 núcleos configuráveis |
| **Escalonadores** | FCFS, SJN, Round Robin, Priority |
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

# Compilar simulador
make simulador

# Executar teste de métricas
make test-metrics

# Executar simulador
./bin/simulador --policy FCFS --cores 2 \
    -p examples/programs/tasks.json examples/processes/process1.json
```

## Uso Básico

```bash
# FCFS com 2 núcleos
./bin/simulador --policy FCFS --cores 2 -p tasks.json process.json

# Round Robin com quantum de 100 ciclos
./bin/simulador --policy RR --cores 4 --quantum 100 -p tasks.json process.json

# SJN (Shortest Job Next)
./bin/simulador --policy SJN --cores 2 -p tasks.json process.json

# Priority com cache LRU
./bin/simulador --policy PRIORITY --cores 4 --cache-policy LRU -p tasks.json process.json
```

## Comandos Make Principais

```bash
make simulador       # Compila simulador multicore
make run-sim         # Executa simulador
make test-metrics    # Teste de métricas (FCFS/SJN/Priority)
make test-single-core # Teste single-core determinístico
make check           # Verificação rápida
make help            # Lista todos os comandos
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
| [Interface Gráfica](uso/gui.md) | Visualização de métricas |
| [FAQ](referencias/faq.md) | Perguntas frequentes |

## Interface Gráfica (GUI)

O projeto inclui uma GUI em Python para visualização de métricas:

```bash
# Instalar dependências
pip install PyQt5 matplotlib pandas

# Executar GUI
python3 gui/monitor_v2.py
```

**Funcionalidades:**
- Compilação e execução de testes
- Gráficos comparativos de políticas
- Exportação de gráficos e dados

## Estrutura do Projeto

```
SO-SimuladorVonNeumann/
├── bin/              # Executáveis compilados
├── src/
│   ├── cpu/          # CPU, cores, escalonadores
│   ├── memory/       # RAM, cache, gerenciador
│   ├── IO/           # I/O manager
│   └── main.cpp      # Ponto de entrada
├── test/             # Testes (test_metrics, test_single_core_no_threads)
├── dados_graficos/   # Métricas CSV e relatórios
│   ├── csv/          # Arquivos CSV
│   └── reports/      # Relatórios texto
├── processes/        # Arquivos de processo JSON
├── tasks/            # Arquivos de tarefas JSON
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
