#!/usr/bin/env python3
"""
SO Monitor V2 - GUI Avançada para Visualização de Escalonadores
Permite criação de gráficos customizados com seleção de escalonadores,
métricas e tipos de visualização.
"""

import os
import sys
import glob
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
    """Carrega e unifica todos os CSVs em um dataset consolidado"""
    
    def __init__(self, dados_dir):
        self.dados_dir = dados_dir
        self.df_multicore = None      # Dados de múltiplos cores
        self.df_metricas = None       # Métricas gerais
        self.df_unified = None        # CSV unificado completo
        self.dados_memoria = {}       # Dados de memória por política
        self.df_unificado = None      # Dataset unificado completo
        
    def carregar_todos(self):
        """Carrega todos os CSVs disponíveis"""
        self._carregar_unified()      # Novo: tenta carregar CSV unificado primeiro
        self._carregar_multicore()
        self._carregar_metricas()
        self._carregar_memoria()
        self._criar_dataset_unificado()
        return self
    
    def _carregar_unified(self):
        """Carrega unified_complete.csv (novo formato completo)"""
        path = os.path.join(self.dados_dir, 'unified_complete.csv')
        if os.path.isfile(path):
            try:
                self.df_unified = pd.read_csv(path)
                print(f"✅ Carregado unified_complete.csv com {len(self.df_unified)} registros")
            except Exception as e:
                print(f"Erro ao carregar unified: {e}")
        
    def _carregar_multicore(self):
        """Carrega escalonadores_multicore.csv"""
        path = os.path.join(self.dados_dir, 'escalonadores_multicore.csv')
        if os.path.isfile(path):
            try:
                self.df_multicore = pd.read_csv(path)
            except Exception as e:
                print(f"Erro ao carregar multicore: {e}")
                
    def _carregar_metricas(self):
        """Carrega metricas_escalonadores.csv"""
        path = os.path.join(self.dados_dir, 'metricas_escalonadores.csv')
        if os.path.isfile(path):
            try:
                df = pd.read_csv(path)
                # Normalizar nomes das políticas
                df['Politica_Key'] = df['Politica'].apply(self._extrair_politica)
                self.df_metricas = df
            except Exception as e:
                print(f"Erro ao carregar métricas: {e}")
                
    def _carregar_memoria(self):
        """Carrega arquivos memoria_*.csv"""
        arquivos = glob.glob(os.path.join(self.dados_dir, 'memoria_*.csv'))
        for f in arquivos:
            nome = os.path.basename(f)
            politica = self._extrair_politica(nome)
            try:
                df = pd.read_csv(f)
                self.dados_memoria[politica] = df
            except Exception:
                pass
                
    def _extrair_politica(self, nome):
        """Extrai nome da política a partir do nome do arquivo/coluna"""
        nome_upper = nome.upper()
        if 'FCFS' in nome_upper:
            return 'FCFS'
        elif 'SJN' in nome_upper:
            return 'SJN'
        elif 'ROUND' in nome_upper or nome_upper.startswith('RR'):
            return 'RR'
        elif 'PRIORITY' in nome_upper:
            return 'PRIORITY'
        return nome.split('_')[0]
        
    def _criar_dataset_unificado(self):
        """Cria um dataset unificado com TODAS as métricas por escalonador"""
        # Criar dicionário base por política
        dados_por_politica = {}
        
        # Inicializar com políticas conhecidas
        for pol in ['RR', 'FCFS', 'SJN', 'PRIORITY']:
            dados_por_politica[pol] = {
                'Politica': pol,
                # Multicore (média de todos os cores)
                'Tempo_ms': np.nan,
                'Speedup': np.nan,
                'Eficiencia_Pct': np.nan,
                'CV_Pct': np.nan,
                # Por número de cores
                'Tempo_1core': np.nan,
                'Tempo_2cores': np.nan,
                'Tempo_4cores': np.nan,
                'Tempo_6cores': np.nan,
                'Speedup_2cores': np.nan,
                'Speedup_4cores': np.nan,
                'Speedup_6cores': np.nan,
                'Eficiencia_2cores': np.nan,
                'Eficiencia_4cores': np.nan,
                'Eficiencia_6cores': np.nan,
                # Métricas gerais
                'Tempo_Espera_ms': np.nan,
                'Tempo_Execucao_ms': np.nan,
                'CPU_Utilizacao_Pct': np.nan,
                'Throughput_proc_s': np.nan,
                # Memória
                'Cache_Hits': np.nan,
                'Cache_Misses': np.nan,
                'Hit_Rate': np.nan,
            }
        
        # Preencher dados do multicore
        if self.df_multicore is not None:
            for pol in dados_por_politica.keys():
                df_pol = self.df_multicore[self.df_multicore['Politica'] == pol]
                if not df_pol.empty:
                    # Médias gerais
                    dados_por_politica[pol]['Tempo_ms'] = df_pol['Tempo_ms'].mean()
                    dados_por_politica[pol]['Speedup'] = df_pol['Speedup'].mean()
                    dados_por_politica[pol]['Eficiencia_Pct'] = df_pol['Eficiencia_Pct'].mean()
                    dados_por_politica[pol]['CV_Pct'] = df_pol['CV_Pct'].mean()
                    
                    # Por número de cores
                    for _, row in df_pol.iterrows():
                        cores = int(row['Cores'])
                        dados_por_politica[pol][f'Tempo_{cores}core{"s" if cores > 1 else ""}'] = row['Tempo_ms']
                        if cores > 1:
                            dados_por_politica[pol][f'Speedup_{cores}cores'] = row['Speedup']
                            dados_por_politica[pol][f'Eficiencia_{cores}cores'] = row['Eficiencia_Pct']
        
        # Preencher dados de métricas
        if self.df_metricas is not None:
            for _, row in self.df_metricas.iterrows():
                pol = row['Politica_Key']
                if pol in dados_por_politica:
                    dados_por_politica[pol]['Tempo_Espera_ms'] = row['Tempo_Espera_ms']
                    dados_por_politica[pol]['Tempo_Execucao_ms'] = row['Tempo_Execucao_ms']
                    dados_por_politica[pol]['CPU_Utilizacao_Pct'] = row['CPU_Utilizacao_Pct']
                    dados_por_politica[pol]['Throughput_proc_s'] = row['Throughput_proc_s']
        
        # Preencher dados de memória
        for pol, df in self.dados_memoria.items():
            if pol in dados_por_politica and len(df) > 0:
                ultimo = df.iloc[-1]
                dados_por_politica[pol]['Cache_Hits'] = ultimo.get('cache_hits', np.nan)
                dados_por_politica[pol]['Cache_Misses'] = ultimo.get('cache_misses', np.nan)
                dados_por_politica[pol]['Hit_Rate'] = ultimo.get('hit_rate', np.nan)
        
        # Criar DataFrame unificado
        self.df_unificado = pd.DataFrame(list(dados_por_politica.values()))
            
    def get_politicas_disponiveis(self):
        """Retorna lista de políticas disponíveis"""
        politicas = set()
        if self.df_multicore is not None:
            politicas.update(self.df_multicore['Politica'].unique())
        if self.df_metricas is not None:
            politicas.update(self.df_metricas['Politica_Key'].unique())
        politicas.update(self.dados_memoria.keys())
        return sorted(list(politicas))
    
    def get_cores_disponiveis(self):
        """Retorna lista de configurações de cores disponíveis"""
        if self.df_multicore is not None:
            return sorted(self.df_multicore['Cores'].unique().tolist())
        return [1, 2, 4, 6]
    
    def get_todas_metricas(self):
        """Retorna TODAS as métricas disponíveis para eixos X e Y"""
        metricas = [
            ('Politica', 'Escalonador (categórico)'),
        ]
        
        # Métricas de multicore
        metricas.extend([
            ('Tempo_ms', 'Tempo Médio (ms)'),
            ('Tempo_1core', 'Tempo 1 Core (ms)'),
            ('Tempo_2cores', 'Tempo 2 Cores (ms)'),
            ('Tempo_4cores', 'Tempo 4 Cores (ms)'),
            ('Tempo_6cores', 'Tempo 6 Cores (ms)'),
            ('Speedup', 'Speedup Médio'),
            ('Speedup_2cores', 'Speedup 2 Cores'),
            ('Speedup_4cores', 'Speedup 4 Cores'),
            ('Speedup_6cores', 'Speedup 6 Cores'),
            ('Eficiencia_Pct', 'Eficiência Média (%)'),
            ('Eficiencia_2cores', 'Eficiência 2 Cores (%)'),
            ('Eficiencia_4cores', 'Eficiência 4 Cores (%)'),
            ('Eficiencia_6cores', 'Eficiência 6 Cores (%)'),
            ('CV_Pct', 'Coef. Variação (%)'),
        ])
        
        # Métricas gerais
        metricas.extend([
            ('Tempo_Espera_ms', 'Tempo de Espera (ms)'),
            ('Tempo_Execucao_ms', 'Tempo Turnaround (ms)'),
            ('CPU_Utilizacao_Pct', 'Utilização CPU (%)'),
            ('Throughput_proc_s', 'Throughput (proc/s)'),
        ])
        
        # Métricas de memória
        metricas.extend([
            ('Cache_Hits', 'Cache Hits'),
            ('Cache_Misses', 'Cache Misses'),
            ('Hit_Rate', 'Taxa de Acerto Cache (%)'),
        ])
        
        # Cores como eixo X
        metricas.append(('Cores', 'Número de Cores'))
            
        return metricas
    
    def get_metricas_disponiveis(self):
        """Retorna lista de métricas disponíveis para plotagem (compatibilidade)"""
        return self.get_todas_metricas()
    
    def get_dados_xy(self, metrica_x, metrica_y, politicas):
        """Retorna dados para gráfico X vs Y para as políticas selecionadas"""
        resultado = {}
        
        # NOVO: Se temos o CSV unificado, usar ele para dados por cores
        if self.df_unified is not None and metrica_x == 'Cores':
            # Mapeamento de métricas para colunas do CSV unificado
            col_map = {
                'Tempo_ms': 'Tempo_ms',
                'Speedup': 'Speedup',
                'Eficiencia_Pct': 'Eficiencia_Pct',
                'CV_Pct': 'CV_Pct',
                'Cache_Hits': 'Cache_Hits',
                'Cache_Misses': 'Cache_Misses',
                'Hit_Rate': 'Hit_Rate_Pct',
                'Hit_Rate_Pct': 'Hit_Rate_Pct',
                'Tempo_Espera_ms': 'Tempo_Espera_ms',
                'Tempo_Turnaround_ms': 'Tempo_Turnaround_ms',
                'Tempo_Execucao_ms': 'Tempo_Turnaround_ms',
                'CPU_Utilizacao_Pct': 'CPU_Utilizacao_Pct',
                'Throughput_proc_s': 'Throughput_proc_s',
            }
            
            col_y = col_map.get(metrica_y, metrica_y)
            
            if col_y in self.df_unified.columns:
                for pol in politicas:
                    df_pol = self.df_unified[self.df_unified['Politica'] == pol]
                    if not df_pol.empty:
                        resultado[pol] = {
                            'x': df_pol['Cores'].values,
                            'y': df_pol[col_y].values
                        }
                if resultado:
                    return resultado
        
        # Caso especial: Cores no eixo X (dados de multicore por core)
        if metrica_x == 'Cores':
            if self.df_multicore is not None:
                for pol in politicas:
                    df_pol = self.df_multicore[self.df_multicore['Politica'] == pol]
                    if not df_pol.empty:
                        # Mapear métrica Y para coluna do multicore
                        col_y = metrica_y
                        if metrica_y == 'Tempo_ms':
                            col_y = 'Tempo_ms'
                        elif metrica_y == 'Eficiencia_Pct':
                            col_y = 'Eficiencia_Pct'
                        
                        if col_y in df_pol.columns:
                            resultado[pol] = {
                                'x': df_pol['Cores'].values,
                                'y': df_pol[col_y].values
                            }
            return resultado
        
        # Caso especial: Politica no eixo X (gráfico de barras comparativo)
        if metrica_x == 'Politica':
            for pol in politicas:
                if self.df_unificado is not None:
                    df_pol = self.df_unificado[self.df_unificado['Politica'] == pol]
                    if not df_pol.empty and metrica_y in df_pol.columns:
                        val = df_pol[metrica_y].values[0]
                        if not np.isnan(val):
                            resultado[pol] = {'x': pol, 'y': val}
            return resultado
        
        # Caso geral: X e Y são métricas numéricas do dataset unificado
        if self.df_unificado is not None:
            for pol in politicas:
                df_pol = self.df_unificado[self.df_unificado['Politica'] == pol]
                if not df_pol.empty:
                    if metrica_x in df_pol.columns and metrica_y in df_pol.columns:
                        val_x = df_pol[metrica_x].values[0]
                        val_y = df_pol[metrica_y].values[0]
                        if not np.isnan(val_x) and not np.isnan(val_y):
                            resultado[pol] = {'x': val_x, 'y': val_y}
        
        return resultado
    
    def get_dados_para_grafico(self, metrica, politicas, cores=None):
        """Retorna dados formatados para plotagem (compatibilidade)"""
        return self.get_dados_xy('Cores', metrica, politicas)


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
        
        self.btn_gerar_unificado = QPushButton('🎯 Gerar Dados Completos')
        self.btn_gerar_unificado.clicked.connect(lambda: self.executar_teste('test_complete_unified'))
        self.btn_gerar_unificado.setStyleSheet('background-color: #27ae60; color: white; font-weight: bold;')
        self.btn_gerar_unificado.setToolTip('Executa teste unificado que gera TODOS os dados\n(Cores vs Cache, Cores vs Throughput, etc.)')
        
        self.btn_gerar_metricas = QPushButton('📊 Métricas Simples')
        self.btn_gerar_metricas.clicked.connect(lambda: self.executar_teste('test_metrics_complete'))
        
        self.btn_gerar_comparativo = QPushButton('⚖️ Comparativo')
        self.btn_gerar_comparativo.clicked.connect(lambda: self.executar_teste('test_multicore_comparative'))
        
        layout.addWidget(self.btn_atualizar)
        layout.addWidget(self.btn_gerar_unificado)
        layout.addWidget(self.btn_gerar_metricas)
        layout.addWidget(self.btn_gerar_comparativo)
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
        
        linhas = []
        
        # Destacar se CSV unificado está disponível
        if dm.df_unified is not None:
            linhas.append(f"🎯 UNIFICADO: {len(dm.df_unified)} registros")
            linhas.append("   (Cores vs Cache disponível!)")
        
        if dm.df_multicore is not None:
            linhas.append(f"📊 Multicore: {len(dm.df_multicore)} registros")
        if dm.df_metricas is not None:
            linhas.append(f"📈 Métricas: {len(dm.df_metricas)} políticas")
        if dm.dados_memoria:
            linhas.append(f"💾 Memória: {len(dm.dados_memoria)} arquivos")
        
        politicas = dm.get_politicas_disponiveis()
        linhas.append(f"\n🔹 Políticas: {', '.join(politicas)}")
        
        cores = dm.get_cores_disponiveis()
        linhas.append(f"🔹 Cores: {', '.join(map(str, cores))}")
        
        self.lbl_resumo.setText('\n'.join(linhas))
        
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
        
    def executar_todos_testes(self):
        """Executa o teste unificado completo"""
        self.console.append('\n🎯 Executando teste unificado completo...\n')
        self.executar_teste('test_complete_unified')
        
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
