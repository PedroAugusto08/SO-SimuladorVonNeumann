# 📚 DOCUMENTAÇÃO DO TRABALHO FINAL - Round Robin Multicore

> **🎯 COMECE AQUI!** Documentação completa para implementar o trabalho final de Sistemas Operacionais.

## 🚀 Acesso Rápido à Documentação

### ⭐ LEIA PRIMEIRO

📖 **[START_HERE.md](docs/START_HERE.md)** - Resumo executivo completo

🚀 **[QUICKSTART.md](docs/QUICKSTART.md)** - Guia rápido de início

📚 **[DOCS_README.md](docs/DOCS_README.md)** - Como usar a documentação

---

## 📋 Sobre o Trabalho

**Título:** Simulação de Arquitetura Multicore com Gerenciamento de Memória e Escalonamento Round Robin

**Objetivo:** Expandir o simulador Von Neumann single-core atual para arquitetura **multicore** com escalonamento **Round Robin**.

**Prazo:** 06/12/2025  
**Valor:** 30 pontos (10 escalonamento + 10 memória + 10 artigo)  
**Equipe:** 4 alunos  

---

## 📖 Visualizar Documentação Completa

### Opção 1: Localmente com Docsify (Recomendado)

```bash
# 1. Instalar docsify (uma vez)
npm install -g docsify-cli

# 2. Navegar até docs
cd docs

# 3. Iniciar servidor
docsify serve .

# 4. Abrir navegador
# http://localhost:3000
```

### Opção 2: Ler Arquivos Markdown

Navegue pela pasta [`docs/`](docs/) e leia os arquivos `.md` em ordem:

1. [`README.md`](docs/README.md) - Introdução
2. [`01-introducao.md`](docs/01-introducao.md) - Contexto completo
3. [`02-requisitos.md`](docs/02-requisitos.md) - O que implementar
4. [`03-arquitetura-atual.md`](docs/03-arquitetura-atual.md) - Análise do código
5. [`04-roadmap.md`](docs/04-roadmap.md) - Como implementar
6. [`08-round-robin.md`](docs/08-round-robin.md) - Round Robin detalhado

---

## 📚 Estrutura da Documentação

```
docs/
├── START_HERE.md              ⭐ COMECE AQUI
├── QUICKSTART.md              🚀 Guia rápido
├── DOCS_README.md             📖 Instruções
│
├── index.html                 🌐 Página Docsify
├── _coverpage.md              📄 Capa
├── _sidebar.md                📑 Menu
├── README.md                  🏠 Página inicial
│
├── 01-introducao.md           📋 Introdução ao trabalho
├── 02-requisitos.md           ✅ Requisitos detalhados
├── 03-arquitetura-atual.md    🔍 Análise do código base
├── 04-roadmap.md              🗺️ Plano de implementação
└── 08-round-robin.md          ⚙️ Escalonador detalhado
```

---

## 🎯 O Que a Documentação Contém

### ✅ Análise Completa

- 📊 Especificação do trabalho
- 🔍 Análise detalhada do código base
- 📐 Diagramas de arquitetura
- 🗂️ Componentes existentes
- 🎯 Pontos de modificação

### ✅ Roadmap de Implementação

- 📅 Plano de 3 semanas
- 🔢 7 etapas incrementais
- 💻 Código para cada etapa
- 📊 Marcos de validação
- ✅ Checklists completos

### ✅ Implementação Detalhada

- 🔧 Classe `Core` completa
- ⚙️ `RoundRobinScheduler` completo
- 🔒 Sincronização com mutexes
- 📈 Coleta de métricas
- 🧪 Casos de teste

### ✅ Guias Práticos

- 📝 Como dividir tarefas na equipe
- 🧪 Como testar cada componente
- 📊 Como coletar métricas
- 📄 Como escrever o artigo IEEE
- 🐛 Troubleshooting

---

## 🎓 Conceitos Fundamentais Cobertos

### Escalonamento Round Robin

```
┌─────────────────────────────────┐
│  Fila de Prontos (FIFO)         │
│  [ P1 ] [ P2 ] [ P3 ] [ P4 ]    │
└──────┬──────┬──────┬─────────────┘
       │      │      │
   ┌───▼──┐┌─▼───┐┌─▼───┐
   │Core0 ││Core1││Core2│
   └──────┘└─────┘└─────┘
```

### Hierarquia de Memória

```
Cache L1 (privada) → RAM (compartilhada) → Disco (compartilhado)
```

### Métricas Coletadas

- Tempo de espera
- Tempo de retorno
- Utilização da CPU
- Throughput
- Speedup multicore vs single-core
- Context switches

---

## 💻 Código de Exemplo Incluído

A documentação inclui implementações completas de:

### `Core.hpp/cpp`
Classe que representa um núcleo de processamento com:
- Pipeline MIPS completo
- Cache L1 privada
- Execução assíncrona (thread)
- Métricas por núcleo

### `RoundRobinScheduler.hpp/cpp`
Escalonador Round Robin multicore com:
- Fila global de processos
- Distribuição entre núcleos
- Context switch
- Coleta de métricas

