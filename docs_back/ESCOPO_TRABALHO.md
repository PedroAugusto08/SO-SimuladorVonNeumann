# 📋 Escopo do Trabalho - Definição Clara

> **Data:** 13/11/2025  
> **Especificação:** Michel Pires (CEFET-MG)  
> **Prazo:** 06/12/2025

---

## 🎯 Escopo da NOSSA EQUIPE

### ✅ O que VAMOS implementar:

**1. Escalonamento Round Robin (10 pontos)**
- ✅ Round Robin multicore com fila global
- ✅ Dois cenários obrigatórios:
  - **Cenário 1 - Não-Preemptivo:** RR sem quantum (run to completion)
  - **Cenário 2 - Preemptivo:** RR com quantum e context switch

**2. Contribuição para Memória (responsabilidade de outros membros)**
- Cache L1 privada por núcleo (já implementado)
- Sincronização de acesso à RAM compartilhada (já implementado)
- **NÃO faremos:** FIFO/LRU (outro membro da equipe)

**3. Artigo IEEE (10 pontos - responsabilidade compartilhada)**
- Seção de escalonamento Round Robin
- Resultados comparativos
- Gráficos de desempenho

---

## ❌ O que NÃO é nosso escopo

A especificação menciona várias políticas, mas **cada equipe escolhe UMA**:

### Escalonadores que NÃO faremos:
- ❌ First Come, First Served (FCFS) - outra equipe
- ❌ Shortest Job Next (SJN) - outra equipe
- ❌ Prioridade - outra equipe

### Gerenciamento de Memória (outro membro fará):
- ❌ Política FIFO
- ❌ Política LRU
- ❌ Segmentação de memória (modelo Tanenbaum)

> **Esclarecimento:** A especificação diz "diferentes políticas de escalonamento, **como**: FCFS, SJN, **Round Robin**, Prioridade..."
> 
> A palavra "como" indica **exemplos**, não que todas devam ser implementadas.
> Cada equipe escolhe uma ou mais políticas para comparar.

---

## 📊 Divisão de Pontos (30 TOTAL)

### Implementação (20 pontos)

#### Escalonamento (10 pontos) - SEU ESCOPO
- [x] ✅ Arquitetura multicore (2 pts) - FEITO
- [ ] ⏳ Cenário não-preemptivo (2 pts)
- [ ] ⏳ Cenário preemptivo com quantum (2 pts)
- [ ] ⏳ Métricas de desempenho (2 pts)
- [ ] ⏳ Comparação baseline single-core (2 pts)

**Pontuação atual: 2/10**

#### Gerenciamento de Memória (10 pontos) - OUTRO MEMBRO
- [x] ✅ Memória compartilhada (2 pts) - FEITO
- [x] ✅ Cache L1 privada (2 pts) - FEITO
- [ ] ⏳ Política FIFO (3 pts) - **NÃO é seu escopo**
- [ ] ⏳ Política LRU (3 pts) - **NÃO é seu escopo**

**Pontuação atual (equipe): 4/10**

### Artigo IEEE (10 pontos) - RESPONSABILIDADE COMPARTILHADA

- [ ] ⏳ Resumo + Introdução (2 pts)
- [ ] ⏳ Referencial Teórico (2 pts)
- [ ] ⏳ Metodologia (2 pts)
- [ ] ⏳ Resultados com gráficos (3 pts)
- [ ] ⏳ Conclusão + Referências (1 pt)

**Pontuação atual: 0/10**

---

## ✅ Requisitos Obrigatórios (SUA PARTE)

### 1. Dois Cenários de Execução

> **Especificação:** "Devem ser considerados dois cenários experimentais distintos"

#### Cenário 1: Round Robin Não-Preemptivo
```cpp
// Processos executam até conclusão
// Round Robin determina ORDEM, mas sem interrupção
while (ready_queue.size() > 0) {
    Process* p = ready_queue.front();
    ready_queue.pop_front();
    
    execute_to_completion(p);  // Sem quantum
    
    if (!p->finished) {
        ready_queue.push_back(p);  // Apenas se bloqueou I/O
    }
}
```

**Características:**
- Usa fila Round Robin (FIFO)
- Mas SEM quantum (não interrompe)
- Processo roda até terminar ou bloquear I/O

#### Cenário 2: Round Robin Preemptivo
```cpp
// Processos são interrompidos por quantum
while (ready_queue.size() > 0) {
    Process* p = ready_queue.front();
    ready_queue.pop_front();
    
    int cycles = execute_with_quantum(p, QUANTUM);
    
    if (!p->finished) {
        ready_queue.push_back(p);  // Volta pro fim da fila
    }
}
```

**Características:**
- Quantum de 100 ciclos (configurável)
- Interrompe processo ao esgotar quantum
- Context switch: salva e restaura estado

### 2. Carga Inicial Completa

> **Especificação:** "todos os programas pertencentes ao lote devem ser completamente carregados na memória principal **antes do início da execução**"

```cpp
void main() {
    RoundRobinScheduler scheduler(num_cores);
    
    // PASSO 1: Carregar TODO o lote ANTES
    for (auto& json_file : process_batch) {
        PCB* process = load_from_disk(json_file);
        scheduler.add_process(process);
    }
    
    // PASSO 2: SÓ DEPOIS executar
    // "Após essa etapa, não será permitida a chegada de novos processos"
    scheduler.run_until_completion();
}
```

### 3. Baseline Single-Core

> **Especificação:** "deve-se utilizar como baseline a arquitetura single-core previamente desenvolvida"

```bash
# Executar com 1 núcleo
./simulador --cores=1 --scheduler=rr --quantum=100

# Executar com 2 núcleos
./simulador --cores=2 --scheduler=rr --quantum=100

# Executar com 4 núcleos
./simulador --cores=4 --scheduler=rr --quantum=100

# Comparar resultados no artigo
```

