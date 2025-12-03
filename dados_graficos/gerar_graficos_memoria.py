#!/usr/bin/env python3
"""
Gerador de Gráficos de Memória - Simulador Von Neumann
Gera visualizações de Cache Hits, Cache Misses e Uso de Memória Temporal.
"""

import matplotlib
matplotlib.use('Agg')  # Backend sem GUI

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob

# Configuração de estilo
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['figure.figsize'] = (14, 8)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12

# Cores para cada política (mesmas do script principal)
CORES_POLITICAS = {
    'FCFS': '#3498db',         # Azul
    'SJN': '#e74c3c',          # Vermelho
    'RR': '#2ecc71',           # Verde
    'PRIORITY': '#9b59b6',     # Roxo
    'PRIORITY_PREEMPT': '#f39c12'  # Laranja
}

def mapear_nome_politica(nome_arquivo):
    """
    Mapeia o nome do arquivo para o nome simplificado da política.
    """
    if 'FCFS' in nome_arquivo:
        return 'FCFS'
    elif 'SJN' in nome_arquivo:
        return 'SJN'
    elif 'Round_Robin' in nome_arquivo:
        return 'RR'
    elif 'PRIORITY_PREEMPTIVO' in nome_arquivo:
        return 'PRIORITY_PREEMPT'
    elif 'PRIORITY' in nome_arquivo:
        return 'PRIORITY'
    return 'UNKNOWN'

def carregar_dados_memoria():
    """
    Carrega todos os CSVs de memória e extrai métricas finais e temporais.
    """
    arquivos = glob.glob('memoria_*.csv')
    
    if not arquivos:
        print('❌ Nenhum arquivo memoria_*.csv encontrado!')
        return None, None
    
    dados = []
    dados_temporais = {}
    
    for arquivo in arquivos:
        politica = mapear_nome_politica(arquivo)
        
        try:
            df = pd.read_csv(arquivo)
            
            # Armazenar dados temporais completos
            dados_temporais[politica] = df
            
            # Pegar última linha (estado final)
            ultima_linha = df.iloc[-1]
            
            cache_hits = int(ultima_linha['cache_hits'])
            cache_misses = int(ultima_linha['cache_misses'])
            hit_rate = float(ultima_linha['hit_rate'])
            
            dados.append({
                'Politica': politica,
                'Cache_Hits': cache_hits,
                'Cache_Misses': cache_misses,
                'Hit_Rate': hit_rate,
                'Total_Acessos': cache_hits + cache_misses
            })
            
            print(f'   ✓ {arquivo}: {cache_hits} hits, {cache_misses} misses, {len(df)} timestamps')
            
        except Exception as e:
            print(f'   ⚠️  Erro ao processar {arquivo}: {e}')
    
    return pd.DataFrame(dados), dados_temporais

def grafico1_cache_hits(df):
    """
    GRÁFICO 1: Cache Hits por Escalonador
    Barras verticais mostrando número de acertos de cache.
    """
    fig, ax = plt.subplots(figsize=(12, 7))
    
    # Ordenar por número de hits decrescente
    df_sorted = df.sort_values('Cache_Hits', ascending=False)
    
    politicas = df_sorted['Politica'].values
    hits = df_sorted['Cache_Hits'].values
    
    x = np.arange(len(politicas))
    width = 0.6
    
    # Cores personalizadas para cada política
    cores = [CORES_POLITICAS.get(p, '#95a5a6') for p in politicas]
    
    bars = ax.bar(x, hits, width, color=cores, edgecolor='black', linewidth=1.5)
    
    # Adicionar valores nas barras
    for bar, hit in zip(bars, hits):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width() / 2, height,
                f'{int(hit)}',
                ha='center', va='bottom', fontweight='bold', fontsize=12)
    
    # Destacar o melhor (maior número de hits)
    bars[0].set_edgecolor('gold')
    bars[0].set_linewidth(3)
    
    ax.set_xlabel('Política de Escalonamento', fontweight='bold', fontsize=12)
    ax.set_ylabel('Número de Cache Hits', fontweight='bold', fontsize=12)
    ax.set_title('Cache Hits por Escalonador\n(maior é melhor - dados encontrados na cache)', 
                 fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels(politicas, fontsize=11)
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    ax.set_ylim(0, max(hits) * 1.15)
    
    # Adicionar linha de média
    media = hits.mean()
    ax.axhline(y=media, color='red', linestyle='--', alpha=0.7, 
               linewidth=2, label=f'Média: {media:.1f}')
    ax.legend(loc='upper right', fontsize=10)
    
    # Anotação explicativa
    ax.annotate('Borda dourada = melhor desempenho\nMais hits = melhor localidade temporal',
               xy=(0.02, 0.98), xycoords='axes fraction',
               fontsize=10, ha='left', va='top',
               bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.9))
    
    plt.tight_layout()
    plt.savefig('grafico_memoria1_cache_hits.png', dpi=150, bbox_inches='tight')
    plt.savefig('grafico_memoria1_cache_hits.pdf', bbox_inches='tight')
    print('✅ Gráfico 1 salvo: grafico_memoria1_cache_hits.png/pdf')
    plt.close()

