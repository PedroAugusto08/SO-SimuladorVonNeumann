#!/usr/bin/env python3
"""
SO Monitor V2 - GUI Avançada para Visualização de Escalonadores
Permite criação de gráficos customizados com seleção de escalonadores,
métricas e tipos de visualização.
"""

import os
import sys
import traceback
import numpy as np

from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QLabel,
    QComboBox, QTextEdit, QCheckBox, QSpinBox, QFileDialog, QMessageBox,
    QGroupBox, QGridLayout, QSplitter, QTabWidget, QListWidget, QListWidgetItem,
    QSlider, QFrame, QScrollArea
)
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt5.QtGui import QFont

import pandas as pd
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure
import matplotlib.pyplot as plt

# ==============================================================================
# CONFIGURAÇÃO DE CORES E ESTILOS
# ==============================================================================

CORES_POLITICAS = {
    'RR': '#2ecc71',           # Verde
    'FCFS': '#3498db',         # Azul
    'SJN': '#e74c3c',          # Vermelho
    'PRIORITY': '#9b59b6',     # Roxo
}

MARCADORES = ['o', 's', '^', 'D', 'v', '<', '>', 'p']

NOMES_COMPLETOS = {
    'RR': 'Round Robin',
    'FCFS': 'First Come First Served',
    'SJN': 'Shortest Job Next',
    'PRIORITY': 'Priority'
}


# ==============================================================================
# GERENCIADOR DE DADOS UNIFICADO
# ==============================================================================

