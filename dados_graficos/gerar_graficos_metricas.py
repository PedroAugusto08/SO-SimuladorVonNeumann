#!/usr/bin/env python3
"""
Gerador de Gráficos de Métricas - Simulador Von Neumann
Gera visualizações comparativas entre políticas de escalonamento
para diferentes configurações de cores (1, 2, 4, 6).
"""

import matplotlib
matplotlib.use('Agg')  # Backend sem GUI

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# Configuração de estilo
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['figure.figsize'] = (14, 8)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12

# Cores para cada política
CORES_POLITICAS = {
    'RR': '#2ecc71',           # Verde
    'FCFS': '#3498db',         # Azul
    'SJN': '#e74c3c',          # Vermelho
    'PRIORITY': '#9b59b6',     # Roxo
}

# Marcadores e estilos de linha
MARCADORES = {'RR': 'o', 'FCFS': 's', 'SJN': '^', 'PRIORITY': 'D'}
ESTILOS = {'RR': '-', 'FCFS': '--', 'SJN': '-.', 'PRIORITY': ':'}

def carregar_todos_csvs():
    """Carrega todos os CSVs de métricas e combina em um DataFrame único."""
    configs_cores = [1, 2, 4, 6]
    dados = []
    
    for num_cores in configs_cores:
        arquivo = f'dados_graficos/csv/metricas_{num_cores}cores.csv'
        if os.path.exists(arquivo):
            df = pd.read_csv(arquivo)
            df['Cores'] = num_cores
            dados.append(df)
            print(f'   ✓ {arquivo}: {len(df)} políticas')
        else:
            print(f'   ⚠️  {arquivo} não encontrado')
    
    if not dados:
        return None
    
    return pd.concat(dados, ignore_index=True)


def grafico1_throughput_por_cores(df):
    """
    GRÁFICO 1: Throughput por Número de Cores (Barras Agrupadas)
    Compara o throughput de cada política em diferentes configurações de cores.
    """
    fig, ax = plt.subplots(figsize=(14, 8))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    x = np.arange(len(cores_list))
    width = 0.18
    n_politicas = len(politicas)
    offset = (n_politicas - 1) * width / 2
    
    for i, politica in enumerate(politicas):
        dados_pol = df[df['Politica'] == politica]
        throughputs = [dados_pol[dados_pol['Cores'] == c]['Throughput_proc_s'].values[0] 
                      if len(dados_pol[dados_pol['Cores'] == c]) > 0 else 0 
                      for c in cores_list]
        
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        bars = ax.bar(x + i * width - offset, throughputs, width, 
                     label=politica, color=cor, edgecolor='black', linewidth=0.8)
        
        # Adicionar valores nas barras
        for bar, val in zip(bars, throughputs):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 50,
                   f'{val:.0f}', ha='center', va='bottom', fontsize=8, fontweight='bold')
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Throughput (processos/segundo)', fontweight='bold', fontsize=12)
    ax.set_title('Throughput por Política e Número de Cores\n', 
                fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{c} core(s)' for c in cores_list], fontsize=11)
    ax.legend(title='Política', loc='upper left', fontsize=10)
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico1_throughput_cores.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico1_throughput_cores.pdf', bbox_inches='tight')
    print('✅ Gráfico 1 salvo: grafico1_throughput_cores.png/pdf')
    plt.close()


def grafico2_throughput_linhas(df):
    """
    GRÁFICO 2: Evolução do Throughput (Gráfico de Linhas)
    Mostra como o throughput evolui com o aumento de cores.
    """
    fig, ax = plt.subplots(figsize=(12, 7))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    
    for politica in politicas:
        dados_pol = df[df['Politica'] == politica].sort_values('Cores')
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        marcador = MARCADORES.get(politica, 'o')
        estilo = ESTILOS.get(politica, '-')
        
        ax.plot(dados_pol['Cores'], dados_pol['Throughput_proc_s'],
               marker=marcador, linestyle=estilo, color=cor,
               linewidth=2.5, markersize=10, markeredgecolor='black',
               markeredgewidth=1.2, label=politica)
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Throughput (processos/segundo)', fontweight='bold', fontsize=12)
    ax.set_title('Evolução do Throughput com Aumento de Cores\n', 
                fontweight='bold', fontsize=14)
    ax.set_xticks(cores_list)
    ax.legend(title='Política', loc='best', fontsize=11)
    ax.grid(True, linestyle='--', alpha=0.6)
    
    # Anotação
    ax.annotate('Throughput tende a aumentar\ncom mais cores disponíveis',
               xy=(0.02, 0.98), xycoords='axes fraction',
               fontsize=10, ha='left', va='top',
               bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.9))
    
    plt.tight_layout()
    plt.savefig('graficos/grafico2_throughput_evolucao.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico2_throughput_evolucao.pdf', bbox_inches='tight')
    print('✅ Gráfico 2 salvo: grafico2_throughput_evolucao.png/pdf')
    plt.close()


