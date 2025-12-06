# Interface Gráfica (GUI)

## Visão Geral

O projeto inclui uma interface gráfica desenvolvida em Python com PyQt5 para monitoramento, compilação e visualização de métricas do simulador.

> **Última atualização:** 06/12/2025

## Requisitos

### Dependências

| Dependência | Versão Mínima | Descrição |
|-------------|---------------|-----------|
| Python | 3.8+ | Interpretador |
| PyQt5 | 5.15.0+ | Framework de interface gráfica |
| matplotlib | 3.0+ | Geração de gráficos |
| pandas | 1.0+ | Manipulação de dados |

### Instalação das Dependências

**Ubuntu/Debian (via apt):**
```bash
sudo apt install -y python3-pyqt5 python3-matplotlib python3-pandas
```

**Via pip:**
```bash
cd gui
pip install -r requirements.txt
```

## Como Executar

```bash
# A partir da raiz do projeto
cd SO-SimuladorVonNeumann
python3 gui/monitor_v2.py
```

## Arquitetura da GUI

### Estrutura de Arquivos

```
gui/
├── monitor_v2.py      # Interface principal (880+ linhas)
├── requirements.txt   # Dependências Python
└── README.md          # Documentação da GUI
```

### Componentes Principais

```
┌─────────────────────────────────────────────────────────────────┐
│                    SO Monitor V2 - GUI                         │
├─────────────────────────────────────────────────────────────────┤
│  [Compile] [Run Tests] [Update Data] [Save Chart] [Export CSV] │
├──────────────────────┬──────────────────────────────────────────┤
│  Painel de Controles │         Área de Gráficos               │
│  ┌────────────────┐  │  ┌────────────────────────────────────┐ │
│  │ Seleção de     │  │  │                                    │ │
│  │ Políticas:     │  │  │     [Gráfico Matplotlib]          │ │
│  │ ☑ RR           │  │  │                                    │ │
│  │ ☑ FCFS         │  │  │     Métricas vs Cores             │ │
│  │ ☑ SJN          │  │  │     por Política                  │ │
│  │ ☑ PRIORITY     │  │  │                                    │ │
│  └────────────────┘  │  └────────────────────────────────────┘ │
│  ┌────────────────┐  │  ┌────────────────────────────────────┐ │
│  │ Eixo X: [▼]    │  │  │     Console de Saída              │ │
│  │ Eixo Y: [▼]    │  │  │                                    │ │
│  │ Tipo:  [▼]     │  │  │     Logs de compilação,           │ │
│  │ [Plot]         │  │  │     execução e eventos            │ │
│  └────────────────┘  │  └────────────────────────────────────┘ │
└──────────────────────┴──────────────────────────────────────────┘
```

## Funcionalidades

### 🔧 Compilação e Execução

| Botão | Ação | Comando Executado |
|-------|------|-------------------|
| **Compile** | Compila o simulador | `make simulador` |
| **Run Tests** | Executa teste de métricas | `./bin/test_metrics` |
| **Update Data** | Recarrega CSVs de métricas | Leitura de `dados_graficos/csv/` |

### 📊 Seleção de Políticas

A GUI permite selecionar quais políticas de escalonamento incluir nos gráficos:

