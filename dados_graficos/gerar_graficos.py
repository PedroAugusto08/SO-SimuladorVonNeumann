#!/usr/bin/env python3
"""
Gerador de Gráficos - Simulador Von Neumann
Gera visualizações dos dados de escalonadores e métricas.
"""

import matplotlib
matplotlib.use('Agg')  # Backend sem GUI

from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Configuração de estilo
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12

# Caminhos base
BASE_DIR = Path(__file__).parent
CSV_DIR = BASE_DIR / 'csv'
PLOTS_DIR = BASE_DIR / 'plots'
PLOTS_DIR.mkdir(parents=True, exist_ok=True)

# Cores para cada política
CORES_POLITICAS = {
    'RR': '#2ecc71',           # Verde
    'FCFS': '#3498db',         # Azul
    'SJN': '#e74c3c',          # Vermelho
    'PRIORITY': '#9b59b6',     # Roxo
}

def carregar_dados():
    """Carrega todos os CSVs disponíveis"""
    multicore = pd.read_csv(CSV_DIR / 'escalonadores_multicore.csv')
    metricas = pd.read_csv(CSV_DIR / 'metricas_escalonadores.csv')
    return multicore, metricas

def grafico1_tempo_execucao_multicore(df):
    """
    GRÁFICO PRINCIPAL: Tempo de Execução por Política e Número de Cores
    Mostra como cada escalonador escala com múltiplos cores.
    """
    fig, ax = plt.subplots(figsize=(14, 8))
    
    politicas = df['Politica'].unique()
    cores = df['Cores'].unique()
    x = np.arange(len(cores))
    n_politicas = len(politicas)
    width = 0.15
    
    # Calcular offset para centralizar as barras
    offset = (n_politicas - 1) * width / 2
    
    for i, politica in enumerate(politicas):
        dados = df[df['Politica'] == politica]
        tempos = dados['Tempo_ms'].values
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        
        bars = ax.bar(x + i * width - offset, tempos, width, label=politica, color=cor, edgecolor='black', linewidth=0.5)
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Tempo de Execução (ms)', fontweight='bold', fontsize=12)
    ax.set_title('Comparativo de Tempo de Execução: 4 Políticas × 4 Configurações de Cores\n(menor é melhor)', 
                 fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{c} core(s)' for c in cores])
    ax.legend(title='Política', loc='upper right', fontsize=10, bbox_to_anchor=(1.15, 1))
    
    # Escala linear com limites ajustados para ver diferenças
    min_val = df['Tempo_ms'].min() * 0.95
    max_val = df['Tempo_ms'].max() * 1.05
    ax.set_ylim(min_val, max_val)
    
    # Adicionar grid horizontal
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    ax.set_axisbelow(True)
    
    # Anotação explicativa - canto superior esquerdo (dentro do gráfico)
    ax.annotate('Todas as políticas otimizadas\ncom tempos similares (~110-122ms)',
               xy=(0.02, 0.98), xycoords='axes fraction',
               fontsize=10, ha='left', va='top',
               bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico1_tempo_multicore.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico1_tempo_multicore.pdf', bbox_inches='tight')
    print('✅ Gráfico 1 salvo: plots/grafico1_tempo_multicore.png/pdf')
    plt.close()

def grafico2_eficiencia_escalabilidade(df):
    """
    GRÁFICO 2: Speedup e Eficiência - Gráficos de Linha
    Mostra a evolução do speedup e eficiência com aumento de cores.
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # Definir marcadores diferentes para cada política
    marcadores = ['o', 's', '^', 'D', 'v']
    estilos_linha = ['-', '--', '-.', ':', '-']
    
    # Subplot 1: Speedup
    politicas = df['Politica'].unique()
    for i, politica in enumerate(politicas):
        dados = df[df['Politica'] == politica]
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        marcador = marcadores[i % len(marcadores)]
        estilo = estilos_linha[i % len(estilos_linha)]
        ax1.plot(dados['Cores'], dados['Speedup'], marker=marcador, linestyle=estilo,
                label=politica, color=cor, linewidth=2.5, markersize=12, 
                markeredgecolor='black', markeredgewidth=1.5)
    
    # Linha de baseline
    cores = df['Cores'].unique()
    ax1.axhline(y=1.0, color='gray', linestyle='--', alpha=0.7, linewidth=1.5, label='Baseline (1.0)')
    
    ax1.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax1.set_ylabel('Speedup', fontweight='bold', fontsize=12)
    ax1.set_title('Speedup por Número de Cores\n(maior é melhor)', fontweight='bold', fontsize=13)
    ax1.legend(loc='upper left', fontsize=9, framealpha=0.9)
    ax1.set_ylim(0.95, 1.20)
    ax1.grid(True, linestyle='--', alpha=0.7)
    ax1.set_xticks(cores)
    
    # Subplot 2: Eficiência (%)
    for i, politica in enumerate(politicas):
        dados = df[df['Politica'] == politica]
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        marcador = marcadores[i % len(marcadores)]
        estilo = estilos_linha[i % len(estilos_linha)]
        eficiencia = np.clip(dados['Eficiencia_Pct'].values, 0, 120)
        ax2.plot(dados['Cores'], eficiencia, marker=marcador, linestyle=estilo,
                label=politica, color=cor, linewidth=2.5, markersize=12,
                markeredgecolor='black', markeredgewidth=1.5)
    
    ax2.axhline(y=100, color='green', linestyle='--', alpha=0.6, linewidth=1.5, label='100% (ideal)')
    ax2.axhline(y=50, color='orange', linestyle='--', alpha=0.6, linewidth=1.5, label='50% (aceitável)')
    
    ax2.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax2.set_ylabel('Eficiência (%)', fontweight='bold', fontsize=12)
    ax2.set_title('Eficiência de Paralelização\n(maior é melhor)', fontweight='bold', fontsize=13)
    ax2.legend(loc='upper right', fontsize=9, framealpha=0.9)
    ax2.set_ylim(0, 115)
    ax2.grid(True, linestyle='--', alpha=0.7)
    ax2.set_xticks(cores)
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico2_speedup_eficiencia.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico2_speedup_eficiencia.pdf', bbox_inches='tight')
    print('✅ Gráfico 2 salvo: plots/grafico2_speedup_eficiencia.png/pdf')
    plt.close()


def grafico2b_eficiencia_por_cores(df):
    """
    GRÁFICO 2B: Eficiência detalhada por número de cores (barras)
    Facilita comparação direta entre políticas em cada configuração.
    """
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    politicas = df['Politica'].unique()
    cores_list = df['Cores'].unique()
    x = np.arange(len(politicas))
    width = 0.6
    
    # Gráfico 2x2: Eficiência por número de cores
    for idx, num_cores in enumerate(cores_list):
        ax = axes[idx // 2, idx % 2]
        dados_cores = df[df['Cores'] == num_cores]
        
        eficiencias = []
        cores_barras = []
        for politica in politicas:
            dado = dados_cores[dados_cores['Politica'] == politica]
            if len(dado) > 0:
                eficiencias.append(dado['Eficiencia_Pct'].values[0])
                cores_barras.append(CORES_POLITICAS.get(politica, '#95a5a6'))
        
        bars = ax.bar(x, eficiencias, width, color=cores_barras, edgecolor='black', linewidth=1)
        
        # Adicionar valores nas barras
        for bar, ef in zip(bars, eficiencias):
            ax.annotate(f'{ef:.1f}%',
                       xy=(bar.get_x() + bar.get_width() / 2, bar.get_height()),
                       xytext=(0, 3), textcoords="offset points",
                       ha='center', va='bottom', fontsize=10, fontweight='bold')
        
        ax.set_xlabel('Política de Escalonamento', fontweight='bold')
        ax.set_ylabel('Eficiência (%)', fontweight='bold')
        ax.set_title(f'Eficiência com {num_cores} Core(s)', fontweight='bold', fontsize=12)
        ax.set_xticks(x)
        ax.set_xticklabels(politicas, rotation=15, ha='right')
        ax.set_ylim(0, 115)
        ax.axhline(y=100, color='green', linestyle='--', alpha=0.5, linewidth=1)
        ax.axhline(y=50, color='orange', linestyle='--', alpha=0.5, linewidth=1)
        ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    # Título geral
    fig.suptitle('Eficiência de Paralelização por Número de Cores\n(maior é melhor - 100% = ideal)', 
                 fontweight='bold', fontsize=14, y=1.02)
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico2b_eficiencia_por_cores.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico2b_eficiencia_por_cores.pdf', bbox_inches='tight')
    print('✅ Gráfico 2b salvo: plots/grafico2b_eficiencia_por_cores.png/pdf')
    plt.close()

def grafico3_metricas_comparativas(df_metricas):
    """
    GRÁFICO 3: Comparativo de Métricas de Desempenho
    Radar chart + barras comparando throughput, CPU e tempo de espera.
    """
    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    
    # Limpar nomes das políticas
    df_metricas = df_metricas.copy()
    df_metricas['Politica_Clean'] = df_metricas['Politica'].str.replace('_', ' ').str.replace('  ', ' ')
    
    # Extrair nome curto da política
    politicas = []
    for p in df_metricas['Politica'].values:
        if 'FCFS' in p:
            politicas.append('FCFS')
        elif 'SJN' in p:
            politicas.append('SJN')
        elif 'Round' in p or 'RR' in p:
            politicas.append('RR')
        elif 'PRIORITY' in p:
            politicas.append('PRIORITY')
        else:
            politicas.append(p[:10])
    
    cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
    
    # Subplot 1: Throughput (proc/s)
    throughputs = df_metricas['Throughput_proc_s'].values
    bars1 = axes[0].barh(range(len(politicas)), throughputs, color=cores, edgecolor='black')
    axes[0].set_yticks(range(len(politicas)))
    axes[0].set_yticklabels(politicas)
    axes[0].set_xlabel('Throughput (processos/segundo)', fontweight='bold')
    axes[0].set_title('Throughput\n(maior é melhor)', fontweight='bold')
    axes[0].grid(True, axis='x', linestyle='--', alpha=0.7)
    
    # Destacar o melhor
    max_idx = np.argmax(throughputs)
    bars1[max_idx].set_edgecolor('gold')
    bars1[max_idx].set_linewidth(3)
    
    # Subplot 2: Utilização de CPU (%)
    cpu_util = df_metricas['CPU_Utilizacao_Pct'].values
    bars2 = axes[1].barh(range(len(politicas)), cpu_util, color=cores, edgecolor='black')
    axes[1].set_yticks(range(len(politicas)))
    axes[1].set_yticklabels(politicas)
    axes[1].set_xlabel('Utilização de CPU (%)', fontweight='bold')
    axes[1].set_title('Utilização de CPU\n(maior é melhor)', fontweight='bold')
    axes[1].grid(True, axis='x', linestyle='--', alpha=0.7)
    axes[1].set_xlim(0, 100)
    
    max_idx = np.argmax(cpu_util)
    bars2[max_idx].set_edgecolor('gold')
    bars2[max_idx].set_linewidth(3)
    
    # Subplot 3: Tempo de Espera (ms)
    wait_time = df_metricas['Tempo_Espera_ms'].values
    bars3 = axes[2].barh(range(len(politicas)), wait_time, color=cores, edgecolor='black')
    axes[2].set_yticks(range(len(politicas)))
    axes[2].set_yticklabels(politicas)
    axes[2].set_xlabel('Tempo de Espera (ms)', fontweight='bold')
    axes[2].set_title('Tempo Médio de Espera\n(menor é melhor)', fontweight='bold')
    axes[2].grid(True, axis='x', linestyle='--', alpha=0.7)
    
    # Destacar o melhor (menor tempo)
    min_idx = np.argmin(wait_time)
    bars3[min_idx].set_edgecolor('gold')
    bars3[min_idx].set_linewidth(3)
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico3_metricas_comparativas.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico3_metricas_comparativas.pdf', bbox_inches='tight')
    print('✅ Gráfico 3 salvo: plots/grafico3_metricas_comparativas.png/pdf')
    plt.close()


def grafico4_heatmap_tempo(df):
    """
    GRÁFICO 4: Heatmap de Tempo de Execução
    Matriz visual: Política × Número de Cores
    """
    # Criar matriz pivot
    pivot = df.pivot(index='Politica', columns='Cores', values='Tempo_ms')
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Criar heatmap
    im = ax.imshow(pivot.values, cmap='RdYlGn_r', aspect='auto')
    
    # Configurar eixos
    ax.set_xticks(np.arange(len(pivot.columns)))
    ax.set_yticks(np.arange(len(pivot.index)))
    ax.set_xticklabels([f'{c} cores' for c in pivot.columns])
    ax.set_yticklabels(pivot.index)
    
    # Adicionar valores nas células
    for i in range(len(pivot.index)):
        for j in range(len(pivot.columns)):
            valor = pivot.values[i, j]
            cor_texto = 'white' if valor > pivot.values.mean() else 'black'
            ax.text(j, i, f'{valor:.1f}', ha='center', va='center', 
                   color=cor_texto, fontweight='bold', fontsize=11)
    
    # Colorbar
    cbar = ax.figure.colorbar(im, ax=ax)
    cbar.ax.set_ylabel('Tempo (ms)', rotation=-90, va='bottom', fontweight='bold')
    
    ax.set_title('Heatmap: Tempo de Execução por Política e Cores\n(verde = mais rápido, vermelho = mais lento)', 
                 fontweight='bold', fontsize=13)
    ax.set_xlabel('Número de Cores', fontweight='bold')
    ax.set_ylabel('Política de Escalonamento', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico4_heatmap_tempo.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico4_heatmap_tempo.pdf', bbox_inches='tight')
    print('✅ Gráfico 4 salvo: plots/grafico4_heatmap_tempo.png/pdf')
    plt.close()


def grafico5_radar_metricas(df_metricas):
    """
    GRÁFICO 5: Radar Chart - Comparação Multidimensional
    Visualização de múltiplas métricas por escalonador.
    """
    # Preparar dados - normalizar para escala 0-100
    categorias = ['Throughput', 'CPU Util.', 'Eficiência', 'Resp. Rápida']
    
    # Normalizar dados (maior = melhor, exceto tempo de espera que invertemos)
    throughput_norm = df_metricas['Throughput_proc_s'] / df_metricas['Throughput_proc_s'].max() * 100
    cpu_norm = df_metricas['CPU_Utilizacao_Pct']
    eficiencia_norm = df_metricas['Eficiencia'] / df_metricas['Eficiencia'].max() * 100
    # Inverter tempo de espera (menor = melhor)
    espera_inv = (1 - df_metricas['Tempo_Espera_ms'] / df_metricas['Tempo_Espera_ms'].max()) * 100
    
    fig, ax = plt.subplots(figsize=(10, 10), subplot_kw=dict(polar=True))
    
    # Ângulos
    angles = np.linspace(0, 2 * np.pi, len(categorias), endpoint=False).tolist()
    angles += angles[:1]  # Fechar o polígono
    
    # Cores para cada política
    cores_radar = ['#3498db', '#e74c3c', '#2ecc71', '#9b59b6']
    
    # Extrair nomes curtos
    politicas_simples = []
    for p in df_metricas['Politica'].values:
        if 'FCFS' in p:
            politicas_simples.append('FCFS')
        elif 'SJN' in p:
            politicas_simples.append('SJN')
        elif 'Round' in p or 'RR' in p:
            politicas_simples.append('RR')
        elif 'PRIORITY' in p:
            politicas_simples.append('PRIORITY')
        else:
            politicas_simples.append(p[:10])
    
    for idx, (_, row) in enumerate(df_metricas.iterrows()):
        valores = [
            throughput_norm.iloc[idx],
            cpu_norm.iloc[idx],
            eficiencia_norm.iloc[idx],
            espera_inv.iloc[idx]
        ]
        valores += valores[:1]  # Fechar o polígono
        
        ax.plot(angles, valores, 'o-', linewidth=2, label=politicas_simples[idx], 
                color=cores_radar[idx], markersize=8)
        ax.fill(angles, valores, alpha=0.15, color=cores_radar[idx])
    
    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categorias, fontsize=12, fontweight='bold')
    ax.set_ylim(0, 100)
    ax.set_title('Radar: Comparação Multidimensional dos Escalonadores\n(mais longe do centro = melhor)', 
                 fontweight='bold', fontsize=13, y=1.08)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.0), fontsize=10)
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico5_radar_metricas.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico5_radar_metricas.pdf', bbox_inches='tight')
    print('✅ Gráfico 5 salvo: plots/grafico5_radar_metricas.png/pdf')
    plt.close()


def grafico6_confiabilidade_cv(df):
    """
    GRÁFICO 6: Confiabilidade das Medições (CV%)
    Mostra a variabilidade/estabilidade de cada configuração.
    """
    fig, ax = plt.subplots(figsize=(12, 6))
    
    politicas = df['Politica'].unique()
    cores_list = df['Cores'].unique()
    x = np.arange(len(cores_list))
    width = 0.15
    n_politicas = len(politicas)
    offset = (n_politicas - 1) * width / 2
    
    for i, politica in enumerate(politicas):
        dados = df[df['Politica'] == politica]
        cvs = dados['CV_Pct'].values
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        
        bars = ax.bar(x + i * width - offset, cvs, width, label=politica, 
                     color=cor, edgecolor='black', linewidth=0.5)
        
        # Adicionar valores
        for bar, cv in zip(bars, cvs):
            ax.annotate(f'{cv:.1f}%',
                       xy=(bar.get_x() + bar.get_width() / 2, bar.get_height()),
                       xytext=(0, 2), textcoords="offset points",
                       ha='center', va='bottom', fontsize=8, fontweight='bold')
    
    # Linhas de referência
    ax.axhline(y=5, color='green', linestyle='--', alpha=0.7, linewidth=1.5, label='Excelente (<5%)')
    ax.axhline(y=15, color='orange', linestyle='--', alpha=0.7, linewidth=1.5, label='Bom (<15%)')
    ax.axhline(y=25, color='red', linestyle='--', alpha=0.7, linewidth=1.5, label='Variável (>25%)')
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Coeficiente de Variação (%)', fontweight='bold', fontsize=12)
    ax.set_title('Confiabilidade das Medições (CV%)\n(menor = mais estável e confiável)', 
                 fontweight='bold', fontsize=13)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{c} core(s)' for c in cores_list])
    ax.legend(loc='upper right', fontsize=9)
    ax.set_ylim(0, max(df['CV_Pct'].max() * 1.3, 10))
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    plt.savefig(PLOTS_DIR / 'grafico6_confiabilidade_cv.png', dpi=150, bbox_inches='tight')
    plt.savefig(PLOTS_DIR / 'grafico6_confiabilidade_cv.pdf', bbox_inches='tight')
    print('✅ Gráfico 6 salvo: plots/grafico6_confiabilidade_cv.png/pdf')
    plt.close()


def main():
    print('=' * 60)
    print('  📊 GERADOR DE GRÁFICOS - SIMULADOR VON NEUMANN')
    print('=' * 60)
    print()
    
    # Carregar dados
    print('📂 Carregando dados...')
    multicore, metricas = carregar_dados()
    print(f'   • escalonadores_multicore.csv: {len(multicore)} linhas')
    print(f'   • metricas_escalonadores.csv: {len(metricas)} linhas')
    print()
    
    # Gerar gráficos
    print('🎨 Gerando gráficos...')
    print()
    
    grafico1_tempo_execucao_multicore(multicore)
    grafico2_eficiencia_escalabilidade(multicore)
    grafico2b_eficiencia_por_cores(multicore)
    grafico3_metricas_comparativas(metricas)
    grafico4_heatmap_tempo(multicore)
    grafico5_radar_metricas(metricas)
    grafico6_confiabilidade_cv(multicore)
    
    print()
    print('=' * 60)
    print('  ✅ TODOS OS GRÁFICOS GERADOS COM SUCESSO!')
    print('=' * 60)
    print()
    print('📁 Arquivos gerados:')
    print('   • plots/grafico1_tempo_multicore.png/pdf')
    print('   • plots/grafico2_speedup_eficiencia.png/pdf (linhas)')
    print('   • plots/grafico2b_eficiencia_por_cores.png/pdf (barras)')
    print('   • plots/grafico3_metricas_comparativas.png/pdf')
    print('   • plots/grafico4_heatmap_tempo.png/pdf')
    print('   • plots/grafico5_radar_metricas.png/pdf')
    print('   • plots/grafico6_confiabilidade_cv.png/pdf')
    print()

if __name__ == '__main__':
    main()