def grafico3_tempo_espera(df):
    """
    GRÁFICO 3: Tempo Médio de Espera por Cores
    Compara o tempo de espera de cada política.
    """
    fig, ax = plt.subplots(figsize=(14, 8))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    x = np.arange(len(cores_list))
    width = 0.18
    n_politicas = len(politicas)
    offset = (n_politicas - 1) * width / 2
    
    for i, politica in enumerate(politicas):
        dados_pol = df[df['Politica'] == politica]
        tempos = [dados_pol[dados_pol['Cores'] == c]['TempoMedioEspera_ms'].values[0] 
                 if len(dados_pol[dados_pol['Cores'] == c]) > 0 else 0 
                 for c in cores_list]
        
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        bars = ax.bar(x + i * width - offset, tempos, width, 
                     label=politica, color=cor, edgecolor='black', linewidth=0.8)
        
        # Adicionar valores nas barras
        for bar, val in zip(bars, tempos):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.02,
                   f'{val:.2f}', ha='center', va='bottom', fontsize=8, fontweight='bold')
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Tempo Médio de Espera (ms)', fontweight='bold', fontsize=12)
    ax.set_title('Tempo Médio de Espera por Política e Número de Cores\n', 
                fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{c} core(s)' for c in cores_list], fontsize=11)
    ax.legend(title='Política', loc='upper right', fontsize=10)
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico3_tempo_espera.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico3_tempo_espera.pdf', bbox_inches='tight')
    print('✅ Gráfico 3 salvo: grafico3_tempo_espera.png/pdf')
    plt.close()


def grafico4_tempo_turnaround(df):
    """
    GRÁFICO 4: Tempo Médio de Retorno (Turnaround) por Cores
    Destaca quanto tempo os processos levam do início ao fim.
    """
    fig, ax = plt.subplots(figsize=(14, 8))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    x = np.arange(len(cores_list))
    width = 0.18
    n_politicas = len(politicas)
    offset = (n_politicas - 1) * width / 2
    
    for i, politica in enumerate(politicas):
        dados_pol = df[df['Politica'] == politica]
        tempos = [dados_pol[dados_pol['Cores'] == c]['TempoMedioTurnaround_ms'].values[0]
                 if len(dados_pol[dados_pol['Cores'] == c]) > 0 else 0
                 for c in cores_list]
        
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        bars = ax.bar(x + i * width - offset, tempos, width,
                     label=politica, color=cor, edgecolor='black', linewidth=0.8)
        
        for bar, val in zip(bars, tempos):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.02,
                    f'{val:.2f}', ha='center', va='bottom', fontsize=8, fontweight='bold')
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Tempo Médio de Retorno (ms)', fontweight='bold', fontsize=12)
    ax.set_title('Tempo Médio de Retorno por Política e Número de Cores\n',
                fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{c} core(s)' for c in cores_list], fontsize=11)
    ax.legend(title='Política', loc='upper right', fontsize=10)
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico4_tempo_turnaround.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico4_tempo_turnaround.pdf', bbox_inches='tight')
    print('✅ Gráfico 4 salvo: grafico4_tempo_turnaround.png/pdf')
    plt.close()


def grafico5_tempo_execucao(df):
    """
    GRÁFICO 5: Tempo Médio de Execução por Cores
    Compara o tempo de execução de cada política.
    """
    fig, ax = plt.subplots(figsize=(12, 7))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    
    for politica in politicas:
        dados_pol = df[df['Politica'] == politica].sort_values('Cores')
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        marcador = MARCADORES.get(politica, 'o')
        estilo = ESTILOS.get(politica, '-')
        
        ax.plot(dados_pol['Cores'], dados_pol['TempoMedioExecucao_us'],
               marker=marcador, linestyle=estilo, color=cor,
               linewidth=2.5, markersize=10, markeredgecolor='black',
               markeredgewidth=1.2, label=politica)
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Tempo Médio de Execução (µs)', fontweight='bold', fontsize=12)
    ax.set_title('Tempo Médio de Execução vs Número de Cores\n', 
                fontweight='bold', fontsize=14)
    ax.set_xticks(cores_list)
    ax.legend(title='Política', loc='best', fontsize=11)
    ax.grid(True, linestyle='--', alpha=0.6)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico5_tempo_execucao.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico5_tempo_execucao.pdf', bbox_inches='tight')
    print('✅ Gráfico 5 salvo: grafico5_tempo_execucao.png/pdf')
    plt.close()


def grafico6_utilizacao_cpu(df):
    """
    GRÁFICO 6: Utilização de CPU por Cores
    Mostra como a utilização de CPU varia com o número de cores.
    """
    fig, ax = plt.subplots(figsize=(14, 8))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    x = np.arange(len(cores_list))
    width = 0.18
    n_politicas = len(politicas)
    offset = (n_politicas - 1) * width / 2
    
    for i, politica in enumerate(politicas):
        dados_pol = df[df['Politica'] == politica]
        cpu_util = [dados_pol[dados_pol['Cores'] == c]['CPUUtilizacao_pct'].values[0] 
                   if len(dados_pol[dados_pol['Cores'] == c]) > 0 else 0 
                   for c in cores_list]
        
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        bars = ax.bar(x + i * width - offset, cpu_util, width, 
                     label=politica, color=cor, edgecolor='black', linewidth=0.8)
        
        # Adicionar valores nas barras
        for bar, val in zip(bars, cpu_util):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5,
                   f'{val:.1f}%', ha='center', va='bottom', fontsize=8, fontweight='bold')
    
    # Linha de referência 100%
    ax.axhline(y=100, color='green', linestyle='--', alpha=0.7, linewidth=2, label='100% (ideal)')
    
    ax.set_xlabel('Número de Cores', fontweight='bold', fontsize=12)
    ax.set_ylabel('Utilização de CPU (%)', fontweight='bold', fontsize=12)
    ax.set_title('Utilização de CPU por Política e Número de Cores\n(maior é melhor, 100% = ideal)', 
                fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels([f'{c} core(s)' for c in cores_list], fontsize=11)
    ax.legend(title='Política', loc='lower right', fontsize=10)
    ax.set_ylim(0, 110)
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    # Anotação
    ax.annotate('Com mais cores que processos,\na utilização tende a diminuir',
               xy=(0.98, 0.02), xycoords='axes fraction',
               fontsize=10, ha='right', va='bottom',
               bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.9))
    
    plt.tight_layout()
    plt.savefig('graficos/grafico6_utilizacao_cpu.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico6_utilizacao_cpu.pdf', bbox_inches='tight')
    print('✅ Gráfico 6 salvo: grafico6_utilizacao_cpu.png/pdf')
    plt.close()


