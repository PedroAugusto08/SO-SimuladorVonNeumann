# Estrutura do Artigo IEEE

## 🎯 Objetivo

Guia completo para estruturar e escrever o artigo científico no formato IEEE sobre o simulador multicore.

---

## 📄 Especificações IEEE

### Formato
- **Template:** IEEE Conference Template (2-column)
- **Páginas:** 6-8 páginas
- **Fonte:** Times New Roman, 10pt
- **Margens:** 0.75" (1.9cm) todas
- **Espaçamento:** Single

### Download do Template
- LaTeX: https://www.ieee.org/conferences/publishing/templates.html
- Word: https://www.ieee.org/conferences/publishing/templates.html

---

## 📋 Estrutura Completa

### 1. Título e Autores

```latex
\title{Simulador de Arquitetura Multicore com Escalonador Round Robin e Gerenciamento de Memória Segmentada}

\author{
\IEEEauthorblockN{Nome Autor 1, Nome Autor 2, Nome Autor 3, Nome Autor 4}
\IEEEauthorblockA{
Centro Federal de Educação Tecnológica de Minas Gerais\\
Campus V - Divinópolis, MG, Brasil\\
\{autor1, autor2, autor3, autor4\}@cefetmg.br
}
}
```

**Dicas:**
- Título claro e descritivo
- Máximo 12-15 palavras
- Incluir palavras-chave principais
- Nomes completos dos autores
- Afiliação institucional

---

### 2. Abstract (Resumo)

**Estrutura (150-200 palavras):**

```
[Contexto] 
Este trabalho apresenta...

[Problema]
A necessidade de...

[Solução]
Foi desenvolvido um simulador...

[Metodologia]
O sistema implementa...

[Resultados]
Os experimentos demonstraram...

[Conclusão]
Os resultados indicam que...
```

**Exemplo:**

```
Este trabalho apresenta o desenvolvimento de um simulador de 
arquitetura multicore com escalonador Round Robin preemptivo e 
gerenciamento de memória segmentada. O simulador foi construído 
sobre uma base existente de arquitetura Von Neumann single-core, 
expandindo-a para suportar múltiplos núcleos de processamento 
executando concorrentemente. O sistema implementa políticas de 
substituição de memória (FIFO e LRU), sincronização entre cores 
usando mutexes e condition variables, e coleta detalhada de 
métricas de desempenho. Experimentos foram conduzidos com 
diferentes configurações (2, 4 e 8 cores) e cargas de trabalho 
variadas. Os resultados demonstraram speedup de até 5.5x com 8 
cores e eficiência de 69%, com a política LRU apresentando taxa 
de acerto 10% superior ao FIFO. O simulador demonstrou 
escalabilidade adequada e pode ser utilizado para estudos de 
arquiteturas paralelas e algoritmos de escalonamento.
```

---

### 3. Keywords

**5-7 palavras-chave:**

```latex
\begin{IEEEkeywords}
Sistemas Operacionais, Multicore, Escalonamento Round Robin, 
Gerenciamento de Memória, Segmentação, Simulação
\end{IEEEkeywords}
```

---

### 4. Introduction (1-1.5 páginas)

#### Seção I. INTRODUÇÃO

**Estrutura:**

**A. Contexto Geral**
```
- Importância de sistemas multicore
- Desafios de programação paralela
- Relevância de simuladores
```

**B. Motivação**
```
- Por que este trabalho é necessário?
- Que problema resolve?
- Qual a contribuição?
```

**C. Objetivos**
```
- Objetivo geral
- Objetivos específicos (3-5 itens)
```

**D. Organização do Artigo**
```
"Este artigo está organizado da seguinte forma: 
Seção II apresenta trabalhos relacionados; 
Seção III descreve a arquitetura do simulador; 
Seção IV detalha a metodologia experimental; 
Seção V apresenta e discute os resultados; 
Seção VI conclui o trabalho."
```

**Exemplo de Introdução:**