class DataManager:
    """Carrega e unifica todos os CSVs de métricas por cores (1, 2, 4, 6)"""
    
    def __init__(self, dados_dir):
        self.dados_dir = dados_dir
        self.df_metricas_cores = None  # DataFrame unificado com todos os cores
        self.configs_cores = [1, 2, 4, 6]
        
    def carregar_todos(self):
        """Carrega todos os CSVs de métricas por cores"""
        self._carregar_metricas_por_cores()
        return self
    
    def _carregar_metricas_por_cores(self):
        """Carrega metricas_Xcores.csv para X = 1, 2, 4, 6"""
        dados = []
        
        for num_cores in self.configs_cores:
            # Tentar diferentes caminhos
            caminhos = [
                os.path.join(self.dados_dir, 'csv', f'metricas_{num_cores}cores.csv'),
                os.path.join(self.dados_dir, f'metricas_{num_cores}cores.csv'),
            ]
            
            for path in caminhos:
                if os.path.isfile(path):
                    try:
                        df = pd.read_csv(path)
                        df['Cores'] = num_cores
                        dados.append(df)
                        print(f"✅ Carregado: metricas_{num_cores}cores.csv ({len(df)} políticas)")
                        break
                    except Exception as e:
                        print(f"Erro ao carregar {path}: {e}")
        
        if dados:
            self.df_metricas_cores = pd.concat(dados, ignore_index=True)
            print(f"📊 Total: {len(self.df_metricas_cores)} registros carregados")
        else:
            print("⚠️ Nenhum CSV de métricas encontrado!")
            self.df_metricas_cores = pd.DataFrame()
            
    def get_politicas_disponiveis(self):
        """Retorna lista de políticas disponíveis"""
        if self.df_metricas_cores is not None and not self.df_metricas_cores.empty:
            return sorted(self.df_metricas_cores['Politica'].unique().tolist())
        return ['RR', 'FCFS', 'SJN', 'PRIORITY']
    
    def get_cores_disponiveis(self):
        """Retorna lista de configurações de cores disponíveis"""
        if self.df_metricas_cores is not None and not self.df_metricas_cores.empty:
            return sorted(self.df_metricas_cores['Cores'].unique().tolist())
        return self.configs_cores
    
    def get_todas_metricas(self):
        """Retorna TODAS as métricas disponíveis para eixos X e Y"""
        metricas = [
            ('Politica', 'Escalonador (categórico)'),
            ('Cores', 'Número de Cores'),
        ]
        
        # Métricas do CSV de métricas por cores
        metricas.extend([
            ('Throughput_proc_s', 'Throughput (proc/s)'),
            ('TempoMedioEspera_ms', 'Tempo Médio de Espera (ms)'),
            ('TempoMedioTurnaround_ms', 'Tempo Médio de Retorno (ms)'),
            ('TempoMedioExecucao_us', 'Tempo Médio de Execução (µs)'),
            ('CPUUtilizacao_pct', 'Utilização de CPU (%)'),
            ('Eficiencia_pct', 'Eficiência (%)'),
            ('CacheHits', 'Cache Hits'),
            ('CacheMisses', 'Cache Misses'),
            ('TaxaHit_pct', 'Taxa de Cache Hit (%)'),
        ])
            
        return metricas
    
    def get_metricas_disponiveis(self):
        """Retorna lista de métricas disponíveis para plotagem (compatibilidade)"""
        return self.get_todas_metricas()
    
    def get_dados_xy(self, metrica_x, metrica_y, politicas):
        """Retorna dados para gráfico X vs Y para as políticas selecionadas"""
        resultado = {}
        
        if self.df_metricas_cores is None or self.df_metricas_cores.empty:
            return resultado
        
        # Caso: X = Cores (gráfico de linha/barras por número de cores)
        if metrica_x == 'Cores':
            for pol in politicas:
                df_pol = self.df_metricas_cores[self.df_metricas_cores['Politica'] == pol]
                df_pol = df_pol.sort_values('Cores')
                
                if not df_pol.empty and metrica_y in df_pol.columns:
                    resultado[pol] = {
                        'x': df_pol['Cores'].values,
                        'y': df_pol[metrica_y].values
                    }
            return resultado
        
        # Caso: X = Politica (gráfico de barras comparativo)
        if metrica_x == 'Politica':
            # Usar dados de 4 cores como padrão para comparação
            df_4cores = self.df_metricas_cores[self.df_metricas_cores['Cores'] == 4]
            
            for pol in politicas:
                df_pol = df_4cores[df_4cores['Politica'] == pol]
                if not df_pol.empty and metrica_y in df_pol.columns:
                    resultado[pol] = {
                        'x': pol,
                        'y': df_pol[metrica_y].values[0]
                    }
            return resultado
        
        # Caso geral: X e Y são métricas numéricas (scatter)
        # Usar dados de 4 cores
        df_4cores = self.df_metricas_cores[self.df_metricas_cores['Cores'] == 4]
        
        for pol in politicas:
            df_pol = df_4cores[df_4cores['Politica'] == pol]
            if not df_pol.empty:
                if metrica_x in df_pol.columns and metrica_y in df_pol.columns:
                    val_x = df_pol[metrica_x].values[0]
                    val_y = df_pol[metrica_y].values[0]
                    resultado[pol] = {'x': val_x, 'y': val_y}
        
        return resultado
    
    def get_dados_para_grafico(self, metrica, politicas, cores=None):
        """Retorna dados formatados para plotagem (compatibilidade)"""
        return self.get_dados_xy('Cores', metrica, politicas)
    
    def get_resumo_dados(self):
        """Retorna resumo dos dados carregados"""
        if self.df_metricas_cores is None or self.df_metricas_cores.empty:
            return "Nenhum dado carregado"
        
        linhas = []
        linhas.append(f"📊 Total: {len(self.df_metricas_cores)} registros")
        
        cores_disp = self.get_cores_disponiveis()
        linhas.append(f"🔢 Cores: {', '.join(map(str, cores_disp))}")
        
        politicas = self.get_politicas_disponiveis()
        linhas.append(f"📋 Políticas: {', '.join(politicas)}")
        
        # Resumo por cores
        for nc in cores_disp:
            df_nc = self.df_metricas_cores[self.df_metricas_cores['Cores'] == nc]
            if not df_nc.empty:
                linhas.append(f"\n--- {nc} Core(s) ---")
                for _, row in df_nc.iterrows():
                    linhas.append(f"  {row['Politica']}: {row['Throughput_proc_s']:.0f} proc/s")
        
        return '\n'.join(linhas)


# ==============================================================================
# WORKERS PARA EXECUÇÃO EM BACKGROUND
# ==============================================================================