| Política | Cor | Marcador |
|----------|-----|----------|
| RR (Round Robin) | Verde (#2ecc71) | ○ |
| FCFS | Azul (#3498db) | □ |
| SJN | Vermelho (#e74c3c) | △ |
| PRIORITY | Roxo (#9b59b6) | ◇ |

### 📈 Tipos de Gráficos

| Tipo | Uso Recomendado |
|------|-----------------|
| **Line** | Comparar evolução por número de cores |
| **Bar** | Comparar políticas lado a lado |
| **Scatter** | Correlação entre duas métricas |

### 📋 Métricas Disponíveis

As métricas disponíveis para os eixos X e Y são:

| Métrica | Nome no CSV | Descrição |
|---------|-------------|-----------|
| Cores | `Cores` | Número de núcleos (1, 2, 4, 6) |
| Politica | `Politica` | Nome da política (categórico) |
| Throughput | `Throughput_proc_s` | Processos por segundo |
| Tempo de Espera | `TempoMedioEspera_ms` | Média em milissegundos |
| Turnaround | `TempoMedioTurnaround_ms` | Tempo total médio (ms) |
| Tempo de Execução | `TempoMedioExecucao_us` | Tempo médio em µs |
| Utilização CPU | `CPUUtilizacao_pct` | Percentual de uso |
| Eficiência | `Eficiencia_pct` | Eficiência do escalonamento |
| Cache Hits | `CacheHits` | Acertos de cache |
| Cache Misses | `CacheMisses` | Falhas de cache |
| Taxa de Hit | `TaxaHit_pct` | Percentual de acertos |

## Workflow Típico

### 1. Executar Testes e Gerar Dados

```bash
# Via terminal (recomendado para primeira execução)
make test-metrics
```

Ou via GUI:
1. Clique em **Compile** para compilar o projeto
2. Clique em **Run Tests** para executar os testes de métricas

### 2. Carregar e Visualizar Dados

1. Clique em **Update Data** para carregar os CSVs
2. Selecione as políticas que deseja comparar
3. Escolha o eixo X (geralmente `Cores`)
4. Escolha o eixo Y (métrica de interesse)
5. Selecione o tipo de gráfico
6. Clique em **Plot**

### 3. Exportar Resultados

- **Save Chart**: Salva o gráfico como PNG (300 DPI)
- **Export CSV**: Exporta os dados filtrados para CSV

## Exemplo de Uso

### Comparar Throughput por Número de Cores

1. Selecione todas as políticas (RR, FCFS, SJN, PRIORITY)
2. Eixo X: `Cores`
3. Eixo Y: `Throughput_proc_s`
4. Tipo: `Line`
5. Clique em **Plot**

**Resultado esperado:**
```
Throughput (proc/s)
     │
 100 │        ╱──────────○ RR
     │      ╱ ╱──────────□ FCFS
  80 │    ╱ ╱────────────△ SJN
     │  ╱╱───────────────◇ PRIORITY
  60 │╱╱
     │
     └────┬────┬────┬────┬────
          1    2    4    6   Cores
```

### Comparar Tempo de Espera por Política

1. Selecione todas as políticas
2. Eixo X: `Politica`
3. Eixo Y: `TempoMedioEspera_ms`
4. Tipo: `Bar`
5. Clique em **Plot**

## Fonte de Dados

A GUI lê os arquivos CSV gerados pelo teste de métricas:

```
dados_graficos/
├── csv/
│   ├── metricas_1cores.csv
│   ├── metricas_2cores.csv
│   ├── metricas_4cores.csv
│   └── metricas_6cores.csv
└── reports/
    └── relatorio_metricas_Xcores.txt
```

### Formato do CSV

```csv
Politica,Throughput_proc_s,TempoMedioEspera_ms,TempoMedioTurnaround_ms,...
FCFS,85.2,12.5,45.3,...
SJN,82.1,10.2,42.1,...
RR,78.5,15.3,48.2,...
PRIORITY,80.3,11.8,44.5,...
```

## Troubleshooting

### GUI não abre

```bash
# Verificar se PyQt5 está instalado
python3 -c "from PyQt5.QtWidgets import QApplication"

# Se der erro, instalar:
pip install PyQt5
```

### Gráficos não aparecem

```bash
# Verificar matplotlib
python3 -c "import matplotlib; print(matplotlib.__version__)"

# Verificar backend
python3 -c "import matplotlib; print(matplotlib.get_backend())"
```

### Dados não carregam

1. Verificar se os CSVs existem:
   ```bash
   ls dados_graficos/csv/
   ```

2. Se não existirem, executar testes:
   ```bash
   make test-metrics
   ```

### Erro de permissão

```bash
# Dar permissão de execução ao script
chmod +x gui/monitor_v2.py
```

## Próximas Implementações (Roadmap)

- [ ] Dashboard com múltiplos gráficos simultâneos
- [ ] Comparação lado-a-lado de políticas
- [ ] Filtros de tempo/range de dados
- [ ] Modo dark theme
- [ ] Exportação de relatórios em PDF
- [ ] Análise estatística (média, desvio padrão, percentis)
- [ ] Monitoramento em tempo real via socket

## Arquivos Relacionados

- [Métricas](../metricas/metricas.md) - Sistema de coleta de métricas
- [Testes](../metricas/testes.md) - Testes disponíveis
- [Comandos](comandos.md) - Comandos do simulador