```latex
\section{Introdução}

Arquiteturas multicore tornaram-se predominantes em sistemas 
computacionais modernos, desde dispositivos móveis até 
supercomputadores \cite{hennessy2017}. No entanto, o 
aproveitamento eficaz de múltiplos núcleos requer sistemas 
operacionais capazes de gerenciar recursos compartilhados e 
escalonar processos adequadamente \cite{tanenbaum2014}.

Este trabalho apresenta o desenvolvimento de um simulador 
educacional de arquitetura multicore que implementa escalonamento 
Round Robin e gerenciamento de memória segmentada. O simulador 
foi construído expandindo um simulador Von Neumann existente, 
permitindo estudar empiricamente o impacto de diferentes 
políticas de escalonamento e gerenciamento de memória no 
desempenho do sistema.

\subsection{Objetivos}

O objetivo geral é desenvolver e avaliar um simulador multicore 
funcional. Os objetivos específicos incluem:

\begin{itemize}
\item Implementar arquitetura multicore com N núcleos
\item Desenvolver escalonador Round Robin preemptivo
\item Implementar gerenciamento de memória segmentada
\item Comparar políticas FIFO e LRU
\item Avaliar speedup e eficiência multicore
\end{itemize}
```

---

### 5. Related Work (0.5-1 página)

#### Seção II. TRABALHOS RELACIONADOS

**Estrutura:**

```
- Simuladores existentes (SimpleScalar, Gem5, etc.)
- Trabalhos sobre escalonamento multicore
- Estudos de gerenciamento de memória
- Como este trabalho se diferencia
```

**Exemplo:**

```latex
\section{Trabalhos Relacionados}

Diversos simuladores de arquitetura foram desenvolvidos para 
fins educacionais e de pesquisa. O SimpleScalar \cite{burger1997} 
é um simulador amplamente usado que modela pipelines complexos, 
porém com foco em single-core. Gem5 \cite{binkert2011} oferece 
simulação detalhada de sistemas multicore, mas possui 
complexidade elevada para fins didáticos.

Em \cite{stallings2018}, são discutidas diversas políticas de 
escalonamento, incluindo Round Robin e suas variantes. O autor 
destaca que quantum adequado é crucial para balancear overhead 
de context switch e responsividade.

Quanto ao gerenciamento de memória, \cite{silberschatz2018} 
compara políticas de substituição, demonstrando superioridade 
de LRU sobre FIFO em cenários típicos, corroborando nossos 
resultados experimentais.

Este trabalho diferencia-se por focar em simplicidade e 
didática, mantendo fidelidade aos conceitos fundamentais de 
SO enquanto oferece ambiente prático para experimentação.
```

---

### 6. Architecture (2-3 páginas)

#### Seção III. ARQUITETURA DO SIMULADOR

**Subseções:**

**A. Visão Geral**
```
- Diagrama de blocos do sistema
- Componentes principais
- Fluxo de dados
```

**B. Arquitetura Multicore**
```
- Estrutura de cores
- Comunicação inter-core
- Sincronização
```

**C. Escalonador Round Robin**
```
- Algoritmo implementado
- Estrutura de filas
- Quantum e preempção
```

**D. Gerenciamento de Memória**
```
- Segmentação
- Políticas FIFO e LRU
- Tratamento de falhas
```

**E. Coleta de Métricas**
```
- Métricas implementadas
- Instrumentação do código
```

**Exemplo com Figura:**

```latex
\section{Arquitetura do Simulador}

\subsection{Visão Geral}

O simulador foi desenvolvido em C++17 e organizado em 
componentes modulares conforme ilustrado na Fig. \ref{fig:arch}.

\begin{figure}[htbp]
\centerline{\includegraphics[width=0.45\textwidth]{architecture.png}}
\caption{Arquitetura geral do simulador multicore.}
\label{fig:arch}
\end{figure}

O sistema consiste de: (i) N cores de processamento, cada um 
com pipeline de 5 estágios; (ii) escalonador global Round Robin; 
(iii) gerenciador de memória segmentada; (iv) coletor de métricas.

\subsection{Arquitetura Multicore}

Cada core implementa um pipeline MIPS simplificado com os 
estágios IF, ID, EX, MEM e WB. Os cores compartilham acesso à 
memória principal através de um barramento sincronizado com 
mutex, prevenindo race conditions.

A sincronização entre cores é realizada através de:
\begin{itemize}
\item Mutex global para fila de processos
\item Condition variables para notificação
\item Atomic operations para contadores
\end{itemize}
```