### Modificações em `main.cpp`
Loop principal adaptado para:
- Gerenciar múltiplos núcleos
- Escalonamento Round Robin
- Sincronização thread-safe

### Extensões de `PCB.hpp`
Métricas adicionadas:
- Tempos de chegada/início/fim
- Context switches
- Núcleo atribuído
- Migrações entre núcleos

---

## 🔧 Ferramentas Necessárias

### Obrigatórias
- ✅ C++17 ou superior
- ✅ CMake 3.10+
- ✅ Make
- ✅ Git
- ✅ pthread

### Recomendadas
- ✅ Node.js (para Docsify)
- ✅ VS Code
- ✅ Docker/WSL
- ✅ Gnuplot (gráficos)

---

## 📊 Roadmap Resumido

### Semana 1 (13-20 Nov): Estrutura Multicore
- [ ] Criar classe `Core`
- [ ] Modificar `main()` para multicore
- [ ] Testar com 2+ núcleos

### Semana 2 (20-27 Nov): Escalonador + Sincronização
- [ ] Implementar `RoundRobinScheduler`
- [ ] Adicionar mutexes
- [ ] Validar distribuição entre núcleos

### Semana 3 (27 Nov - 04 Dez): Memória + Métricas
- [ ] Segmentação de memória
- [ ] Políticas de substituição (FIFO/LRU)
- [ ] Coletar todas métricas
- [ ] Comparação baseline

### Semana 4 (04-06 Dez): Artigo
- [ ] Escrever artigo IEEE
- [ ] Gerar gráficos
- [ ] Revisão final

---

## 👥 Sugestão de Divisão de Tarefas

| Membro | Responsabilidade | Entregas |
|--------|------------------|----------|
| **Dev 1** | Arquitetura Multicore | `Core.hpp/cpp`, integração |
| **Dev 2** | Escalonador Round Robin | `RoundRobinScheduler.hpp/cpp` |
| **Dev 3** | Gerência de Memória | `SegmentTable`, sincronização |
| **Dev 4** | Métricas + Artigo | Coleta, análise, artigo IEEE |

---

## 🎯 Critérios de Avaliação

### Implementação (20 pontos)

**Escalonamento (10 pts):**
- Round Robin implementado corretamente
- Context switch funcional
- Quantum configurável
- Distribuição multicore

**Memória (10 pts):**
- Segmentação implementada
- Tradução de endereços
- Política de substituição
- Thread-safe

### Artigo IEEE (10 pontos)
- Formato correto
- Fundamentação teórica
- Metodologia clara
- Resultados e análises
- Conclusões

---

## 📚 Referências

### Documentação do Projeto
- [START_HERE.md](docs/START_HERE.md) - Resumo executivo
- [QUICKSTART.md](docs/QUICKSTART.md) - Guia rápido
- [Documentação completa](docs/) - Todos capítulos

### Templates
- [Artigo IEEE](https://pt.overleaf.com/latex/templates/ieee-conference-template/grfzhhncsfqn)
- [Docsify](https://docsify.js.org/)

### Livros
1. **Tanenbaum, A. S.** - Modern Operating Systems
2. **Patterson & Hennessy** - Computer Organization and Design
3. **Silberschatz et al.** - Operating System Concepts

---

## 🆘 Suporte

### Dúvidas sobre a Documentação?
- Leia [`docs/DOCS_README.md`](docs/DOCS_README.md)
- Consulte o FAQ (quando disponível)

### Problemas Técnicos?
- Veja Troubleshooting (quando disponível)
- Abra issue no GitHub
- Consulte o professor

### Contribuir com a Documentação?
- Fork o repositório
- Edite arquivos em `docs/`
- Faça pull request

---

## ✅ Checklist Antes de Começar

- [ ] Li `START_HERE.md`
- [ ] Li `QUICKSTART.md`
- [ ] Configurei Docsify (opcional)
- [ ] Li introdução completa
- [ ] Entendi os requisitos
- [ ] Analisei o código base
- [ ] Revisei o roadmap
- [ ] Formei equipe de 4 pessoas
- [ ] Dividi tarefas
- [ ] Configurei ambiente de desenvolvimento

---

## 🚀 Começar Agora

```bash
# 1. Ver documentação
cd docs
docsify serve .
# Abra http://localhost:3000

# 2. Ler resumo executivo
cat docs/START_HERE.md

# 3. Ler guia rápido
cat docs/QUICKSTART.md

# 4. Compilar projeto base
cd build
cmake ..
make
./simulador
```

---

<div align="center">

## 🎓 Trabalho Final - Sistemas Operacionais

**CEFET-MG Campus V - 2025**

**Professor:** Michel Pires da Silva

**Prazo de Entrega:** 06/12/2025

---

**📚 [ACESSE A DOCUMENTAÇÃO COMPLETA](docs/START_HERE.md)**

---

*Desenvolvido com ❤️ para ajudar na implementação do trabalho*

**Boa sorte! 🚀**

</div>