def grafico7_heatmap_throughput(df):
    """
    GRÁFICO 7: Heatmap de Throughput
    Matriz visual: Política × Número de Cores
    """
    # Criar matriz pivot
    pivot = df.pivot(index='Politica', columns='Cores', values='Throughput_proc_s')
    
    # Reordenar políticas
    ordem_politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    pivot = pivot.reindex([p for p in ordem_politicas if p in pivot.index])
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Criar heatmap
    im = ax.imshow(pivot.values, cmap='YlGn', aspect='auto')
    
    # Configurar eixos
    ax.set_xticks(np.arange(len(pivot.columns)))
    ax.set_yticks(np.arange(len(pivot.index)))
    ax.set_xticklabels([f'{c} cores' for c in pivot.columns], fontsize=11)
    ax.set_yticklabels(pivot.index, fontsize=11)
    
    # Adicionar valores nas células
    for i in range(len(pivot.index)):
        for j in range(len(pivot.columns)):
            valor = pivot.values[i, j]
            cor_texto = 'white' if valor > pivot.values.mean() else 'black'
            ax.text(j, i, f'{valor:.0f}', ha='center', va='center', 
                   color=cor_texto, fontweight='bold', fontsize=12)
    
    # Colorbar
    cbar = ax.figure.colorbar(im, ax=ax)
    cbar.ax.set_ylabel('Throughput (proc/s)', rotation=-90, va='bottom', fontweight='bold')
    
    ax.set_title('Heatmap: Throughput por Política e Cores\n(verde escuro = maior throughput)', 
                fontweight='bold', fontsize=13)
    ax.set_xlabel('Número de Cores', fontweight='bold')
    ax.set_ylabel('Política de Escalonamento', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig('graficos/grafico7_heatmap_throughput.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico7_heatmap_throughput.pdf', bbox_inches='tight')
    print('✅ Gráfico 7 salvo: grafico7_heatmap_throughput.png/pdf')
    plt.close()


def grafico8_heatmap_tempo_espera(df):
    """
    GRÁFICO 8: Heatmap de Tempo de Espera
    Matriz visual: Política × Número de Cores
    """
    # Criar matriz pivot
    pivot = df.pivot(index='Politica', columns='Cores', values='TempoMedioEspera_ms')
    
    # Reordenar políticas
    ordem_politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    pivot = pivot.reindex([p for p in ordem_politicas if p in pivot.index])
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Criar heatmap (invertido - menor é melhor)
    im = ax.imshow(pivot.values, cmap='RdYlGn_r', aspect='auto')
    
    # Configurar eixos
    ax.set_xticks(np.arange(len(pivot.columns)))
    ax.set_yticks(np.arange(len(pivot.index)))
    ax.set_xticklabels([f'{c} cores' for c in pivot.columns], fontsize=11)
    ax.set_yticklabels(pivot.index, fontsize=11)
    
    # Adicionar valores nas células
    for i in range(len(pivot.index)):
        for j in range(len(pivot.columns)):
            valor = pivot.values[i, j]
            cor_texto = 'white' if valor > pivot.values.mean() else 'black'
            ax.text(j, i, f'{valor:.2f}', ha='center', va='center', 
                   color=cor_texto, fontweight='bold', fontsize=11)
    
    # Colorbar
    cbar = ax.figure.colorbar(im, ax=ax)
    cbar.ax.set_ylabel('Tempo de Espera (ms)', rotation=-90, va='bottom', fontweight='bold')
    
    ax.set_title('Heatmap: Tempo Médio de Espera por Política e Cores\n(verde = menor tempo = melhor)', 
                fontweight='bold', fontsize=13)
    ax.set_xlabel('Número de Cores', fontweight='bold')
    ax.set_ylabel('Política de Escalonamento', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig('graficos/grafico8_heatmap_tempo_espera.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico8_heatmap_tempo_espera.pdf', bbox_inches='tight')
    print('✅ Gráfico 8 salvo: grafico8_heatmap_tempo_espera.png/pdf')
    plt.close()


def grafico9_radar_4cores(df):
    """
    GRÁFICO 9: Radar Chart - Comparação com 4 Cores
    Visualização multidimensional das métricas.
    """
    # Filtrar apenas 4 cores
    df_4cores = df[df['Cores'] == 4].copy()
    
    if df_4cores.empty:
        print('⚠️  Dados de 4 cores não encontrados para radar chart')
        return
    
    categorias = ['Throughput', 'CPU Util.', 'Eficiência', 'Resp. Rápida']
    
    # Normalizar dados (0-100)
    throughput_norm = df_4cores['Throughput_proc_s'] / df_4cores['Throughput_proc_s'].max() * 100
    cpu_norm = df_4cores['CPUUtilizacao_pct']
    eficiencia_norm = df_4cores['Eficiencia_pct']
    # Inverter tempo de espera (menor = melhor)
    max_espera = df_4cores['TempoMedioEspera_ms'].max()
    if max_espera > 0:
        espera_inv = (1 - df_4cores['TempoMedioEspera_ms'] / max_espera) * 100
    else:
        espera_inv = pd.Series([100] * len(df_4cores))
    
    fig, ax = plt.subplots(figsize=(10, 10), subplot_kw=dict(polar=True))
    
    # Ângulos
    angles = np.linspace(0, 2 * np.pi, len(categorias), endpoint=False).tolist()
    angles += angles[:1]
    
    politicas = df_4cores['Politica'].values
    
    for idx, politica in enumerate(politicas):
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        
        valores = [
            throughput_norm.iloc[idx],
            cpu_norm.iloc[idx],
            eficiencia_norm.iloc[idx],
            espera_inv.iloc[idx]
        ]
        valores += valores[:1]
        
        ax.plot(angles, valores, 'o-', linewidth=2.5, label=politica, 
               color=cor, markersize=8, markeredgecolor='black')
        ax.fill(angles, valores, alpha=0.15, color=cor)
    
    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categorias, fontsize=12, fontweight='bold')
    ax.set_ylim(0, 100)
    ax.set_title('Radar: Comparação Multidimensional (4 Cores)\n(mais longe do centro = melhor)', 
                fontweight='bold', fontsize=13, y=1.08)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.0), fontsize=11)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico9_radar_4cores.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico9_radar_4cores.pdf', bbox_inches='tight')
    print('✅ Gráfico 9 salvo: grafico9_radar_4cores.png/pdf')
    plt.close()