class TestWorker(QThread):
    """Worker para executar testes em background"""
    output = pyqtSignal(str)
    finished = pyqtSignal(int)

    def __init__(self, test_name, base_dir):
        super().__init__()
        self.test_name = test_name
        self.base_dir = base_dir

    def run(self):
        try:
            import subprocess
            test_path = os.path.join(self.base_dir, 'bin', self.test_name)
            
            if not os.path.isfile(test_path):
                self.output.emit(f"⏳ Compilando {self.test_name}...\n")
                make_proc = subprocess.Popen(
                    ["make", f"bin/{self.test_name}"],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    cwd=self.base_dir,
                    text=True
                )
                for line in make_proc.stdout:
                    self.output.emit(line)
                make_proc.wait()
                if make_proc.returncode != 0:
                    self.finished.emit(-1)
                    return
            
            self.output.emit(f"\n🚀 Executando {self.test_name}...\n")
            proc = subprocess.Popen(
                [test_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                cwd=self.base_dir,
                text=True
            )
            for line in proc.stdout:
                self.output.emit(line)
            proc.wait()
            self.finished.emit(proc.returncode)
        except Exception as e:
            self.output.emit(f"❌ Erro: {e}\n{traceback.format_exc()}")
            self.finished.emit(-1)


# ==============================================================================
# GUI PRINCIPAL
# ==============================================================================

class MonitorGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('SO Monitor V2 - Visualização de Escalonadores')
        self.resize(1400, 900)
        
        # Diretórios
        self.base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.dados_dir = os.path.join(self.base_dir, 'dados_graficos')
        
        # Gerenciador de dados
        self.data_manager = DataManager(self.dados_dir)
        
        # Workers
        self.test_worker = None
        
        # Construir interface
        self._construir_ui()
        
        # Carregar dados iniciais
        self.carregar_dados()
        
    def _construir_ui(self):
        """Constrói toda a interface"""
        layout_principal = QVBoxLayout()
        
        # === BARRA DE FERRAMENTAS ===
        toolbar = self._criar_toolbar()
        layout_principal.addLayout(toolbar)
        
        # === ÁREA PRINCIPAL (Splitter) ===
        splitter = QSplitter(Qt.Horizontal)
        
        # Painel esquerdo: Controles
        painel_controles = self._criar_painel_controles()
        splitter.addWidget(painel_controles)
        
        # Painel direito: Gráfico e Console
        painel_grafico = self._criar_painel_grafico()
        splitter.addWidget(painel_grafico)
        
        splitter.setSizes([350, 1050])
        layout_principal.addWidget(splitter)
        
        self.setLayout(layout_principal)
        
    def _criar_toolbar(self):
        """Cria barra de ferramentas superior"""
        layout = QHBoxLayout()
        
        self.btn_atualizar = QPushButton('🔄 Atualizar Dados')
        self.btn_atualizar.clicked.connect(self.carregar_dados)
        
        self.btn_gerar_metricas = QPushButton('📊 Gerar Métricas (4 cores)')
        self.btn_gerar_metricas.clicked.connect(lambda: self.executar_teste('test_metrics'))
        self.btn_gerar_metricas.setStyleSheet('background-color: #27ae60; color: white; font-weight: bold;')
        self.btn_gerar_metricas.setToolTip('Executa teste de métricas com configuração atual de cores')
        
        self.btn_gerar_todos = QPushButton('🎯 Gerar Todos (1,2,4,6 cores)')
        self.btn_gerar_todos.clicked.connect(self.executar_todos_testes_cores)
        self.btn_gerar_todos.setStyleSheet('background-color: #3498db; color: white; font-weight: bold;')
        self.btn_gerar_todos.setToolTip('Executa testes para 1, 2, 4 e 6 cores')
        
        self.btn_abrir_graficos = QPushButton('📈 Gerar Gráficos Python')
        self.btn_abrir_graficos.clicked.connect(self.executar_script_graficos)
        
        layout.addWidget(self.btn_atualizar)
        layout.addWidget(self.btn_gerar_metricas)
        layout.addWidget(self.btn_gerar_todos)
        layout.addWidget(self.btn_abrir_graficos)
        layout.addStretch()
        
        # Status
        self.lbl_status = QLabel('Pronto')
        layout.addWidget(self.lbl_status)
        
        return layout
        
    def _criar_painel_controles(self):
        """Cria painel de controles à esquerda"""
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # === SELEÇÃO DE ESCALONADORES ===
        grupo_escalonadores = QGroupBox('📋 Escalonadores')
        layout_esc = QVBoxLayout()
        
        self.chk_escalonadores = {}
        for key, cor in CORES_POLITICAS.items():
            chk = QCheckBox(f'{key} - {NOMES_COMPLETOS[key]}')
            chk.setChecked(True)
            chk.setStyleSheet(f'color: {cor}; font-weight: bold;')
            chk.stateChanged.connect(self.atualizar_preview)
            self.chk_escalonadores[key] = chk
            layout_esc.addWidget(chk)
        
        # Botões de seleção rápida
        layout_sel = QHBoxLayout()
        btn_todos = QPushButton('Todos')
        btn_todos.clicked.connect(lambda: self._selecionar_escalonadores(True))
        btn_nenhum = QPushButton('Nenhum')
        btn_nenhum.clicked.connect(lambda: self._selecionar_escalonadores(False))
        layout_sel.addWidget(btn_todos)
        layout_sel.addWidget(btn_nenhum)
        layout_esc.addLayout(layout_sel)
        
        grupo_escalonadores.setLayout(layout_esc)
        layout.addWidget(grupo_escalonadores)
        
        # === SELEÇÃO DE EIXO X ===
        grupo_eixo_x = QGroupBox('📊 Eixo X (Horizontal)')
        layout_x = QVBoxLayout()
        
        self.combo_eixo_x = QComboBox()
        self.combo_eixo_x.setMaxVisibleItems(20)
        self.combo_eixo_x.currentIndexChanged.connect(self.atualizar_preview)
        layout_x.addWidget(self.combo_eixo_x)
        
        grupo_eixo_x.setLayout(layout_x)
        layout.addWidget(grupo_eixo_x)
        
        # === SELEÇÃO DE EIXO Y ===
        grupo_eixo_y = QGroupBox('📈 Eixo Y (Vertical)')
        layout_y = QVBoxLayout()
        
        self.combo_eixo_y = QComboBox()
        self.combo_eixo_y.setMaxVisibleItems(20)
        self.combo_eixo_y.currentIndexChanged.connect(self.atualizar_preview)
        layout_y.addWidget(self.combo_eixo_y)
        
        grupo_eixo_y.setLayout(layout_y)
        layout.addWidget(grupo_eixo_y)
        
        # === TIPO DE GRÁFICO ===
        grupo_tipo = QGroupBox('🎨 Tipo de Gráfico')
        layout_tipo = QVBoxLayout()
        
        self.combo_tipo = QComboBox()
        self.combo_tipo.addItems([
            'Scatter (pontos)',
            'Barras Comparativas',
            'Linha (se X = Cores)',
            'Barras Agrupadas (se X = Cores)',
        ])
        self.combo_tipo.currentIndexChanged.connect(self.atualizar_preview)
        layout_tipo.addWidget(self.combo_tipo)
        
        # Checkbox para mostrar valores
        self.chk_mostrar_valores = QCheckBox('Mostrar valores no gráfico')
        self.chk_mostrar_valores.setChecked(True)
        layout_tipo.addWidget(self.chk_mostrar_valores)
        
        # Checkbox para mostrar legenda
        self.chk_mostrar_legenda = QCheckBox('Mostrar legenda')
        self.chk_mostrar_legenda.setChecked(True)
        layout_tipo.addWidget(self.chk_mostrar_legenda)
        
        grupo_tipo.setLayout(layout_tipo)
        layout.addWidget(grupo_tipo)
        
        # === BOTÕES DE AÇÃO ===
        grupo_acoes = QGroupBox('⚡ Ações')
        layout_acoes = QVBoxLayout()
        
        self.btn_plotar = QPushButton('📊 Plotar Gráfico')
        self.btn_plotar.setStyleSheet('background-color: #3498db; color: white; font-weight: bold; padding: 10px;')
        self.btn_plotar.clicked.connect(self.plotar_grafico)
        layout_acoes.addWidget(self.btn_plotar)
        
        layout_export = QHBoxLayout()
        self.btn_salvar_png = QPushButton('💾 PNG')
        self.btn_salvar_png.clicked.connect(lambda: self.salvar_grafico('png'))
        self.btn_salvar_pdf = QPushButton('📄 PDF')
        self.btn_salvar_pdf.clicked.connect(lambda: self.salvar_grafico('pdf'))
        layout_export.addWidget(self.btn_salvar_png)
        layout_export.addWidget(self.btn_salvar_pdf)
        layout_acoes.addLayout(layout_export)
        
        grupo_acoes.setLayout(layout_acoes)
        layout.addWidget(grupo_acoes)
        
        # === RESUMO DE DADOS ===
        grupo_resumo = QGroupBox('📋 Resumo dos Dados')
        layout_resumo = QVBoxLayout()
        
        self.lbl_resumo = QLabel('Carregando...')
        self.lbl_resumo.setWordWrap(True)
        layout_resumo.addWidget(self.lbl_resumo)
        
        grupo_resumo.setLayout(layout_resumo)
        layout.addWidget(grupo_resumo)
        
        layout.addStretch()
        return widget
        
    def _criar_painel_grafico(self):
        """Cria painel de gráfico à direita"""
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Canvas Matplotlib
        self.fig = Figure(figsize=(10, 7), dpi=100)
        self.canvas = FigureCanvas(self.fig)
        
        # Toolbar de navegação
        self.toolbar = NavigationToolbar(self.canvas, widget)
        
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas, stretch=4)
        
        # Console de saída
        self.console = QTextEdit()
        self.console.setReadOnly(True)
        self.console.setMaximumHeight(150)
        self.console.setStyleSheet('background-color: #2c3e50; color: #ecf0f1; font-family: monospace;')
        layout.addWidget(self.console, stretch=1)
        
        return widget
        
    def _selecionar_escalonadores(self, selecionar):
        """Seleciona ou deseleciona todos os escalonadores"""
        for chk in self.chk_escalonadores.values():
            chk.setChecked(selecionar)
            
    def carregar_dados(self):
        """Carrega todos os dados dos CSVs"""
        self.lbl_status.setText('Carregando dados...')
        self.data_manager.carregar_todos()
        
        # Atualizar combos de eixos X e Y
        metricas = self.data_manager.get_todas_metricas()
        
        self.combo_eixo_x.clear()
        self.combo_eixo_y.clear()
        
        for key, nome in metricas:
            self.combo_eixo_x.addItem(nome, key)
            self.combo_eixo_y.addItem(nome, key)
        
        # Defaults: X = Escalonador, Y = Tempo
        idx_x = self.combo_eixo_x.findData('Politica')
        idx_y = self.combo_eixo_y.findData('Throughput_proc_s')
        if idx_x >= 0:
            self.combo_eixo_x.setCurrentIndex(idx_x)
        if idx_y >= 0:
            self.combo_eixo_y.setCurrentIndex(idx_y)
        
        # Atualizar resumo
        self._atualizar_resumo()
        
        self.lbl_status.setText('Dados carregados')
        self.console.append('✅ Dados carregados com sucesso\n')
        
    def _atualizar_resumo(self):
        """Atualiza o resumo de dados carregados"""
        dm = self.data_manager
        resumo = dm.get_resumo_dados()
        self.lbl_resumo.setText(resumo)
        
    def get_escalonadores_selecionados(self):
        """Retorna lista de escalonadores selecionados"""
        return [k for k, chk in self.chk_escalonadores.items() if chk.isChecked()]
        
    def atualizar_preview(self):
        """Atualiza preview do gráfico (pode ser chamado automaticamente)"""
        pass  # Opcional: auto-atualizar
        
    def plotar_grafico(self):
        """Plota o gráfico com as configurações atuais"""
        try:
            self.fig.clear()
            ax = self.fig.add_subplot(111)
            
            # Obter configurações
            escalonadores = self.get_escalonadores_selecionados()
            metrica_x_key = self.combo_eixo_x.currentData()
            metrica_x_nome = self.combo_eixo_x.currentText()
            metrica_y_key = self.combo_eixo_y.currentData()
            metrica_y_nome = self.combo_eixo_y.currentText()
            tipo_grafico = self.combo_tipo.currentText()
            mostrar_valores = self.chk_mostrar_valores.isChecked()
            mostrar_legenda = self.chk_mostrar_legenda.isChecked()
            
            if not escalonadores:
                ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', 
                       ha='center', va='center', fontsize=14)
                self.canvas.draw()
                return
                
            if not metrica_x_key or not metrica_y_key:
                ax.text(0.5, 0.5, 'Selecione métricas para X e Y', 
                       ha='center', va='center', fontsize=14)
                self.canvas.draw()
                return
            
            # Obter dados
            dados = self.data_manager.get_dados_xy(metrica_x_key, metrica_y_key, escalonadores)
            
            if not dados:
                ax.text(0.5, 0.5, 'Sem dados para exibir.\nExecute os testes primeiro.', 
                       ha='center', va='center', fontsize=14)
                self.canvas.draw()
                return
            
            # Plotar conforme o tipo e os dados
            if metrica_x_key == 'Politica':
                # X é categórico - usar barras
                self._plotar_barras_categorico(ax, dados, metrica_y_nome, mostrar_valores)
            elif metrica_x_key == 'Cores':
                # X é Cores - pode ser linha ou barras
                if 'Linha' in tipo_grafico:
                    self._plotar_linha_cores(ax, dados, metrica_y_nome)
                elif 'Agrupadas' in tipo_grafico:
                    self._plotar_barras_agrupadas_cores(ax, dados, metrica_y_nome, mostrar_valores)
                else:
                    self._plotar_scatter_xy(ax, dados, metrica_x_nome, metrica_y_nome, mostrar_valores)
            else:
                # X e Y são métricas numéricas - scatter
                if 'Barras' in tipo_grafico:
                    self._plotar_barras_categorico(ax, dados, metrica_y_nome, mostrar_valores)
                else:
                    self._plotar_scatter_xy(ax, dados, metrica_x_nome, metrica_y_nome, mostrar_valores)
            
            if mostrar_legenda:
                ax.legend(loc='best')
            
            ax.set_title(f'{metrica_y_nome} vs {metrica_x_nome}', fontweight='bold', fontsize=12)
            self.fig.tight_layout()
            self.canvas.draw()
            self.console.append(f'✅ Gráfico: {metrica_y_nome} vs {metrica_x_nome}\n')
            
        except Exception as e:
            self.console.append(f'❌ Erro ao plotar: {e}\n')
            traceback.print_exc()
    
    def _plotar_barras_categorico(self, ax, dados, metrica_y_nome, mostrar_valores):
        """Plota barras quando X é categórico (Politica)"""
        politicas = list(dados.keys())
        valores = [dados[p]['y'] for p in politicas]
        cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
        
        x_pos = np.arange(len(politicas))
        bars = ax.bar(x_pos, valores, color=cores, edgecolor='black', linewidth=0.5)
        
        if mostrar_valores:
            for bar, val in zip(bars, valores):
                ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                       f'{val:.2f}', ha='center', va='bottom', fontweight='bold', fontsize=9)
        
        ax.set_xticks(x_pos)
        ax.set_xticklabels([f'{p}\n{NOMES_COMPLETOS.get(p, p)}' for p in politicas], fontsize=9)
        ax.set_xlabel('Escalonador', fontweight='bold')
        ax.set_ylabel(metrica_y_nome, fontweight='bold')
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)
        
        # Adicionar legendas coloridas
        for p in politicas:
            ax.bar([], [], color=CORES_POLITICAS.get(p, '#95a5a6'), label=p)
    
    def _plotar_linha_cores(self, ax, dados, metrica_y_nome):
        """Plota linha quando X é Cores"""
        for i, (pol, info) in enumerate(dados.items()):
            if 'x' in info and 'y' in info:
                cor = CORES_POLITICAS.get(pol, '#95a5a6')
                ax.plot(info['x'], info['y'], 
                       marker=MARCADORES[i % len(MARCADORES)],
                       color=cor, label=f'{pol} ({NOMES_COMPLETOS.get(pol, pol)})', 
                       linewidth=2, markersize=8)
        
        ax.set_xlabel('Número de Cores', fontweight='bold')
        ax.set_ylabel(metrica_y_nome, fontweight='bold')
        ax.grid(True, linestyle='--', alpha=0.5)
    
    def _plotar_barras_agrupadas_cores(self, ax, dados, metrica_y_nome, mostrar_valores):
        """Plota barras agrupadas por cores"""
        politicas = list(dados.keys())
        
        # Coletar todos os valores de cores disponíveis
        todos_cores = set()
        for info in dados.values():
            if 'x' in info:
                todos_cores.update(info['x'])
        cores_disp = sorted(todos_cores)
        
        if not cores_disp:
            return
        
        x = np.arange(len(cores_disp))
        width = 0.8 / len(politicas)
        offset = (len(politicas) - 1) * width / 2
        
        for i, pol in enumerate(politicas):
            info = dados[pol]
            valores = []
            for c in cores_disp:
                if 'x' in info:
                    idx = np.where(np.array(info['x']) == c)[0]
                    valores.append(info['y'][idx[0]] if len(idx) > 0 else 0)
                else:
                    valores.append(0)
            
            cor = CORES_POLITICAS.get(pol, '#95a5a6')
            bars = ax.bar(x + i * width - offset, valores, width, 
                         label=f'{pol}', color=cor, edgecolor='black', linewidth=0.5)
            
            if mostrar_valores:
                for bar, val in zip(bars, valores):
                    if val > 0:
                        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                               f'{val:.1f}', ha='center', va='bottom', fontsize=7, rotation=45)
        
        ax.set_xlabel('Número de Cores', fontweight='bold')
        ax.set_ylabel(metrica_y_nome, fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels([f'{c} core(s)' for c in cores_disp])
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    def _plotar_scatter_xy(self, ax, dados, metrica_x_nome, metrica_y_nome, mostrar_valores):
        """Plota scatter para X e Y numéricos"""
        for i, (pol, info) in enumerate(dados.items()):
            cor = CORES_POLITICAS.get(pol, '#95a5a6')
            
            if 'x' in info and 'y' in info:
                x_vals = info['x']
                y_vals = info['y']
                
                # Verificar se são arrays ou valores únicos
                is_array_x = hasattr(x_vals, '__iter__') and not isinstance(x_vals, (str, float, int, np.floating))
                is_array_y = hasattr(y_vals, '__iter__') and not isinstance(y_vals, (str, float, int, np.floating))
                
                if is_array_x and is_array_y:
                    # Dados de array (ex: Cores vs métrica)
                    ax.scatter(x_vals, y_vals, color=cor, s=150, alpha=0.8,
                              marker=MARCADORES[i % len(MARCADORES)],
                              label=f'{pol} ({NOMES_COMPLETOS.get(pol, pol)})',
                              edgecolors='black', linewidths=0.5)
                    
                    if mostrar_valores:
                        for x, y in zip(x_vals, y_vals):
                            ax.annotate(f'{pol}', (x, y), textcoords="offset points", 
                                       xytext=(5, 5), fontsize=8)
                else:
                    # Dados escalares
                    x_val = float(x_vals) if not is_array_x else x_vals[0]
                    y_val = float(y_vals) if not is_array_y else y_vals[0]
                    ax.scatter([x_val], [y_val], color=cor, s=200, alpha=0.8,
                              marker=MARCADORES[i % len(MARCADORES)],
                              label=f'{pol} ({NOMES_COMPLETOS.get(pol, pol)})',
                              edgecolors='black', linewidths=1)
                    
                    if mostrar_valores:
                        ax.annotate(f'{pol}\n({y_val:.2f})', (x_val, y_val), 
                                   textcoords="offset points", xytext=(10, 5), 
                                   fontsize=9, fontweight='bold')
            else:
                # Dado sem x ou y
                x_val = info.get('x', i)
                y_val = info.get('y', 0)
                if isinstance(x_val, str):
                    x_val = i  # Usar índice se X for string
                ax.scatter([x_val], [y_val], color=cor, s=200, alpha=0.8,
                          marker=MARCADORES[i % len(MARCADORES)],
                          label=f'{pol} ({NOMES_COMPLETOS.get(pol, pol)})',
                          edgecolors='black', linewidths=1)
                
                if mostrar_valores:
                    ax.annotate(f'{pol}\n({y_val:.2f})', (x_val, y_val), 
                               textcoords="offset points", xytext=(10, 5), 
                               fontsize=9, fontweight='bold')
        
        ax.set_xlabel(metrica_x_nome, fontweight='bold')
        ax.set_ylabel(metrica_y_nome, fontweight='bold')
        ax.grid(True, linestyle='--', alpha=0.5)
        
    def salvar_grafico(self, formato):
        """Salva o gráfico atual"""
        path, _ = QFileDialog.getSaveFileName(
            self, f'Salvar como {formato.upper()}',
            os.path.join(self.base_dir, f'grafico_custom.{formato}'),
            f'{formato.upper()} Files (*.{formato});;All Files (*)'
        )
        if path:
            self.fig.savefig(path, dpi=150, bbox_inches='tight')
            self.console.append(f'✅ Gráfico salvo: {path}\n')
            
    def executar_teste(self, nome_teste):
        """Executa um teste específico"""
        if self.test_worker and self.test_worker.isRunning():
            QMessageBox.warning(self, 'Teste em execução', 
                              'Aguarde o teste atual terminar.')
            return
            
        self.lbl_status.setText(f'Executando {nome_teste}...')
        self.console.append(f'\n🚀 Iniciando {nome_teste}...\n')
        
        self.test_worker = TestWorker(nome_teste, self.base_dir)
        self.test_worker.output.connect(self.console.append)
        self.test_worker.finished.connect(self._on_teste_finalizado)
        self.test_worker.start()
    
    def executar_todos_testes_cores(self):
        """Executa testes para todas as configurações de cores (1, 2, 4, 6)"""
        self.console.append('\n🎯 Executando testes para 1, 2, 4 e 6 cores...\n')
        self.console.append('   Isso executará test_metrics 4 vezes com diferentes configurações.\n')
        self.console.append('   Os CSVs serão gerados em dados_graficos/csv/\n\n')
        
        # Executar via shell script para alterar DEFAULT_NUM_CORES
        import subprocess
        
        self.lbl_status.setText('Executando testes para todos os cores...')
        
        for num_cores in [1, 2, 4, 6]:
            self.console.append(f'═══ Executando com {num_cores} core(s) ═══\n')
            
            # Alterar DEFAULT_NUM_CORES no arquivo
            sed_cmd = f"sed -i 's/constexpr int DEFAULT_NUM_CORES = [0-9]*/constexpr int DEFAULT_NUM_CORES = {num_cores}/' test/test_metrics.cpp"
            subprocess.run(sed_cmd, shell=True, cwd=self.base_dir)
            
            # Compilar e executar
            result = subprocess.run(
                ['make', 'test-metrics'],
                capture_output=True,
                text=True,
                cwd=self.base_dir
            )
            
            if result.returncode == 0:
                self.console.append(f'✅ {num_cores} core(s): OK\n')
            else:
                self.console.append(f'❌ {num_cores} core(s): Erro\n')
                self.console.append(result.stderr[:500] + '\n')
        
        # Restaurar para 4 cores
        sed_cmd = "sed -i 's/constexpr int DEFAULT_NUM_CORES = [0-9]*/constexpr int DEFAULT_NUM_CORES = 4/' test/test_metrics.cpp"
        subprocess.run(sed_cmd, shell=True, cwd=self.base_dir)
        
        self.console.append('\n✅ Todos os testes concluídos!\n')
        self.console.append('   CSVs gerados: metricas_1cores.csv, metricas_2cores.csv, metricas_4cores.csv, metricas_6cores.csv\n')
        self.lbl_status.setText('Pronto')
        
        # Recarregar dados
        self.carregar_dados()
    
    def executar_script_graficos(self):
        """Executa o script Python de geração de gráficos"""
        import subprocess
        
        self.console.append('\n📈 Executando gerar_graficos_metricas.py...\n')
        self.lbl_status.setText('Gerando gráficos...')
        
        script_path = os.path.join(self.dados_dir, 'gerar_graficos_metricas.py')
        
        if not os.path.exists(script_path):
            self.console.append(f'❌ Script não encontrado: {script_path}\n')
            return
        
        result = subprocess.run(
            ['python3', script_path],
            capture_output=True,
            text=True,
            cwd=self.dados_dir
        )
        
        self.console.append(result.stdout)
        if result.stderr:
            self.console.append(result.stderr)
        
        if result.returncode == 0:
            self.console.append('\n✅ Gráficos gerados em dados_graficos/graficos/\n')
        else:
            self.console.append(f'\n❌ Erro ao gerar gráficos (código {result.returncode})\n')
        
        self.lbl_status.setText('Pronto')
        
    def executar_todos_testes(self):
        """Executa o teste unificado completo (compatibilidade)"""
        self.executar_todos_testes_cores()
        
    def _on_teste_finalizado(self, codigo):
        """Callback quando um teste termina"""
        if codigo == 0:
            self.console.append('✅ Teste concluído com sucesso!\n')
        else:
            self.console.append(f'⚠️ Teste finalizado com código {codigo}\n')
        
        self.lbl_status.setText('Pronto')
        
        # Executar próximo teste se houver
        if hasattr(self, '_proximo_teste') and self._proximo_teste:
            proximo = self._proximo_teste
            self._proximo_teste = None
            self.executar_teste(proximo)
        else:
            # Recarregar dados
            self.carregar_dados()


# ==============================================================================
# MAIN
# ==============================================================================

def main():
    app = QApplication(sys.argv)
    
    # Estilo
    app.setStyle('Fusion')
    
    gui = MonitorGUI()
    gui.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
