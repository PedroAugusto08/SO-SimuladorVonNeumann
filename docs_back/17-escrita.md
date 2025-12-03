# Escrita Científica

## 🎯 Objetivo

Guia de boas práticas para escrita científica de artigos em Sistemas Operacionais no formato IEEE.

---

## ✍️ Princípios de Escrita Científica

### 1. Clareza
- Use frases curtas e diretas
- Evite ambiguidades
- Defina termos técnicos

### 2. Objetividade
- Escreva em terceira pessoa
- Evite opiniões pessoais não fundamentadas
- Base afirmações em dados

### 3. Concisão
- Elimine palavras desnecessárias
- Seja direto ao ponto
- Evite redundâncias

### 4. Precisão
- Use termos técnicos corretos
- Seja específico com números
- Cite fontes adequadamente

---

## 📝 Estilo de Escrita IEEE

### Tempo Verbal

**Abstract:** Presente ou passado
```
✅ "This paper presents..." 
✅ "The system was implemented..."
❌ "This paper will present..."
```

**Introduction:** Presente
```
✅ "Multicore systems are prevalent..."
✅ "This work aims to..."
```

**Methodology:** Passado
```
✅ "Experiments were conducted..."
✅ "The system was configured..."
```

**Results:** Passado
```
✅ "Results showed that..."
✅ "Performance improved by 30%"
```

**Conclusion:** Presente
```
✅ "This work demonstrates..."
✅ "The results indicate..."
```

---

### Voz Ativa vs Passiva

**Prefira voz ativa quando possível:**

```
❌ "The experiment was conducted by the authors"
✅ "We conducted the experiment"
✅ "This work presents..." (melhor ainda)
```

**Use passiva para enfatizar a ação:**

```
✅ "Processes were scheduled using Round Robin"
✅ "Memory was allocated in segments of 4KB"
```

---

## 🔤 Vocabulário Técnico

### Palavras de Transição

**Para adicionar informação:**
- Furthermore, Moreover, Additionally, In addition

**Para contrastar:**
- However, Nevertheless, Conversely, On the other hand

**Para exemplificar:**
- For instance, For example, Specifically, In particular

**Para concluir:**
- Therefore, Thus, Consequently, As a result, Hence

### Frases Úteis

**Introdução:**
```
- "This paper addresses..."
- "The main contribution of this work is..."
- "We propose a novel approach to..."
- "The remainder of this paper is organized as follows..."
```

**Metodologia:**
```
- "Experiments were conducted on..."
- "The system was configured with..."
- "Performance was measured using..."
- "Each test was repeated 30 times to ensure..."
```

**Resultados:**
```
- "Results demonstrate that..."
- "As shown in Fig. X, ..."
- "Table Y summarizes..."
- "Performance improved by X% compared to..."
```

**Discussão:**
```
- "These results suggest that..."
- "The observed behavior can be explained by..."
- "Contrary to expectations, ..."
- "This is consistent with previous findings..."
```

**Conclusão:**
```
- "This work presented..."
- "Results indicate that..."
- "Future work will focus on..."
- "The main findings are..."
```

---

## ❌ Erros Comuns

### 1. Palavras Informais

```
❌ "The system works really well"
✅ "The system demonstrates high performance"

❌ "We got good results"
✅ "Results show significant improvement"

❌ "The algorithm is pretty fast"
✅ "The algorithm exhibits low execution time"
```

---

### 2. Redundâncias

```
❌ "Past history"
✅ "History"

❌ "End result"
✅ "Result"

❌ "Future plans"
✅ "Plans"

❌ "Each and every"
✅ "Each" ou "Every"
```

---

### 3. Palavras Vazias

```
❌ "It is important to note that..."
✅ (Simplesmente apresente a informação)

❌ "As a matter of fact..."
✅ (Remova completamente)

❌ "In order to..."
✅ "To..."
```

---

### 4. Uso Incorreto de Artigos

```
❌ "The multicore systems are..."
✅ "Multicore systems are..." (geral)
✅ "The proposed multicore system is..." (específico)
```

---

## 📐 Formatação de Números e Unidades

### Números

```
❌ "4 cores"
✅ "Four cores" (no início de frase)
✅ "The system has 4 cores" (meio de frase)

✅ "We tested 10, 50, and 100 processes"
✅ "Performance improved by 25%"
```

### Unidades

```
✅ "4 KB" (com espaço)
✅ "3.5 GHz"
✅ "10 ms"
❌ "4KB" (sem espaço)
```

### Equações

```latex
% Inline
The speedup is calculated as $S = T_1 / T_n$.

% Display
\begin{equation}
S = \frac{T_1}{T_n}
\label{eq:speedup}
\end{equation}

where $T_1$ is the execution time with one core and $T_n$ is 
the execution time with $n$ cores.
```

---

## 📊 Apresentação de Dados

### Figuras

```latex
\begin{figure}[htbp]
\centerline{\includegraphics[width=0.45\textwidth]{speedup.png}}
\caption{Speedup as a function of the number of cores. The 
dashed line represents ideal speedup.}
\label{fig:speedup}
\end{figure}

As shown in Fig. \ref{fig:speedup}, speedup increases with 
the number of cores...
```

**Boas práticas:**
- Caption descreve o que está sendo mostrado
- Figuras são referenciadas no texto ANTES de aparecerem
- Use "Fig." não "Figure" no texto
- Explique elementos importantes da figura no texto

---

### Tabelas

