#!/usr/bin/env python3
"""
Gerador de Gráficos - Simulador Von Neumann
Gera visualizações dos dados de escalonadores e métricas.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Configuração de estilo
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12

# Cores para cada política
CORES_POLITICAS = {
    'RR': '#2ecc71',           # Verde
    'FCFS': '#3498db',         # Azul
    'SJN': '#e74c3c',          # Vermelho
    'PRIORITY': '#9b59b6',     # Roxo
    'PRIORITY_PREEMPT': '#f39c12'  # Laranja
}

def carregar_dados():
    """Carrega todos os CSVs"""
    multicore = pd.read_csv('escalonadores_multicore.csv')
    metricas = pd.read_csv('metricas_escalonadores.csv')
    throughput = pd.read_csv('throughput_multicore.csv')
    return multicore, metricas, throughput

def grafico1_tempo_execucao_multicore(df):
    """
    GRÁFICO PRINCIPAL: Tempo de Execução por Política e Número de Cores
    Mostra como cada escalonador escala com múltiplos cores.
    """
    fig, ax = plt.subplots(figsize=(14, 8))
    
    politicas = df['Politica'].unique()
    cores = df['Cores'].unique()
    x = np.arange(len(cores))
    width = 0.15
    
    for i, politica in enumerate(politicas):
        dados = df[df['Politica'] == politica]
        tempos = dados['Tempo_ms'].values
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        
        bars = ax.bar(x + i * width, tempos, width, label=politica, color=cor, edgecolor='black', linewidth=0.5)
        
        # Adicionar valores nas barras (apenas se tempo < 1000ms para não poluir)
        for bar, tempo in zip(bars, tempos):
            if tempo < 1000:
                ax.annotate(f'{tempo:.0f}',
                           xy=(bar.get_x() + bar.get_width() / 2, bar.get_height()),
                           xytext=(0, 3), textcoords="offset points",
                           ha='center', va='bottom', fontsize=8, fontweight='bold')
    
    ax.set_xlabel('Número de Cores', fontweight='bold')
    ax.set_ylabel('Tempo de Execução (ms)', fontweight='bold')
    ax.set_title('⏱️ Comparativo de Tempo de Execução: 5 Políticas × 4 Configurações de Cores\n(menor é melhor)', 
                 fontweight='bold', fontsize=14)
    ax.set_xticks(x + width * 2)
    ax.set_xticklabels([f'{c} core(s)' for c in cores])
    ax.legend(title='Política', loc='upper left', fontsize=10)
    ax.set_yscale('log')  # Escala log devido à grande diferença entre RR e outros
    
    # Adicionar grid horizontal
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    ax.set_axisbelow(True)
    
    # Anotação explicativa
    ax.annotate('📊 Round Robin (RR) domina em todos os cenários\n    com tempo ~100ms vs ~3-9s das outras políticas',
               xy=(0.02, 0.98), xycoords='axes fraction',
               fontsize=10, ha='left', va='top',
               bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig('grafico1_tempo_multicore.png', dpi=150, bbox_inches='tight')
    plt.savefig('grafico1_tempo_multicore.pdf', bbox_inches='tight')
    print('✅ Gráfico 1 salvo: grafico1_tempo_multicore.png/pdf')
    plt.close()

def grafico2_eficiencia_escalabilidade(df):
    """
    GRÁFICO 2: Eficiência de Escalabilidade por Política
    Mostra quão bem cada política aproveita múltiplos cores.
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # Subplot 1: Speedup
    for politica in df['Politica'].unique():
        dados = df[df['Politica'] == politica]
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        ax1.plot(dados['Cores'], dados['Speedup'], 'o-', label=politica, 
                color=cor, linewidth=2, markersize=8)
    
    # Linha ideal (speedup linear)
    cores = df['Cores'].unique()
    ax1.plot(cores, [1, 1, 1, 1], 'k--', label='Baseline (1.0)', alpha=0.5, linewidth=1)
    
    ax1.set_xlabel('Número de Cores', fontweight='bold')
    ax1.set_ylabel('Speedup', fontweight='bold')
    ax1.set_title('📈 Speedup por Número de Cores\n(maior é melhor)', fontweight='bold')
    ax1.legend(loc='upper left', fontsize=9)
    ax1.set_ylim(0, 2)
    ax1.grid(True, linestyle='--', alpha=0.7)
    
    # Subplot 2: Eficiência (%)
    for politica in df['Politica'].unique():
        dados = df[df['Politica'] == politica]
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        # Limitar eficiência a 100% para visualização
        eficiencia = np.clip(dados['Eficiencia_Pct'].values, 0, 120)
        ax2.plot(dados['Cores'], eficiencia, 's-', label=politica,
                color=cor, linewidth=2, markersize=8)
    
    ax2.axhline(y=100, color='green', linestyle='--', alpha=0.5, label='100% (ideal)')
    ax2.axhline(y=50, color='orange', linestyle='--', alpha=0.5, label='50% (aceitável)')
    
    ax2.set_xlabel('Número de Cores', fontweight='bold')
    ax2.set_ylabel('Eficiência (%)', fontweight='bold')
    ax2.set_title('⚡ Eficiência de Paralelização\n(maior é melhor)', fontweight='bold')
    ax2.legend(loc='upper right', fontsize=9)
    ax2.set_ylim(0, 120)
    ax2.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig('grafico2_eficiencia_escalabilidade.png', dpi=150, bbox_inches='tight')
    plt.savefig('grafico2_eficiencia_escalabilidade.pdf', bbox_inches='tight')
    print('✅ Gráfico 2 salvo: grafico2_eficiencia_escalabilidade.png/pdf')
    plt.close()

