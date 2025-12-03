# FAQ - Perguntas Frequentes

## 🎯 Perguntas Gerais

### Qual é o objetivo deste trabalho?

Desenvolver um simulador educacional de arquitetura multicore que implementa escalonamento Round Robin e gerenciamento de memória segmentada, permitindo estudar empiricamente conceitos de Sistemas Operacionais.

---

### Quantas páginas deve ter o artigo?

6-8 páginas no formato IEEE Conference (2 colunas).

---

### Qual a data de entrega?

06/12/2025

---

### Qual a pontuação do trabalho?

30 pontos total:
- 20 pontos pela implementação
- 10 pontos pelo artigo IEEE

---

## 🔧 Perguntas Técnicas

### Quantos cores devo implementar?

O sistema deve suportar N cores configurável. Recomenda-se testar com 2, 4 e 8 cores para análise comparativa.

---

### Qual quantum usar no Round Robin?

Não há valor fixo. Você deve experimentar diferentes valores (5, 10, 20, 50, 100 ciclos) e analisar o impacto no desempenho.

---

### Devo implementar FIFO e LRU?

Sim, ambas as políticas de substituição de memória devem ser implementadas para comparação.

---

### Preciso implementar cache L2/L3?

Não é obrigatório. O foco é em multicore, escalonamento e gerenciamento de memória. Cache adicional é uma extensão opcional.

---

### Como sincronizar os cores?

Use mutexes para proteger estruturas compartilhadas (fila de processos, memória) e condition variables para notificação entre cores.

---

## 💻 Perguntas de Implementação

### Posso usar C++ moderno (C++17/20)?

Sim, C++17 é recomendado. Use `std::thread`, `std::mutex`, smart pointers, etc.

---

### Preciso usar threads reais?

Sim, cada core deve ser uma thread C++ (`std::thread`) executando concorrentemente.

---

### Como carregar processos?

Recomenda-se usar JSON para facilitar configuração. Use bibliotecas como `nlohmann/json`.

---

### Como coletar métricas?

Implemente uma classe `MetricsCollector` que registra eventos (início de processo, fim, context switch, etc.) de forma thread-safe.

---

### Como evitar race conditions?

- Proteja todas as estruturas compartilhadas com mutexes
- Use `std::atomic` para contadores simples
- Teste com ThreadSanitizer: `g++ -fsanitize=thread`

---

## 📊 Perguntas sobre Testes

### Quantos processos usar nos testes?

Varie a carga:
- Baixa: 10-20 processos
- Média: 50-100 processos
- Alta: 200-500 processos

---

### Como verificar memory leaks?

```bash
valgrind --leak-check=full ./simulador
```

---

### Como detectar deadlocks?

Use ThreadSanitizer ou implemente timeouts em todas as operações bloqueantes.

---

### Quantas vezes repetir experimentos?

Mínimo 30 repetições para ter significância estatística. Reporte média e desvio padrão.

---

## 📝 Perguntas sobre o Artigo

### Posso escrever em português?

Verifique com o professor. Geralmente artigos IEEE são em inglês, mas pode haver exceção para trabalhos acadêmicos.

---

### Preciso de Abstract e Keywords?

Sim, ambos são obrigatórios no formato IEEE.

---

### Quantas referências preciso?

Mínimo 10-15 referências, misturando livros-texto e artigos científicos.

---

### Como fazer os gráficos?

Use Python com matplotlib/seaborn para gerar gráficos em alta resolução (300 DPI, formato PDF ou PNG).

---

### Preciso comparar com trabalhos existentes?

Sim, seção de "Trabalhos Relacionados" é obrigatória. Compare com outros simuladores (SimpleScalar, Gem5) e destaque diferenciais.

---

## 🐛 Problemas Comuns

### Sistema trava ao executar

**Possíveis causas:**
- Deadlock (verificar ordem de locks)
- Busy waiting excessivo
- Fila de processos vazia mas cores aguardando