```latex
\begin{table}[htbp]
\caption{Performance comparison between FIFO and LRU policies}
\begin{center}
\begin{tabular}{|l|c|c|c|}
\hline
\textbf{Metric} & \textbf{FIFO} & \textbf{LRU} & \textbf{Improvement} \\
\hline
Hit Rate (\%) & 75.3 & 85.7 & +10.4 \\
Page Faults & 247 & 143 & -42.1 \\
\hline
\end{tabular}
\label{tab:comparison}
\end{center}
\end{table}

Table \ref{tab:comparison} presents the comparison between...
```

**Boas práticas:**
- Caption vem ANTES da tabela (diferente de figuras)
- Use unidades nos headers
- Alinhe números à direita, texto à esquerda
- Mantenha tabelas simples e legíveis

---

## 📚 Citações

### Formatos de Citação

```latex
% Citação numérica (IEEE)
Multicore systems have become prevalent \cite{hennessy2017}.

% Múltiplas citações
Various works address this topic \cite{tanenbaum2014, 
silberschatz2018, stallings2018}.

% Citação integrada ao texto
As demonstrated by Hennessy and Patterson \cite{hennessy2017}, 
parallelism is essential...
```

### Quando Citar

✅ **Sempre cite:**
- Ideias de outros autores
- Dados ou estatísticas externas
- Algoritmos ou métodos existentes
- Trabalhos relacionados
- Fundamentação teórica

❌ **Não precisa citar:**
- Conhecimento comum da área
- Suas próprias contribuições
- Resultados seus

---

## ✅ Checklist de Qualidade

### Conteúdo
- [ ] Contribuições claramente identificadas
- [ ] Resultados suportados por dados
- [ ] Limitações reconhecidas
- [ ] Trabalhos futuros definidos
- [ ] Todas as afirmações fundamentadas

### Estrutura
- [ ] Abstract completo e informativo
- [ ] Introdução motiva o problema
- [ ] Metodologia reproduzível
- [ ] Resultados apresentados claramente
- [ ] Conclusão resume contribuições

### Escrita
- [ ] Gramática correta
- [ ] Ortografia verificada
- [ ] Termos técnicos corretos
- [ ] Transições entre parágrafos
- [ ] Voz ativa predominante

### Formatação
- [ ] Formato IEEE correto
- [ ] Figuras em alta resolução
- [ ] Tabelas formatadas corretamente
- [ ] Equações numeradas
- [ ] Referências completas

### Figuras e Tabelas
- [ ] Todas referenciadas no texto
- [ ] Captions descritivas
- [ ] Legíveis e claras
- [ ] Consistentes entre si

---

## 🔍 Revisão em Camadas

### 1ª Leitura: Conteúdo
- Argumentos fazem sentido?
- Lógica está correta?
- Dados suportam conclusões?

### 2ª Leitura: Estrutura
- Seções bem organizadas?
- Transições suaves?
- Informação na seção correta?

### 3ª Leitura: Estilo
- Escrita clara e concisa?
- Tom apropriado?
- Termos consistentes?

### 4ª Leitura: Detalhes
- Gramática correta?
- Ortografia correta?
- Formatação consistente?

---

## 📖 Exemplos de Parágrafos Bem Escritos

### Parágrafo de Introdução

```
Multicore processors have become ubiquitous in modern computing 
systems, from mobile devices to data centers [1]. However, 
effectively utilizing multiple cores requires sophisticated 
operating system support for process scheduling and resource 
management [2]. Round Robin (RR) scheduling is a widely-used 
algorithm due to its simplicity and fairness [3], yet its 
performance in multicore environments depends on proper 
configuration of the time quantum and load balancing strategies. 
This work presents an educational simulator that implements 
multicore RR scheduling with configurable parameters, enabling 
empirical study of design tradeoffs.
```

**Por que é bom:**
- Contextualiza (primeira frase)
- Identifica problema (segunda frase)
- Menciona solução existente (terceira frase)
- Apresenta contribuição (última frase)
- Cita referências apropriadamente

---

### Parágrafo de Resultados

```
Fig. 3 shows the speedup achieved with 1, 2, 4, and 8 cores. 
Performance scales nearly linearly up to 4 cores, achieving a 
speedup of 3.3x with an efficiency of 82%. With 8 cores, speedup 
reaches 5.5x but efficiency drops to 69%, indicating increased 
synchronization overhead. These results are consistent with 
Amdahl's Law [15], which predicts sublinear speedup due to 
sequential portions of the workload. The observed efficiency is 
comparable to commercial operating systems [16], suggesting the 
simulator accurately models real-world behavior.
```

**Por que é bom:**
- Referencia figura
- Apresenta dados específicos
- Interpreta resultados
- Compara com teoria/trabalhos anteriores
- Valida a implementação

---

## 🔗 Ferramentas Úteis

### Verificadores de Gramática
- Grammarly
- LanguageTool
- ProWritingAid

### LaTeX
- Overleaf (editor online)
- TeXstudio (desktop)
- VS Code + LaTeX Workshop

### Gerenciamento de Referências
- Zotero
- Mendeley
- JabRef

---

## 📚 Leitura Recomendada

- GLASS, R. et al. How to Write a Good Scientific Paper
- WHITESIDES, G. M. Writing a Paper (2004)
- IEEE Editorial Style Manual

---

## 🔗 Próximos Passos

- ➡️ [Bibliografia](18-bibliografia.md)
- ➡️ [FAQ](20-faq.md)