---

### 7. Methodology (1-1.5 páginas)

#### Seção IV. METODOLOGIA

**Estrutura:**

**A. Ambiente Experimental**
```
- Hardware usado
- Software e compiladores
- Configurações do sistema
```

**B. Workloads**
```
- Descrição dos processos
- Conjuntos de teste
- Parâmetros variados
```

**C. Métricas Coletadas**
```
- Lista de métricas
- Como foram calculadas
- Critérios de avaliação
```

**D. Experimentos Realizados**
```
- Experimento 1: Escalabilidade (2, 4, 8 cores)
- Experimento 2: Comparação FIFO vs LRU
- Experimento 3: Variação de quantum
```

**Exemplo com Tabela:**

```latex
\section{Metodologia}

\subsection{Ambiente Experimental}

Os experimentos foram executados em um sistema com as 
especificações listadas na Tabela \ref{tab:setup}.

\begin{table}[htbp]
\caption{Configuração do Ambiente Experimental}
\begin{center}
\begin{tabular}{|l|l|}
\hline
\textbf{Componente} & \textbf{Especificação} \\
\hline
Processador & Intel Core i7-9700K @ 3.6GHz \\
Memória RAM & 16GB DDR4 \\
SO & Ubuntu 22.04 LTS \\
Compilador & GCC 11.3.0 \\
Flags & -O3 -std=c++17 -pthread \\
\hline
\end{tabular}
\label{tab:setup}
\end{center}
\end{table}

\subsection{Workloads}

Três conjuntos de processos foram utilizados:
\begin{itemize}
\item \textit{Low}: 10 processos, burst 50-150ms
\item \textit{Medium}: 50 processos, burst 100-300ms
\item \textit{High}: 100 processos, burst 150-500ms
\end{itemize}

Cada experimento foi repetido 30 vezes e os resultados 
apresentados correspondem à média com intervalo de confiança 
de 95\%.
```

---

### 8. Results (2-3 páginas)

#### Seção V. RESULTADOS E DISCUSSÃO

**Estrutura:**

**A. Escalabilidade Multicore**
```
- Gráfico: Speedup vs Número de Cores
- Gráfico: Eficiência vs Número de Cores
- Análise e discussão
```

**B. Comparação de Políticas de Memória**
```
- Gráfico: Taxa de Acerto FIFO vs LRU
- Tabela: Swaps e Page Faults
- Análise
```

**C. Impacto do Quantum**
```
- Gráfico: Turnaround Time vs Quantum
- Discussão
```

**D. Análise de Overhead**
```
- Context switches
- Contenção em locks
```

**Exemplo com Gráficos:**

```latex
\section{Resultados e Discussão}

\subsection{Escalabilidade Multicore}

A Fig. \ref{fig:speedup} apresenta o speedup obtido variando 
o número de cores de 1 a 8.

\begin{figure}[htbp]
\centerline{\includegraphics[width=0.45\textwidth]{speedup.png}}
\caption{Speedup em função do número de cores.}
\label{fig:speedup}
\end{figure}

Observa-se que o speedup aumenta com o número de cores, 
alcançando 5.5x com 8 cores. A eficiência de 69\% indica overhead 
moderado de sincronização, consistente com implementações reais 
\cite{tanenbaum2014}.

\subsection{Comparação FIFO vs LRU}

A Tabela \ref{tab:policies} compara as políticas de substituição.

\begin{table}[htbp]
\caption{Comparação de Políticas de Substituição}
\begin{center}
\begin{tabular}{|l|c|c|c|}
\hline
\textbf{Métrica} & \textbf{FIFO} & \textbf{LRU} & \textbf{Melhoria} \\
\hline
Hit Rate (\%) & 75.3 & 85.7 & +10.4\% \\
Page Faults & 247 & 143 & -42.1\% \\
Swaps & 156 & 98 & -37.2\% \\
Tempo (s) & 12.4 & 10.2 & -17.7\% \\
\hline
\end{tabular}
\label{tab:policies}
\end{center}
\end{table}

LRU demonstrou superioridade em todas as métricas, justificando 
sua adoção em sistemas reais apesar da complexidade adicional.
```