def grafico2_cache_misses(df):
    """
    GRÁFICO 2: Cache Misses por Escalonador
    Barras verticais mostrando número de falhas de cache.
    """
    fig, ax = plt.subplots(figsize=(12, 7))
    
    # Ordenar por número de misses crescente (menor é melhor)
    df_sorted = df.sort_values('Cache_Misses', ascending=True)
    
    politicas = df_sorted['Politica'].values
    misses = df_sorted['Cache_Misses'].values
    
    x = np.arange(len(politicas))
    width = 0.6
    
    # Usar vermelho para misses (negativo)
    cores = ['#e74c3c' for _ in politicas]
    
    bars = ax.bar(x, misses, width, color=cores, edgecolor='black', linewidth=1.5, alpha=0.8)
    
    # Adicionar valores nas barras
    for bar, miss in zip(bars, misses):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width() / 2, height,
                f'{int(miss)}',
                ha='center', va='bottom', fontweight='bold', fontsize=12, color='black')
    
    # Destacar o melhor (menor número de misses)
    bars[0].set_edgecolor('gold')
    bars[0].set_linewidth(3)
    
    ax.set_xlabel('Política de Escalonamento', fontweight='bold', fontsize=12)
    ax.set_ylabel('Número de Cache Misses', fontweight='bold', fontsize=12)
    ax.set_title('Cache Misses por Escalonador\n(menor é melhor - dados não encontrados na cache)', 
                 fontweight='bold', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels(politicas, fontsize=11)
    ax.grid(True, axis='y', linestyle='--', alpha=0.5)
    ax.set_ylim(0, max(misses) * 1.15)
    
    # Adicionar linha de média
    media = misses.mean()
    ax.axhline(y=media, color='darkred', linestyle='--', alpha=0.7, 
               linewidth=2, label=f'Média: {media:.1f}')
    ax.legend(loc='upper right', fontsize=10)
    
    # Anotação explicativa
    ax.annotate('Borda dourada = melhor desempenho\nMenos misses = menos acessos à RAM',
               xy=(0.02, 0.98), xycoords='axes fraction',
               fontsize=10, ha='left', va='top',
               bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.9))
    
    plt.tight_layout()
    plt.savefig('grafico_memoria2_cache_misses.png', dpi=150, bbox_inches='tight')
    plt.savefig('grafico_memoria2_cache_misses.pdf', bbox_inches='tight')
    print('✅ Gráfico 2 salvo: grafico_memoria2_cache_misses.png/pdf')
    plt.close()