def grafico3_metricas_comparativas(df_metricas):
    """
    GRÁFICO 3: Comparativo de Métricas de Desempenho
    Radar chart + barras comparando throughput, CPU e tempo de espera.
    """
    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    
    # Limpar nomes das políticas
    df_metricas['Politica_Clean'] = df_metricas['Politica'].str.replace('_', ' ').str.replace('  ', ' ')
    politicas = ['FCFS', 'SJN', 'RR', 'PRIORITY', 'PRIORITY_PREEMPT']
    cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
    
    # Subplot 1: Throughput (proc/s)
    throughputs = df_metricas['Throughput_proc_s'].values
    bars1 = axes[0].barh(range(len(politicas)), throughputs, color=cores, edgecolor='black')
    axes[0].set_yticks(range(len(politicas)))
    axes[0].set_yticklabels(politicas)
    axes[0].set_xlabel('Throughput (processos/segundo)', fontweight='bold')
    axes[0].set_title('🚀 Throughput\n(maior é melhor)', fontweight='bold')
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
    axes[1].set_title('💻 Utilização de CPU\n(maior é melhor)', fontweight='bold')
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
    axes[2].set_title('⏳ Tempo Médio de Espera\n(menor é melhor)', fontweight='bold')
    axes[2].grid(True, axis='x', linestyle='--', alpha=0.7)
    
    # Destacar o melhor (menor tempo)
    min_idx = np.argmin(wait_time)
    bars3[min_idx].set_edgecolor('gold')
    bars3[min_idx].set_linewidth(3)
    
    plt.tight_layout()
    plt.savefig('grafico3_metricas_comparativas.png', dpi=150, bbox_inches='tight')
    plt.savefig('grafico3_metricas_comparativas.pdf', bbox_inches='tight')
    print('✅ Gráfico 3 salvo: grafico3_metricas_comparativas.png/pdf')
    plt.close()

def main():
    print('=' * 60)
    print('  📊 GERADOR DE GRÁFICOS - SIMULADOR VON NEUMANN')
    print('=' * 60)
    print()
    
    # Carregar dados
    print('📂 Carregando dados...')
    multicore, metricas, throughput = carregar_dados()
    print(f'   • escalonadores_multicore.csv: {len(multicore)} linhas')
    print(f'   • metricas_escalonadores.csv: {len(metricas)} linhas')
    print(f'   • throughput_multicore.csv: {len(throughput)} linhas')
    print()
    
    # Gerar gráficos
    print('🎨 Gerando gráficos...')
    print()
    
    grafico1_tempo_execucao_multicore(multicore)
    grafico2_eficiencia_escalabilidade(multicore)
    grafico3_metricas_comparativas(metricas)
    
    print()
    print('=' * 60)
    print('  ✅ TODOS OS GRÁFICOS GERADOS COM SUCESSO!')
    print('=' * 60)
    print()
    print('📁 Arquivos gerados:')
    print('   • grafico1_tempo_multicore.png/pdf')
    print('   • grafico2_eficiencia_escalabilidade.png/pdf')
    print('   • grafico3_metricas_comparativas.png/pdf')
    print()

if __name__ == '__main__':
    main()