---

### 9. Conclusion (0.5-1 página)

#### Seção VI. CONCLUSÃO

**Estrutura:**

**A. Síntese**
```
- Recapitular objetivos
- Resumir o que foi feito
```

**B. Principais Resultados**
```
- Destacar descobertas principais
- Validações obtidas
```

**C. Limitações**
```
- Reconhecer limitações do trabalho
- Simplificações adotadas
```

**D. Trabalhos Futuros**
```
- 3-5 possíveis extensões
- Melhorias identificadas
```

**Exemplo:**

```latex
\section{Conclusão}

Este trabalho apresentou o desenvolvimento e avaliação de um 
simulador educacional de arquitetura multicore com escalonamento 
Round Robin e gerenciamento de memória segmentada.

Os experimentos demonstraram que o sistema alcança speedup de 
5.5x com 8 cores e eficiência de 69\%, valores compatíveis com 
sistemas reais considerando o overhead de sincronização. A 
política LRU mostrou-se superior ao FIFO, com taxa de acerto 
10.4\% maior e 42.1\% menos page faults.

O simulador atingiu os objetivos propostos, oferecendo plataforma 
didática para estudo de conceitos de sistemas operacionais. Como 
trabalhos futuros, propõe-se:

\begin{itemize}
\item Implementar escalonadores alternativos (prioridades, CFS)
\item Adicionar suporte a threads
\item Simular cache compartilhada L2
\item Implementar migração de processos entre cores
\item Desenvolver interface gráfica para visualização
\end{itemize}
```

---

### 10. References

```latex
\begin{thebibliography}{00}

\bibitem{hennessy2017} 
J. L. Hennessy and D. A. Patterson, 
\textit{Computer Architecture: A Quantitative Approach}, 
6th ed. Morgan Kaufmann, 2017.

\bibitem{tanenbaum2014}
A. S. Tanenbaum and H. Bos,
\textit{Modern Operating Systems},
4th ed. Pearson, 2014.

\bibitem{silberschatz2018}
A. Silberschatz, P. B. Galvin, and G. Gagne,
\textit{Operating System Concepts},
10th ed. Wiley, 2018.

\bibitem{stallings2018}
W. Stallings,
\textit{Operating Systems: Internals and Design Principles},
9th ed. Pearson, 2018.

\end{thebibliography}
```

---

## ✅ Checklist Final

Antes de submeter:

- [ ] Formato IEEE correto
- [ ] 6-8 páginas
- [ ] Abstract com 150-200 palavras
- [ ] 5-7 keywords
- [ ] Todas as figuras referenciadas no texto
- [ ] Todas as tabelas com caption
- [ ] Referências no formato IEEE
- [ ] Revisão ortográfica e gramatical
- [ ] Equações numeradas
- [ ] Código fonte formatado (se incluído)
- [ ] Verificação de plágio
- [ ] PDF/A compatível

---

## 📚 Referências Úteis

- IEEE Author Center: https://journals.ieeeauthorcenter.ieee.org/
- IEEE Citation Guidelines: https://ieee-dataport.org/sites/default/files/analysis/27/IEEE%20Citation%20Guidelines.pdf
- Overleaf IEEE Template: https://www.overleaf.com/latex/templates/ieee-conference-template/grfzhhncsfqn

---

## 🔗 Próximos Passos

- ➡️ [Resultados e Gráficos](16-resultados.md)
- ➡️ [Escrita Científica](17-escrita.md)