def grafico3_uso_memoria_temporal(dados_temporais):
    """
    GRÁFICO 3: Uso de Memória Principal ao Longo do Tempo
    Gráfico de linhas mostrando evolução temporal do uso de RAM.
    """
    fig, ax = plt.subplots(figsize=(14, 7))
    
    # Ordenar políticas para consistência
    politicas_ordenadas = ['FCFS', 'SJN', 'RR', 'PRIORITY', 'PRIORITY_PREEMPT']
    
    # Marcadores diferentes para cada política
    marcadores = ['o', 's', '^', 'D', 'v']
    estilos_linha = ['-', '--', '-.', ':', '-']
    
    for idx, politica in enumerate(politicas_ordenadas):
        if politica not in dados_temporais:
            continue
            
        df = dados_temporais[politica]
        cor = CORES_POLITICAS.get(politica, '#95a5a6')
        marcador = marcadores[idx % len(marcadores)]
        estilo = estilos_linha[idx % len(estilos_linha)]
        
        # Plotar linha com marcadores espaçados (a cada N pontos para não poluir)
        markevery = max(1, len(df)//10)
        ax.plot(df['timestamp_ms'], df['main_memory_bytes'], 
                linestyle=estilo, color=cor, linewidth=2.5, 
                marker=marcador, markersize=8, markevery=markevery,
                markeredgecolor='black', markeredgewidth=0.5,
                label=politica, alpha=0.9)
    
    ax.set_xlabel('Tempo (ms)', fontweight='bold', fontsize=12)
    ax.set_ylabel('Memória Principal (bytes)', fontweight='bold', fontsize=12)
    ax.set_title('Evolução do Uso de Memória Principal ao Longo do Tempo\n(por escalonador)', 
                 fontweight='bold', fontsize=14)
    ax.legend(loc='best', fontsize=11, framealpha=0.95)
    ax.grid(True, linestyle='--', alpha=0.5)
    
    # Adicionar anotação explicativa
    ax.annotate('Mostra como cada escalonador\nconsome memória durante execução',
               xy=(0.02, 0.98), xycoords='axes fraction',
               fontsize=10, ha='left', va='top',
               bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.9))
    
    plt.tight_layout()
    plt.savefig('grafico_memoria3_uso_temporal.png', dpi=150, bbox_inches='tight')
    plt.savefig('grafico_memoria3_uso_temporal.pdf', bbox_inches='tight')
    print('✅ Gráfico 3 salvo: grafico_memoria3_uso_temporal.png/pdf')
    plt.close()

def main():
    print('=' * 70)
    print('  📊 GERADOR DE GRÁFICOS DE MEMÓRIA - SIMULADOR VON NEUMANN')
    print('=' * 70)
    print()
    
    # Carregar dados
    print('📂 Carregando dados de memória...')
    df, dados_temporais = carregar_dados_memoria()
    
    if df is None or df.empty:
        print('❌ Nenhum dado encontrado. Verifique os arquivos memoria_*.csv')
        return
    
    print(f'\n   Total de escalonadores: {len(df)}')
    print()
    
    # Gerar gráficos
    print('🎨 Gerando gráficos de memória...')
    print()
    
    grafico1_cache_hits(df)
    grafico2_cache_misses(df)
    grafico3_uso_memoria_temporal(dados_temporais)
    
    print()
    print('=' * 70)
    print('  ✅ GRÁFICOS DE MEMÓRIA GERADOS COM SUCESSO!')
    print('=' * 70)
    print()
    print('📁 Arquivos gerados:')
    print('   • grafico_memoria1_cache_hits.png/pdf - Cache Hits por escalonador')
    print('   • grafico_memoria2_cache_misses.png/pdf - Cache Misses por escalonador')
    print('   • grafico_memoria3_uso_temporal.png/pdf - Uso de memória ao longo do tempo')
    print()
    
    # Exibir resumo estatístico
    print('📈 RESUMO ESTATÍSTICO:')
    print()
    for _, row in df.sort_values('Hit_Rate', ascending=False).iterrows():
        print(f"   {row['Politica']:20s} | Hits: {row['Cache_Hits']:6d} | "
              f"Misses: {row['Cache_Misses']:6d} | Hit Rate: {row['Hit_Rate']:6.2f}%")
    print()

if __name__ == '__main__':
    main()
