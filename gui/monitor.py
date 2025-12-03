#!/usr/bin/env python3
import os
import sys
import glob
import traceback
import numpy as np

from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QLabel,
    QComboBox, QTextEdit, QCheckBox, QSpinBox, QFileDialog, QMessageBox,
    QGroupBox, QGridLayout
)
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QTimer

import pandas as pd
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

# Cores para cada política de escalonamento
CORES_POLITICAS = {
    'RR': '#2ecc71',           # Verde
    'FCFS': '#3498db',         # Azul
    'SJN': '#e74c3c',          # Vermelho
    'PRIORITY': '#9b59b6',     # Roxo
    'Round_Robin': '#2ecc71',
}


class MakeWorker(QThread):
    output = pyqtSignal(str)
    finished = pyqtSignal(int)

    def __init__(self, target="simulador"):
        super().__init__()
        self.target = target
        self.base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    def run(self):
        try:
            import subprocess
            proc = subprocess.Popen(
                ["make", self.target], 
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
            self.output.emit(repr(e) + "\n" + traceback.format_exc())
            self.finished.emit(-1)


class TestWorker(QThread):
    """Worker para executar testes em background"""
    output = pyqtSignal(str)
    finished = pyqtSignal(int)

    def __init__(self, test_name):
        super().__init__()
        self.test_name = test_name
        self.base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    def run(self):
        try:
            import subprocess
            test_path = os.path.join(self.base_dir, 'bin', self.test_name)
            if not os.path.isfile(test_path):
                self.output.emit(f"Teste {self.test_name} não encontrado. Compilando...\n")
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
            
            self.output.emit(f"\n=== Executando {self.test_name} ===\n")
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
            self.output.emit(repr(e) + "\n" + traceback.format_exc())
            self.finished.emit(-1)


class GUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('SO Monitor - Simulador Von Neumann')
        self.resize(1200, 800)
        
        # Diretório base do projeto
        self.base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.dados_dir = os.path.join(self.base_dir, 'dados_graficos')

        self.dataframes = {}
        self.current_cols = []
        self.df_multicore = None
        self.df_metricas = None
        self.dados_memoria = {}

        # Layout principal
        main_layout = QVBoxLayout()

        # === Controles superiores ===
        ctrl_layout = QHBoxLayout()
        
        self.btn_compile = QPushButton('🔨 Compilar')
        self.btn_compile.clicked.connect(self.on_compile)
        self.btn_run = QPushButton('▶️ Simulador')
        self.btn_run.clicked.connect(self.on_run_simulator)
        self.btn_run_metrics = QPushButton('📊 Gerar Métricas')
        self.btn_run_metrics.clicked.connect(self.on_run_metrics_test)
        self.btn_run_comparative = QPushButton('⚖️ Comparativo')
        self.btn_run_comparative.clicked.connect(self.on_run_comparative_test)
        self.btn_update = QPushButton('🔄 Atualizar')
        self.btn_update.clicked.connect(self.load_csvs)

        ctrl_layout.addWidget(self.btn_compile)
        ctrl_layout.addWidget(self.btn_run)
        ctrl_layout.addWidget(self.btn_run_metrics)
        ctrl_layout.addWidget(self.btn_run_comparative)
        ctrl_layout.addWidget(self.btn_update)
        ctrl_layout.addStretch()

        # === Layout horizontal principal ===
        top_layout = QHBoxLayout()
        
        # === Painel esquerdo: Gráficos predefinidos ===
        left_panel = QGroupBox("Gráficos de Escalonadores")
        left_layout = QVBoxLayout()
        
        self.combo_grafico = QComboBox()
        self.combo_grafico.addItems([
            'Tempo por Cores (Barras)',
            'Tempo por Cores (Linhas)',
            'Speedup por Cores',
            'Eficiência por Cores',
            'CPU Utilização',
            'Throughput',
            'Cache Hits',
            'Cache Misses',
            'Hit Rate'
        ])
        self.btn_plot_preset = QPushButton('📈 Plotar')
        self.btn_plot_preset.clicked.connect(self.plot_preset_graph)
        
        left_layout.addWidget(QLabel('Selecione o gráfico:'))
        left_layout.addWidget(self.combo_grafico)
        left_layout.addWidget(self.btn_plot_preset)
        
        # Checkboxes para selecionar escalonadores
        left_layout.addWidget(QLabel(''))
        left_layout.addWidget(QLabel('Escalonadores a plotar:'))
        
        self.chk_escalonadores = {}
        escalonadores_info = [
            ('RR', 'Round Robin', '#2ecc71'),
            ('FCFS', 'FCFS', '#3498db'),
            ('SJN', 'SJN', '#e74c3c'),
            ('PRIORITY', 'Priority', '#9b59b6')
        ]
        
        for key, nome, cor in escalonadores_info:
            chk = QCheckBox(nome)
            chk.setChecked(True)
            chk.setStyleSheet(f'color: {cor}; font-weight: bold;')
            self.chk_escalonadores[key] = chk
            left_layout.addWidget(chk)
        
        # Botões de seleção rápida
        sel_layout = QHBoxLayout()
        btn_all = QPushButton('Todos')
        btn_none = QPushButton('Nenhum')
        btn_all.clicked.connect(self.select_all_schedulers)
        btn_none.clicked.connect(self.select_no_schedulers)
        sel_layout.addWidget(btn_all)
        sel_layout.addWidget(btn_none)
        left_layout.addLayout(sel_layout)
        
        # Métricas resumidas
        self.lbl_politicas = QLabel("Políticas: --")
        self.lbl_melhor_tempo = QLabel("Melhor Tempo: --")
        self.lbl_melhor_throughput = QLabel("Melhor Throughput: --")
        self.lbl_cache_hits = QLabel("Cache Hits: --")
        
        left_layout.addWidget(QLabel(''))
        left_layout.addWidget(QLabel('📊 Métricas Resumidas:'))
        left_layout.addWidget(self.lbl_politicas)
        left_layout.addWidget(self.lbl_melhor_tempo)
        left_layout.addWidget(self.lbl_melhor_throughput)
        left_layout.addWidget(self.lbl_cache_hits)
        left_layout.addStretch()
        
        left_panel.setLayout(left_layout)
        left_panel.setMaximumWidth(250)

        # === Painel direito: Canvas e controles custom ===
        right_layout = QVBoxLayout()
        
        # Controles de plot customizado
        plot_ctrl = QHBoxLayout()
        self.combo_x = QComboBox()
        self.combo_y = QComboBox()
        self.combo_y.setMaxVisibleItems(15)
        self.combo_type = QComboBox()
        self.combo_type.addItems(['Line', 'Bar', 'Scatter'])
        self.combo_dataset = QComboBox()
        self.btn_plot = QPushButton('Plot Custom')
        self.btn_plot.clicked.connect(self.plot_selected)
        self.btn_save = QPushButton('💾 Salvar')
        self.btn_save.clicked.connect(self.save_chart)

        plot_ctrl.addWidget(QLabel('Dataset:'))
        plot_ctrl.addWidget(self.combo_dataset)
        plot_ctrl.addWidget(QLabel('X:'))
        plot_ctrl.addWidget(self.combo_x)
        plot_ctrl.addWidget(QLabel('Y:'))
        plot_ctrl.addWidget(self.combo_y)
        plot_ctrl.addWidget(QLabel('Tipo:'))
        plot_ctrl.addWidget(self.combo_type)
        plot_ctrl.addWidget(self.btn_plot)
        plot_ctrl.addWidget(self.btn_save)

        # Matplotlib canvas
        self.fig = Figure(figsize=(8, 5))
        self.canvas = FigureCanvas(self.fig)

        right_layout.addLayout(plot_ctrl)
        right_layout.addWidget(self.canvas, stretch=4)

        # Console de saída
        self.output_box = QTextEdit()
        self.output_box.setReadOnly(True)
        self.output_box.setMaximumHeight(150)
        right_layout.addWidget(self.output_box, stretch=1)

        # Montar layout
        top_layout.addWidget(left_panel)
        top_layout.addLayout(right_layout, stretch=4)

        main_layout.addLayout(ctrl_layout)
        main_layout.addLayout(top_layout)

        self.setLayout(main_layout)

        self.make_worker = None
        self.test_worker = None
        
        # Setup auto-refresh timer
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.load_csvs)
        
        # Conectar mudança de dataset
        self.combo_dataset.currentTextChanged.connect(self.update_columns)

        # Initial load
        self.load_csvs()

    def on_compile(self):
        self.output_box.clear()
        self.btn_compile.setEnabled(False)
        self.make_worker = MakeWorker("simulador")
        self.make_worker.output.connect(self.append_output)
        self.make_worker.finished.connect(self.on_make_finished)
        self.make_worker.start()

    def on_run_metrics_test(self):
        """Executa o teste de métricas completas"""
        self.output_box.clear()
        self.btn_run_metrics.setEnabled(False)
        self.test_worker = TestWorker("test_metrics_complete")
        self.test_worker.output.connect(self.append_output)
        self.test_worker.finished.connect(self.on_test_finished)
        self.test_worker.start()

    def on_run_comparative_test(self):
        """Executa o teste comparativo multicore"""
        self.output_box.clear()
        self.btn_run_comparative.setEnabled(False)
        self.output_box.append("⏳ Este teste pode demorar alguns minutos...\n")
        self.test_worker = TestWorker("test_multicore_comparative")
        self.test_worker.output.connect(self.append_output)
        self.test_worker.finished.connect(self.on_test_finished)
        self.test_worker.start()

    def on_test_finished(self, code):
        self.btn_run_metrics.setEnabled(True)
        self.btn_run_comparative.setEnabled(True)
        self.append_output(f"\n✅ Teste finalizado com código {code}\n")
        self.load_csvs()  # Recarregar dados

    def select_all_schedulers(self):
        """Seleciona todos os escalonadores"""
        for chk in self.chk_escalonadores.values():
            chk.setChecked(True)

    def select_no_schedulers(self):
        """Deseleciona todos os escalonadores"""
        for chk in self.chk_escalonadores.values():
            chk.setChecked(False)

    def get_selected_schedulers(self):
        """Retorna lista de escalonadores selecionados"""
        return [key for key, chk in self.chk_escalonadores.items() if chk.isChecked()]

    def append_output(self, text):
        self.output_box.moveCursor(self.output_box.textCursor().End)
        self.output_box.insertPlainText(text)

    def on_make_finished(self, code):
        self.btn_compile.setEnabled(True)
        self.append_output(f"\nmake finished with code {code}\n")

    def on_run_simulator(self):
        sim_path = os.path.join(self.base_dir, 'bin', 'simulador')
        if not os.path.isfile(sim_path):
            QMessageBox.warning(self, 'Simulador não encontrado', 
                              'Compile o projeto primeiro usando o botão Compilar.')
            return
        
        self.output_box.append("\n=== Executando simulador ===\n")
        import subprocess
        try:
            result = subprocess.run([sim_path], capture_output=True, text=True, 
                                  timeout=30, cwd=self.base_dir)
            self.output_box.append(result.stdout)
            if result.stderr:
                self.output_box.append("\nErros/Avisos:\n" + result.stderr)
            self.output_box.append(f"\n✅ Simulador finalizado com código {result.returncode}\n")
            self.load_csvs()
        except subprocess.TimeoutExpired:
            self.output_box.append("\n⚠️ Simulador excedeu o tempo limite de 30s\n")
        except Exception as e:
            self.output_box.append(f"\n❌ Erro ao executar: {e}\n")

    def load_csvs(self):
        if not os.path.isdir(self.dados_dir):
            self.output_box.append(f"⚠️ Pasta {self.dados_dir} não encontrada.\n")
            return

        # Carregar escalonadores_multicore.csv
        multicore_path = os.path.join(self.dados_dir, 'escalonadores_multicore.csv')
        if os.path.isfile(multicore_path):
            try:
                self.df_multicore = pd.read_csv(multicore_path)
            except Exception as e:
                self.output_box.append(f"❌ Erro ao carregar multicore: {e}\n")

        # Carregar metricas_escalonadores.csv
        metricas_path = os.path.join(self.dados_dir, 'metricas_escalonadores.csv')
        if os.path.isfile(metricas_path):
            try:
                self.df_metricas = pd.read_csv(metricas_path)
            except Exception as e:
                self.output_box.append(f"❌ Erro ao carregar métricas: {e}\n")

        # Carregar arquivos de memória
        memoria_files = glob.glob(os.path.join(self.dados_dir, 'memoria_*.csv'))
        self.dados_memoria = {}
        for f in memoria_files:
            nome = os.path.basename(f)
            politica = self.mapear_politica(nome)
            try:
                df = pd.read_csv(f)
                self.dados_memoria[politica] = df
            except Exception:
                pass

        # Carregar todos CSVs para plots customizados
        csvs = glob.glob(os.path.join(self.dados_dir, '*.csv'))
        self.dataframes = {}
        for f in csvs:
            name = os.path.basename(f)
            try:
                self.dataframes[name] = pd.read_csv(f)
            except Exception:
                continue

        # Atualizar combo de datasets
        self.combo_dataset.clear()
        for name in self.dataframes.keys():
            self.combo_dataset.addItem(name)

        # Atualizar métricas resumidas
        self.update_summary_metrics()
        self.output_box.append(f"✅ Dados carregados: {len(self.dataframes)} arquivos\n")

    def mapear_politica(self, nome_arquivo):
        """Mapeia nome do arquivo para política"""
        if 'FCFS' in nome_arquivo:
            return 'FCFS'
        elif 'SJN' in nome_arquivo:
            return 'SJN'
        elif 'Round_Robin' in nome_arquivo:
            return 'RR'
        elif 'PRIORITY' in nome_arquivo:
            return 'PRIORITY'
        return nome_arquivo

    def update_summary_metrics(self):
        """Atualiza labels de métricas resumidas"""
        if self.df_multicore is not None and len(self.df_multicore) > 0:
            politicas = self.df_multicore['Politica'].unique()
            self.lbl_politicas.setText(f"Políticas: {', '.join(politicas)}")
            
            melhor = self.df_multicore.loc[self.df_multicore['Tempo_ms'].idxmin()]
            self.lbl_melhor_tempo.setText(f"Melhor Tempo: {melhor['Politica']} ({melhor['Tempo_ms']:.2f}ms)")

        if self.df_metricas is not None and len(self.df_metricas) > 0:
            melhor_thr = self.df_metricas.loc[self.df_metricas['Throughput_proc_s'].idxmax()]
            pol_name = melhor_thr['Politica'].split('_')[0]
            self.lbl_melhor_throughput.setText(f"Melhor Throughput: {pol_name} ({melhor_thr['Throughput_proc_s']:.0f} proc/s)")

        if self.dados_memoria:
            total_hits = sum(df['cache_hits'].iloc[-1] for df in self.dados_memoria.values() if len(df) > 0)
            self.lbl_cache_hits.setText(f"Cache Hits Total: {int(total_hits)}")

    def update_columns(self, dataset_name):
        """Atualiza colunas quando muda dataset"""
        self.combo_x.clear()
        self.combo_y.clear()
        
        if dataset_name in self.dataframes:
            cols = self.dataframes[dataset_name].columns.tolist()
            self.combo_x.addItems(['index'] + cols)
            self.combo_y.addItems(cols)

    # ==================== GRÁFICOS PREDEFINIDOS ====================
    
    def plot_preset_graph(self):
        """Plota gráficos predefinidos de escalonadores"""
        grafico = self.combo_grafico.currentText()
        
        self.fig.clear()
        ax = self.fig.add_subplot(111)
        
        try:
            if 'Tempo por Cores (Barras)' in grafico:
                self.plot_tempo_multicore_bar(ax)
            elif 'Tempo por Cores (Linhas)' in grafico:
                self.plot_tempo_multicore_line(ax)
            elif 'Speedup' in grafico:
                self.plot_speedup(ax)
            elif 'Eficiência' in grafico:
                self.plot_eficiencia(ax)
            elif 'CPU Utilização' in grafico:
                self.plot_cpu_utilizacao(ax)
            elif 'Throughput' in grafico:
                self.plot_throughput(ax)
            elif 'Cache Hits' in grafico:
                self.plot_cache_hits(ax)
            elif 'Cache Misses' in grafico:
                self.plot_cache_misses(ax)
            elif 'Hit Rate' in grafico:
                self.plot_hit_rate(ax)
            
            self.fig.tight_layout()
            self.canvas.draw()
        except Exception as e:
            self.output_box.append(f"❌ Erro ao plotar: {e}\n")
            traceback.print_exc()

    def plot_tempo_multicore_bar(self, ax):
        """Gráfico de barras: Tempo por política e cores"""
        if self.df_multicore is None:
            ax.text(0.5, 0.5, 'Execute o teste comparativo primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
            
        df = self.df_multicore
        politicas = [p for p in df['Politica'].unique() if p in selected]
        
        if not politicas:
            ax.text(0.5, 0.5, 'Nenhum escalonador selecionado tem dados', ha='center', va='center', fontsize=12)
            return
        
        cores = df['Cores'].unique()
        x = np.arange(len(cores))
        width = 0.8 / len(politicas) if len(politicas) > 0 else 0.2
        offset = (len(politicas) - 1) * width / 2
        
        for i, politica in enumerate(politicas):
            dados = df[df['Politica'] == politica]
            tempos = dados['Tempo_ms'].values
            cor = CORES_POLITICAS.get(politica, '#95a5a6')
            ax.bar(x + i * width - offset, tempos, width, label=politica, 
                   color=cor, edgecolor='black', linewidth=0.5)
        
        ax.set_xlabel('Número de Cores', fontweight='bold')
        ax.set_ylabel('Tempo de Execução (ms)', fontweight='bold')
        ax.set_title('Tempo de Execução por Política e Cores', fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels([f'{c} core(s)' for c in cores])
        ax.legend(title='Política', loc='upper right')
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)

    def plot_tempo_multicore_line(self, ax):
        """Gráfico de linha: Tempo por cores"""
        if self.df_multicore is None:
            ax.text(0.5, 0.5, 'Execute o teste comparativo primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
            
        df = self.df_multicore
        marcadores = ['o', 's', '^', 'D']
        
        i = 0
        for politica in df['Politica'].unique():
            if politica not in selected:
                continue
            dados = df[df['Politica'] == politica]
            cor = CORES_POLITICAS.get(politica, '#95a5a6')
            ax.plot(dados['Cores'], dados['Tempo_ms'], marker=marcadores[i % 4],
                   label=politica, color=cor, linewidth=2, markersize=8)
            i += 1
        
        ax.set_xlabel('Número de Cores', fontweight='bold')
        ax.set_ylabel('Tempo de Execução (ms)', fontweight='bold')
        ax.set_title('Tempo de Execução por Número de Cores', fontweight='bold')
        ax.legend()
        ax.grid(True, linestyle='--', alpha=0.5)

    def plot_speedup(self, ax):
        """Gráfico de linha: Speedup por cores"""
        if self.df_multicore is None:
            ax.text(0.5, 0.5, 'Execute o teste comparativo primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
            
        df = self.df_multicore
        marcadores = ['o', 's', '^', 'D']
        
        i = 0
        for politica in df['Politica'].unique():
            if politica not in selected:
                continue
            dados = df[df['Politica'] == politica]
            cor = CORES_POLITICAS.get(politica, '#95a5a6')
            ax.plot(dados['Cores'], dados['Speedup'], marker=marcadores[i % 4],
                   label=politica, color=cor, linewidth=2, markersize=8)
            i += 1
        
        ax.axhline(y=1.0, color='gray', linestyle='--', alpha=0.7, label='Baseline')
        ax.set_xlabel('Número de Cores', fontweight='bold')
        ax.set_ylabel('Speedup', fontweight='bold')
        ax.set_title('Speedup por Número de Cores', fontweight='bold')
        ax.legend()
        ax.grid(True, linestyle='--', alpha=0.5)

    def plot_eficiencia(self, ax):
        """Gráfico de linha: Eficiência por cores"""
        if self.df_multicore is None:
            ax.text(0.5, 0.5, 'Execute o teste comparativo primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
            
        df = self.df_multicore
        marcadores = ['o', 's', '^', 'D']
        
        i = 0
        for politica in df['Politica'].unique():
            if politica not in selected:
                continue
            dados = df[df['Politica'] == politica]
            cor = CORES_POLITICAS.get(politica, '#95a5a6')
            ax.plot(dados['Cores'], dados['Eficiencia_Pct'], marker=marcadores[i % 4],
                   label=politica, color=cor, linewidth=2, markersize=8)
            i += 1
        
        ax.axhline(y=100, color='green', linestyle='--', alpha=0.6, label='100% (ideal)')
        ax.axhline(y=50, color='orange', linestyle='--', alpha=0.6, label='50%')
        ax.set_xlabel('Número de Cores', fontweight='bold')
        ax.set_ylabel('Eficiência (%)', fontweight='bold')
        ax.set_title('Eficiência de Paralelização', fontweight='bold')
        ax.legend()
        ax.grid(True, linestyle='--', alpha=0.5)

    def plot_cpu_utilizacao(self, ax):
        """Gráfico de barras: CPU Utilização"""
        if self.df_metricas is None:
            ax.text(0.5, 0.5, 'Execute o teste de métricas primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
        
        df = self.df_metricas
        df_filtered = df[df['Politica'].str.split('_').str[0].isin(selected)]
        
        if df_filtered.empty:
            ax.text(0.5, 0.5, 'Nenhum escalonador selecionado tem dados', ha='center', va='center', fontsize=12)
            return
        
        politicas = [p.split('_')[0] for p in df_filtered['Politica']]
        cpu = df_filtered['CPU_Utilizacao_Pct'].values
        
        cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
        bars = ax.bar(politicas, cpu, color=cores, edgecolor='black')
        
        for bar, c in zip(bars, cpu):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                   f'{c:.1f}%', ha='center', va='bottom', fontsize=9, fontweight='bold')
        
        ax.set_xlabel('Política', fontweight='bold')
        ax.set_ylabel('CPU Utilização (%)', fontweight='bold')
        ax.set_title('Utilização de CPU por Escalonador', fontweight='bold')
        ax.set_ylim(0, 100)
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)

    def plot_throughput(self, ax):
        """Gráfico de barras: Throughput"""
        if self.df_metricas is None:
            ax.text(0.5, 0.5, 'Execute o teste de métricas primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
        
        df = self.df_metricas
        df_filtered = df[df['Politica'].str.split('_').str[0].isin(selected)]
        
        if df_filtered.empty:
            ax.text(0.5, 0.5, 'Nenhum escalonador selecionado tem dados', ha='center', va='center', fontsize=12)
            return
        
        politicas = [p.split('_')[0] for p in df_filtered['Politica']]
        throughput = df_filtered['Throughput_proc_s'].values
        
        cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
        bars = ax.bar(politicas, throughput, color=cores, edgecolor='black')
        
        for bar, t in zip(bars, throughput):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                   f'{t:.0f}', ha='center', va='bottom', fontsize=9, fontweight='bold')
        
        ax.set_xlabel('Política', fontweight='bold')
        ax.set_ylabel('Throughput (proc/s)', fontweight='bold')
        ax.set_title('Throughput por Escalonador (maior é melhor)', fontweight='bold')
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)

    def plot_cache_hits(self, ax):
        """Gráfico de barras: Cache Hits"""
        if not self.dados_memoria:
            ax.text(0.5, 0.5, 'Execute o teste de métricas primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
        
        politicas = []
        hits = []
        for pol, df in self.dados_memoria.items():
            if pol in selected and len(df) > 0:
                politicas.append(pol)
                hits.append(int(df['cache_hits'].iloc[-1]))
        
        if not politicas:
            ax.text(0.5, 0.5, 'Nenhum escalonador selecionado tem dados', ha='center', va='center', fontsize=12)
            return
        
        cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
        bars = ax.bar(politicas, hits, color=cores, edgecolor='black')
        
        for bar, h in zip(bars, hits):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                   f'{h}', ha='center', va='bottom', fontweight='bold')
        
        ax.set_xlabel('Política', fontweight='bold')
        ax.set_ylabel('Cache Hits', fontweight='bold')
        ax.set_title('Cache Hits por Escalonador (maior é melhor)', fontweight='bold')
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)

    def plot_cache_misses(self, ax):
        """Gráfico de barras: Cache Misses"""
        if not self.dados_memoria:
            ax.text(0.5, 0.5, 'Execute o teste de métricas primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
        
        politicas = []
        misses = []
        for pol, df in self.dados_memoria.items():
            if pol in selected and len(df) > 0:
                politicas.append(pol)
                misses.append(int(df['cache_misses'].iloc[-1]))
        
        if not politicas:
            ax.text(0.5, 0.5, 'Nenhum escalonador selecionado tem dados', ha='center', va='center', fontsize=12)
            return
        
        cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
        bars = ax.bar(politicas, misses, color=cores, edgecolor='black')
        
        for bar, m in zip(bars, misses):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                   f'{m}', ha='center', va='bottom', fontweight='bold')
        
        ax.set_xlabel('Política', fontweight='bold')
        ax.set_ylabel('Cache Misses', fontweight='bold')
        ax.set_title('Cache Misses por Escalonador (menor é melhor)', fontweight='bold')
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)

    def plot_hit_rate(self, ax):
        """Gráfico de barras: Hit Rate"""
        if not self.dados_memoria:
            ax.text(0.5, 0.5, 'Execute o teste de métricas primeiro', ha='center', va='center', fontsize=12)
            return
        
        selected = self.get_selected_schedulers()
        if not selected:
            ax.text(0.5, 0.5, 'Selecione pelo menos um escalonador', ha='center', va='center', fontsize=12)
            return
        
        politicas = []
        rates = []
        for pol, df in self.dados_memoria.items():
            if pol in selected and len(df) > 0:
                politicas.append(pol)
                rates.append(float(df['hit_rate'].iloc[-1]))
        
        if not politicas:
            ax.text(0.5, 0.5, 'Nenhum escalonador selecionado tem dados', ha='center', va='center', fontsize=12)
            return
        
        cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
        bars = ax.bar(politicas, rates, color=cores, edgecolor='black')
        
        for bar, r in zip(bars, rates):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height(),
                   f'{r:.2f}%', ha='center', va='bottom', fontweight='bold')
        
        ax.set_xlabel('Política', fontweight='bold')
        ax.set_ylabel('Hit Rate (%)', fontweight='bold')
        ax.set_title('Taxa de Acerto de Cache por Escalonador', fontweight='bold')
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)

    # ==================== GRÁFICOS CUSTOMIZADOS ====================
    
    def plot_selected(self):
        """Plota gráfico customizado selecionado pelo usuário"""
        try:
            xcol = self.combo_x.currentText()
            ycol = self.combo_y.currentText()
            gtype = self.combo_type.currentText()
            dataset = self.combo_dataset.currentText()

            if not ycol or ycol == '':
                QMessageBox.warning(self, 'Seleção inválida', 'Escolha coluna Y para plotar.')
                return

            if not dataset or dataset not in self.dataframes:
                QMessageBox.warning(self, 'Dataset não encontrado', 'Selecione um dataset válido.')
                return

            df = self.dataframes[dataset]

            if xcol == 'index':
                x = np.arange(len(df))
            else:
                if xcol not in df.columns:
                    QMessageBox.warning(self, 'Coluna X ausente', f'Coluna {xcol} não encontrada.')
                    return
                x = pd.to_numeric(df[xcol], errors='coerce').values

            if ycol not in df.columns:
                QMessageBox.warning(self, 'Coluna Y ausente', f'Coluna {ycol} não encontrada.')
                return
            y = pd.to_numeric(df[ycol], errors='coerce').values

            # Remover NaN
            mask = ~(np.isnan(x) | np.isnan(y))
            x = x[mask]
            y = y[mask]

            if len(x) == 0:
                QMessageBox.warning(self, 'Dados inválidos', 'Nenhum dado numérico válido para plotar.')
                return

            self.fig.clear()
            ax = self.fig.add_subplot(111)
            
            if gtype == 'Line':
                ax.plot(x, y, '-o', linewidth=2, markersize=4, color='#3498db')
            elif gtype == 'Bar':
                ax.bar(range(len(y)), y, color='#3498db', edgecolor='black')
                ax.set_xticks(range(len(y)))
                ax.set_xticklabels([f'{v:.1f}' for v in x], rotation=45, ha='right')
            else:  # Scatter
                ax.scatter(x, y, alpha=0.6, color='#3498db', s=50)
                
            ax.set_xlabel(xcol, fontweight='bold')
            ax.set_ylabel(ycol, fontweight='bold')
            ax.set_title(f'{ycol} vs {xcol}', fontweight='bold')
            ax.grid(True, linestyle='--', alpha=0.5)
            self.fig.tight_layout()
            self.canvas.draw()
        except Exception as e:
            QMessageBox.critical(self, 'Erro ao plotar', f'Erro ao processar dados:\n{str(e)}')

    def save_chart(self):
        path, _ = QFileDialog.getSaveFileName(self, 'Salvar Gráfico', 
                                             os.path.join(self.base_dir, 'grafico.png'),
                                             'PNG Files (*.png);;PDF Files (*.pdf);;All Files (*)')
        if path:
            self.fig.savefig(path, dpi=150, bbox_inches='tight')
            self.output_box.append(f"✅ Gráfico salvo: {path}\n")


def main():
    app = QApplication(sys.argv)
    gui = GUI()
    gui.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