**Solução:**
```cpp
// Use condition variables ao invés de busy waiting
cv.wait(lock, [this]{ return !queue.empty() || !running; });
```

---

### Speedup muito baixo

**Possíveis causas:**
- Overhead de sincronização alto
- Seções críticas muito grandes
- Processos muito curtos (quantum > burst time)

**Solução:**
- Reduza tamanho das seções críticas
- Use locks mais granulares
- Aumente tamanho dos processos de teste

---

### Memory leaks detectados

**Solução:**
```cpp
// Use smart pointers
std::unique_ptr<Core> core = std::make_unique<Core>(0);
std::shared_ptr<PCB> process = std::make_shared<PCB>();
```

---

### Compilação falha

**Erro comum:**
```
error: 'thread' is not a member of 'std'
```

**Solução:**
```bash
# Adicionar flag -pthread
g++ -std=c++17 -pthread main.cpp -o simulador
```

---

## 🎓 Perguntas de Conceitos

### Qual a diferença entre paralelismo e concorrência?

- **Concorrência:** Múltiplas tarefas progridem no mesmo período (podem se intercalar)
- **Paralelismo:** Múltiplas tarefas executam simultaneamente (requer múltiplos cores)

---

### O que é speedup ideal?

Speedup ideal = N (número de cores). Na prática, sempre menor devido a overhead de sincronização e partes sequenciais (Lei de Amdahl).

---

### Por que LRU é melhor que FIFO?

LRU explora localidade temporal: páginas recentemente usadas tendem a ser usadas novamente. FIFO ignora esse padrão, removendo páginas arbitrariamente.

---

### O que é preempção?

Interrupção forçada de um processo em execução para dar vez a outro. No Round Robin, ocorre quando quantum expira.

---

## 🔗 Recursos Úteis

### Onde encontrar exemplos de código?

- [Código de Referência](19-codigo-referencia.md)
- [Arquitetura Multicore](07-estrutura-multicore.md)
- [Round Robin](08-round-robin.md)

---

### Onde estudar mais sobre o tema?

- [Bibliografia](18-bibliografia.md)
- Livro: Tanenbaum - Modern Operating Systems
- Livro: Silberschatz - Operating System Concepts

---

### Ferramentas recomendadas?

- **IDE:** VS Code, CLion
- **Debugger:** GDB, LLDB
- **Profiler:** gprof, Valgrind
- **Sanitizers:** AddressSanitizer, ThreadSanitizer

---

## 📞 Contato e Suporte

### Onde tirar dúvidas?

1. Consulte esta documentação
2. [Troubleshooting](23-troubleshooting.md)
3. Professor da disciplina
4. Colegas de equipe

---

### Como reportar bugs na documentação?

Abra uma issue no repositório GitHub ou envie pull request com correções.

---

## 💡 Dicas Finais

### Para Implementação

✅ **DO:**
- Comece simples e incremente
- Teste frequentemente
- Use controle de versão (git)
- Documente decisões importantes
- Colete métricas desde o início

❌ **DON'T:**
- Não deixe tudo para última hora
- Não ignore warnings do compilador
- Não teste só no final
- Não copie código sem entender

---

### Para o Artigo

✅ **DO:**
- Comece a escrever cedo
- Gere gráficos conforme implementa
- Peça feedback de colegas
- Revise múltiplas vezes
- Verifique formatação IEEE

❌ **DON'T:**
- Não deixe escrita para última semana
- Não invente dados
- Não plagie
- Não use gráficos de baixa qualidade

---

## 🔗 Links Rápidos

- [Quickstart](QUICKSTART.md)
- [Roadmap](04-roadmap.md)
- [Testing Guide](TESTING_GUIDE.md)
- [Achievements](ACHIEVEMENTS.md)

---

**Última atualização:** Novembro 2025

**Contribuidores:** Equipe de Documentação SO 2025