def grafico10_comparativo_geral(df):
    """
    GRÁFICO 10: Comparativo Geral - Subplots
    6 métricas principais em um único painel.
    """
    fig, axes = plt.subplots(2, 3, figsize=(18, 10))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())

    def plot_linhas(ax, coluna, ylabel, titulo):
        for politica in politicas:
            dados_pol = df[df['Politica'] == politica].sort_values('Cores')
            cor = CORES_POLITICAS.get(politica, '#95a5a6')
            ax.plot(dados_pol['Cores'], dados_pol[coluna],
                    marker=MARCADORES[politica], linestyle=ESTILOS[politica],
                    color=cor, linewidth=2, markersize=8, label=politica)
        ax.set_xlabel('Cores')
        ax.set_ylabel(ylabel)
        ax.set_title(titulo, fontweight='bold')
        ax.legend(loc='best')
        ax.grid(True, linestyle='--', alpha=0.5)
        ax.set_xticks(cores_list)
    
    plot_linhas(axes[0, 0], 'Throughput_proc_s', 'Throughput (proc/s)', 'Throughput')
    plot_linhas(axes[0, 1], 'TempoMedioEspera_ms', 'Tempo de Espera (ms)', 'Tempo Médio de Espera')
    plot_linhas(axes[0, 2], 'TempoMedioTurnaround_ms', 'Tempo de Retorno (ms)', 'Tempo Médio de Retorno')
    plot_linhas(axes[1, 0], 'TempoMedioExecucao_us', 'Tempo de Execução (µs)', 'Tempo Médio de Execução')
    plot_linhas(axes[1, 1], 'CPUUtilizacao_pct', 'Utilização de CPU (%)', 'Utilização de CPU')
    axes[1, 1].axhline(y=100, color='gray', linestyle='--', alpha=0.5, linewidth=1)
    axes[1, 1].set_ylim(0, 110)
    plot_linhas(axes[1, 2], 'Eficiencia_pct', 'Eficiência (%)', 'Eficiência da CPU')
    axes[1, 2].set_ylim(0, 110)
    
    fig.suptitle('Comparativo Geral: Métricas vs Número de Cores', 
                fontweight='bold', fontsize=16, y=1.02)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico10_comparativo_geral.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico10_comparativo_geral.pdf', bbox_inches='tight')
    print('✅ Gráfico 10 salvo: grafico10_comparativo_geral.png/pdf')
    plt.close()