### 4. Métricas Obrigatórias

> **Especificação:** "todas as execuções devem gerar arquivos de log"

**Métricas a coletar:**
- Tempo médio de espera
- Tempo médio de retorno (turnaround)
- Utilização média da CPU
- Eficiência por núcleo
- Throughput total

**Salvar em:** `logs/metrics_{scenario}_{cores}.csv`

---

## 📅 Cronograma Focado (Round Robin)

```
Novembro          |  Dezembro
------------------+-----------
13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 01 02 03 04 05 06
✅ ⏳ ⏳ ⏳ ⏳ ⏳ ⏳ -- -- -- -- -- -- -- ⏳ ⏳ ⏳ ⏳ ⏳ ⏳ ⏳ ⏳ ⏳ 🏁

│  │  │     │     │              │                    │        │
│  │  │     │     │              │                    │        └─ ENTREGA
│  │  │     │     │              │                    └────────── Artigo
│  │  │     │     │              └─────────────────────────────── Gráficos
│  │  │     │     └────────────────────────────────────────────── Baseline
│  │  │     └──────────────────────────────────────────────────── Preemptivo
│  │  └────────────────────────────────────────────────────────── Não-Preemptivo
│  └───────────────────────────────────────────────────────────── JSONs
└──────────────────────────────────────────────────────────────── Hoje ✅
```

### Seu foco (Round Robin):
- **13-15 Nov:** ✅ Core + RoundRobinScheduler (FEITO)
- **16-17 Nov:** ⚠️ Cenário não-preemptivo + JSONs de teste
- **18-19 Nov:** ⚠️ Cenário preemptivo + context switch
- **20-23 Nov:** -- (outro membro faz FIFO/LRU)
- **24-26 Nov:** ⚠️ Baseline + testes multicore + logs
- **27-06 Dez:** ⚠️ Artigo (seção de escalonamento + resultados)

---

## 🎯 Ações Imediatas (SUA PARTE)

### Hoje/Amanhã (14-15 Nov):

1. **Criar 5 processos de teste (JSON)**
   - `processo1.json` - CPU-bound (muitas operações)
   - `processo2.json` - I/O-bound (muitos acessos memória)
   - `processo3.json` - Balanceado
   - `processo4.json` - Curto (100 instruções)
   - `processo5.json` - Longo (1000 instruções)

2. **Implementar carga inicial completa**
   ```cpp
   // Em main.cpp
   vector<string> process_files = {
       "processo1.json",
       "processo2.json", 
       "processo3.json",
       "processo4.json",
       "processo5.json"
   };
   
   // Carregar TODOS antes de executar
   for (auto& file : process_files) {
       scheduler.add_process(load_pcb(file));
   }
   
   // Só depois executar
   scheduler.run();
   ```

### Próxima Semana (16-19 Nov):

3. **Cenário Não-Preemptivo**
   - Criar flag `--non-preemptive`
   - Desabilitar quantum
   - Processos rodam até conclusão

4. **Cenário Preemptivo**
   - Garantir quantum funciona
   - Context switch preserva estado
   - Testes com quantum 50, 100, 200

5. **Logging de Métricas**
   - Criar `Logger` class
   - Salvar CSV com resultados
   - Formato: PID, wait_time, turnaround, core, etc.

### Semana 24-26 Nov:

6. **Baseline Single-Core**
   - Testar com `--cores=1`
   - Coletar métricas
   - Comparar com 2 e 4 núcleos

7. **Gráficos**
   - Speedup vs número de núcleos
   - Tempo de espera por processo
   - Utilização de CPU ao longo do tempo

---

## 📞 Divisão com Outros Membros

| Membro | Responsabilidade | Pontos |
|--------|------------------|--------|
| **Você** | Round Robin (não-preemptivo + preemptivo) | 10 |
| Membro 2 | Políticas FIFO/LRU | 10 |
| Membro 3 | Testes + Métricas + Logs | 0 (suporte) |
| Membro 4 | Artigo IEEE | 10 (compartilhado) |

---

## ✅ Checklist (SUA PARTE)

### Implementação Round Robin (10 pts)

- [x] ✅ RoundRobinScheduler criado
- [x] ✅ Fila FIFO implementada
- [x] ✅ Multicore funcionando
- [ ] ⏳ Carga inicial completa
- [ ] ⏳ Cenário não-preemptivo
- [ ] ⏳ Cenário preemptivo
- [ ] ⏳ Context switch funcional
- [ ] ⏳ 5+ processos JSON
- [ ] ⏳ Logging de métricas
- [ ] ⏳ Baseline single-core
- [ ] ⏳ Testes multicore (2, 4 cores)

### Artigo IEEE (contribuição - 3 pts dos 10)

- [ ] ⏳ Seção "Escalonamento Round Robin"
- [ ] ⏳ Pseudocódigo do RR
- [ ] ⏳ Gráficos de desempenho
- [ ] ⏳ Tabelas comparativas
- [ ] ⏳ Discussão dos resultados

---

## 📚 Referências para seu Artigo

Seção de Round Robin deve citar:

1. Tanenbaum, A. S., & Bos, H. (2014). *Modern Operating Systems*. 4th ed.
2. Silberschatz, A., Galvin, P. B., & Gagne, G. (2018). *Operating System Concepts*. 10th ed.
3. Stallings, W. (2018). *Operating Systems: Internals and Design Principles*. 9th ed.

**Foco:** Round Robin em sistemas multicore, impacto do quantum, fairness.

---

**Última atualização:** 13/11/2025  
**Próxima revisão:** 16/11/2025 (após implementar cenário não-preemptivo)
