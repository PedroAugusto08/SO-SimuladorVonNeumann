# GUI Monitor para SO-SimuladorVonNeumann

Esta pasta contém uma interface gráfica avançada em Python para compilar o projeto, executar o simulador, atualizar dados existentes e criar gráficos dinâmicos a partir dos CSVs em `dados_graficos`.

## Requisitos

- Python 3.8+
- Instalar dependências via apt (Ubuntu/Debian):

```bash
sudo apt install -y python3-pyqt5 python3-matplotlib python3-pandas
```

Ou via pip (se não houver restrições):

```bash
cd gui
pip install -r requirements.txt
```

## Como usar

Para executar a interface:

```bash
cd /caminho/para/SO-SimuladorVonNeumann
python3 gui/monitor.py
```

## Funcionalidades principais

### 🔧 Compilação e Execução
- **Compile**: executa `make simulador` na raiz do repositório e exibe saída em tempo real
- **Run Simulator**: executa o simulador compilado (`bin/simulador`) e atualiza dados automaticamente
- **Update Data**: carrega/recarrega todos os CSVs da pasta `dados_graficos`

### 📊 Visualização de Métricas
- **Painel estilo "Afterburner"**: mostra valores principais detectados nas tabelas:
  - CPU usage/utilization
  - Memory/RAM usage
  - I/O operations
  - Throughput
  
### 📈 Gráficos Dinâmicos
- **Seleção de Eixos**: escolha colunas X e Y dos CSVs carregados
- **Tipos de Gráfico**: 
  - Line (linha com marcadores)
  - Bar (barras)
  - Scatter (dispersão)
- **Seleção de Dataset**: plote dados de um CSV específico ou combine todos
- **Grid e formatação**: gráficos profissionais com grid, labels e títulos

### 🔄 Auto-refresh
- **Checkbox Auto-refresh**: ativa atualização automática dos dados
- **Intervalo configurável**: de 1 a 3600 segundos
- Timer visual no console de saída

### 💾 Exportação
- **Save Chart**: salva o gráfico atual como PNG de alta resolução (300 DPI)
- **Export CSV**: exporta os dados do dataset selecionado ou combinado para CSV

### 📝 Console de Saída
- Exibe saída da compilação em tempo real
- Mostra resultado da execução do simulador
- Logs de auto-refresh e operações

## Exemplos de Uso

### Workflow típico:

1. Abra a GUI: `python3 gui/monitor.py`
2. Clique em **Compile** para compilar o projeto
3. Após compilação bem-sucedida, clique em **Run Simulator**
4. Os dados serão atualizados automaticamente
5. Selecione eixos X e Y nos dropdowns
6. Escolha o tipo de gráfico desejado
7. Clique em **Plot** para visualizar
8. Use **Save Chart** para exportar como imagem
9. Ative **Auto-refresh** para monitoramento contínuo

### Comparação de schedulers:

1. Selecione dataset específico (ex: `memoria_Round_Robin__Preemptivo_.csv`)
2. Eixo X: índice ou tempo
3. Eixo Y: métrica de interesse (throughput, latência, etc.)
4. Compare visualmente alterando o dataset

## Próximos passos sugeridos

- [ ] Integrar leitura direta das saídas do simulador em tempo real (socket ou pipe)
- [ ] Adicionar dashboards com múltiplos gráficos simultâneos
- [ ] Criar presets de visualização (CPU, Memória, I/O)
- [ ] Adicionar comparação lado-a-lado de múltiplos schedulers
- [ ] Exportar relatórios em PDF com gráficos e estatísticas
- [ ] Adicionar análise estatística (média, desvio padrão, percentis)
- [ ] Suporte a filtros de tempo/range de dados
- [ ] Modo dark theme

## Troubleshooting

### Erro: "ModuleNotFoundError: No module named 'PyQt5'"
Instale as dependências do sistema:
```bash
sudo apt install -y python3-pyqt5 python3-matplotlib python3-pandas
```

### Erro: "make finished with code 2"
Verifique se há erros de compilação no console de saída. Corrija os erros no código C++ e tente novamente.

### Simulador não encontrado
Certifique-se de compilar o projeto primeiro usando o botão **Compile**.