def grafico11_cache_analysis(df):
    """
    GRÁFICO 11: Análise de Cache
    Cache Hits e Taxa de Hit por configuração.
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    cores_list = sorted(df['Cores'].unique())
    x = np.arange(len(cores_list))
    width = 0.18
    n_politicas = len(politicas)
    offset = (n_politicas - 1) * width / 2
    
    # Subplot 1: Cache Hits
    for i, politica in enumerate(politicas):
        dados_pol = df[df['Politica'] == politica]
        hits = [dados_pol[dados_pol['Cores'] == c]['CacheHits'].values[0] 
               if len(dados_pol[dados_pol['Cores'] == c]) > 0 else 0 
               for c in cores_list]
        
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        ax1.bar(x + i * width - offset, hits, width, 
               label=politica, color=cor, edgecolor='black', linewidth=0.8)
    
    ax1.set_xlabel('Número de Cores', fontweight='bold')
    ax1.set_ylabel('Cache Hits', fontweight='bold')
    ax1.set_title('Cache Hits por Política e Cores', fontweight='bold')
    ax1.set_xticks(x)
    ax1.set_xticklabels([f'{c}' for c in cores_list])
    ax1.legend(title='Política', loc='best')
    ax1.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    # Subplot 2: Taxa de Hit
    for i, politica in enumerate(politicas):
        dados_pol = df[df['Politica'] == politica]
        taxa = [dados_pol[dados_pol['Cores'] == c]['TaxaHit_pct'].values[0] 
               if len(dados_pol[dados_pol['Cores'] == c]) > 0 else 0 
               for c in cores_list]
        
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        ax2.bar(x + i * width - offset, taxa, width, 
               label=politica, color=cor, edgecolor='black', linewidth=0.8)
    
    ax2.set_xlabel('Número de Cores', fontweight='bold')
    ax2.set_ylabel('Taxa de Hit (%)', fontweight='bold')
    ax2.set_title('Taxa de Cache Hit por Política e Cores', fontweight='bold')
    ax2.set_xticks(x)
    ax2.set_xticklabels([f'{c}' for c in cores_list])
    ax2.legend(title='Política', loc='best')
    ax2.grid(True, axis='y', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    plt.savefig('graficos/grafico11_cache_analysis.png', dpi=150, bbox_inches='tight')
    plt.savefig('graficos/grafico11_cache_analysis.pdf', bbox_inches='tight')
    print('✅ Gráfico 11 salvo: grafico11_cache_analysis.png/pdf')
    plt.close()


def gerar_tabela_resumo(df):
    """Gera uma tabela resumo em texto."""
    print('\n' + '=' * 80)
    print('TABELA RESUMO - MÉTRICAS POR POLÍTICA E CORES')
    print('=' * 80)
    
    cores_list = sorted(df['Cores'].unique())
    politicas = ['RR', 'FCFS', 'SJN', 'PRIORITY']
    
    for num_cores in cores_list:
        print(f'\n--- {num_cores} CORE(S) ---')
        print(f'{"Política":<10} {"Throughput":>12} {"Espera(ms)":>12} {"Exec(µs)":>12} {"Retorno(ms)":>14} {"CPU%":>8}')
        print('-' * 72)
        
        df_cores = df[df['Cores'] == num_cores]
        for politica in politicas:
            dados = df_cores[df_cores['Politica'] == politica]
            if len(dados) > 0:
                row = dados.iloc[0]
                print(
                    f'{politica:<10} {row["Throughput_proc_s"]:>12.1f} '
                    f'{row["TempoMedioEspera_ms"]:>12.2f} '
                    f'{row["TempoMedioExecucao_us"]:>12.2f} '
                    f'{row["TempoMedioTurnaround_ms"]:>14.2f} '
                    f'{row["CPUUtilizacao_pct"]:>8.1f}'
                )
    
    print('\n' + '=' * 80)


def main():
    print('=' * 70)
    print('  📊 GERADOR DE GRÁFICOS DE MÉTRICAS - SIMULADOR VON NEUMANN')
    print('=' * 70)
    print()
    
    # Criar diretório de saída
    os.makedirs('graficos', exist_ok=True)
    
    # Carregar dados
    print('📂 Carregando dados dos CSVs...')
    df = carregar_todos_csvs()
    
    if df is None or df.empty:
        print('❌ Nenhum dado encontrado!')
        return
    
    print(f'\n   Total de registros: {len(df)}')
    print(f'   Configurações de cores: {sorted(df["Cores"].unique())}')
    print(f'   Políticas: {df["Politica"].unique().tolist()}')
    print()
    
    # Gerar gráficos
    print('🎨 Gerando gráficos...')
    print()
    
    grafico1_throughput_por_cores(df)
    grafico2_throughput_linhas(df)
    grafico3_tempo_espera(df)
    grafico4_tempo_turnaround(df)
    grafico5_tempo_execucao(df)
    grafico6_utilizacao_cpu(df)
    grafico7_heatmap_throughput(df)
    grafico8_heatmap_tempo_espera(df)
    grafico9_radar_4cores(df)
    grafico10_comparativo_geral(df)
    grafico11_cache_analysis(df)
    
    # Gerar tabela resumo
    gerar_tabela_resumo(df)
    
    print()
    print('=' * 70)
    print('  ✅ TODOS OS GRÁFICOS GERADOS COM SUCESSO!')
    print('=' * 70)
    print()
    print('📁 Arquivos gerados em graficos/:')
    print('   • grafico1_throughput_cores.png/pdf - Throughput por cores (barras)')
    print('   • grafico2_throughput_evolucao.png/pdf - Evolução do throughput (linhas)')
    print('   • grafico3_tempo_espera.png/pdf - Tempo de espera por cores')
    print('   • grafico4_tempo_turnaround.png/pdf - Tempo de retorno (barras)')
    print('   • grafico5_tempo_execucao.png/pdf - Tempo de execução (linhas)')
    print('   • grafico6_utilizacao_cpu.png/pdf - Utilização de CPU')
    print('   • grafico7_heatmap_throughput.png/pdf - Heatmap de throughput')
    print('   • grafico8_heatmap_tempo_espera.png/pdf - Heatmap de tempo de espera')
    print('   • grafico9_radar_4cores.png/pdf - Radar chart (4 cores)')
    print('   • grafico10_comparativo_geral.png/pdf - Comparativo 6 métricas')
    print('   • grafico11_cache_analysis.png/pdf - Análise de cache')
    print()


if __name__ == '__main__':
    main()
