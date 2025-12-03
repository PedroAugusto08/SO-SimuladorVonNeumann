# 🚀 Guia Rápido de Início

## 📋 Sumário Executivo

Você acabou de receber uma **documentação completa** para implementar o trabalho final de Sistemas Operacionais: um **Simulador Multicore com Escalonamento Round Robin**.

## ✅ O Que Foi Criado

### 📚 Documentação Docsify

Uma documentação interativa e navegável com:

1. **Visão Geral do Trabalho**
   - Especificação completa
   - Requisitos detalhados
   - Critérios de avaliação

2. **Análise do Código Base**
   - Estrutura atual do simulador
   - Componentes existentes
   - Pontos de modificação

3. **Roadmap de Implementação**
   - 7 etapas bem definidas
   - 3 semanas de desenvolvimento
   - Marcos de validação

4. **Guia Round Robin Detalhado**
   - Teoria completa
   - Código implementado
   - Exemplos de teste
   - Métricas a coletar

## 🎯 Como Usar Este Guia

### Passo 1: Visualizar a Documentação

**Opção A: Localmente com Docsify (Recomendado)**

```bash
# 1. Instalar docsify (uma vez)
npm install -g docsify-cli

# 2. Navegar até a pasta docs
cd c:\Users\Henrique\Documents\github\SO-SimuladorVonNeumann\docs

# 3. Iniciar servidor
docsify serve .

# 4. Abrir navegador
# http://localhost:3000
```

**Opção B: Ler os Arquivos Markdown**

Abra os arquivos `.md` na ordem:
1. `README.md` - Introdução
2. `01-introducao.md` - Contexto completo
3. `02-requisitos.md` - O que implementar
4. `03-arquitetura-atual.md` - Análise do código
5. `04-roadmap.md` - Como implementar
6. `08-round-robin.md` - Round Robin detalhado

### Passo 2: Entender o Trabalho

Leia atentamente:

- ✅ **Requisitos:** O que é obrigatório implementar
- ✅ **Prazo:** 06/12/2025
- ✅ **Pontuação:** 30 pontos (10 escalonamento + 10 memória + 10 artigo)
- ✅ **Equipe:** 4 alunos

### Passo 3: Planejar com a Equipe

Use o roadmap fornecido:

```
Semana 1 (13-20 Nov): Estrutura Multicore
  ├─ Criar classe Core
  ├─ Modificar main() para multicore
  └─ Testes básicos

Semana 2 (20-27 Nov): Escalonador + Sincronização
  ├─ Implementar RoundRobinScheduler
  ├─ Adicionar mutexes
  └─ Testar distribuição

Semana 3 (27 Nov - 04 Dez): Memória + Métricas
  ├─ Segmentação de memória
  ├─ Políticas de substituição
  ├─ Coletar métricas
  └─ Comparação baseline

Semana 4 (04-06 Dez): Artigo IEEE
  ├─ Escrever artigo
  ├─ Gerar gráficos
  └─ Revisão final
```

### Passo 4: Dividir Tarefas

Sugestão de divisão para 4 pessoas:

| Membro | Responsabilidade | Entregas |
|--------|------------------|----------|
| **Dev 1** | Arquitetura Multicore | `Core.hpp/cpp`, integração |
| **Dev 2** | Escalonador Round Robin | `RoundRobinScheduler.hpp/cpp` |
| **Dev 3** | Gerência de Memória | `SegmentTable`, políticas |
| **Dev 4** | Métricas + Artigo | Coleta, análise, artigo IEEE |

**Todos devem:**
- Entender todos os componentes
- Revisar código uns dos outros
- Testar integrações

### Passo 5: Implementar Incrementalmente

**Não tente fazer tudo de uma vez!**

Siga as etapas do roadmap:

1. **Etapa 1:** Core básico (3 dias)
   - Crie `Core.hpp` e `Core.cpp`
   - Teste com 2 núcleos
   - Valide compilação

2. **Etapa 2:** Escalonador (4 dias)
   - Crie `RoundRobinScheduler`
   - Integre com cores
   - Teste Round Robin

3. **Etapa 3:** Sincronização (3 dias)
   - Adicione mutexes
   - Torne thread-safe
   - Valide sem race conditions

4. **Continue...**

### Passo 6: Testar Continuamente

**Não deixe testes para o final!**

Após cada etapa:
```bash
cd build
cmake ..
make
./simulador
```

Valide:
- ✅ Compila sem erros
- ✅ Executa sem crashes
- ✅ Resultados corretos
- ✅ Métricas fazem sentido

### Passo 7: Documentar Tudo

**Facilita a escrita do artigo depois!**

Anote:
- Decisões de design
- Problemas encontrados
- Soluções aplicadas
- Resultados de testes

## 📊 Estrutura do Código a Implementar

```
src/
├── main.cpp                        # ← Modificar (multicore)
├── cpu/
│   ├── Core.hpp                    # ← NOVO
│   ├── Core.cpp                    # ← NOVO
│   ├── RoundRobinScheduler.hpp     # ← NOVO
│   ├── RoundRobinScheduler.cpp     # ← NOVO
│   ├── CONTROL_UNIT.cpp            # ← Modificar (sync)
│   ├── PCB.hpp                     # ← Modificar (métricas)
│   └── ...
├── memory/
│   ├── MemoryManager.cpp           # ← Modificar (mutex)
│   ├── SegmentTable.hpp            # ← NOVO
│   ├── SegmentTable.cpp            # ← NOVO
│   └── ...
└── ...
```

## 🎯 Marcos de Validação

### Marco 1 (20 Nov) - Multicore Básico
- [ ] Código compila
- [ ] 2+ núcleos executam processos
- [ ] Round Robin básico funciona
- [ ] Sem crashes

### Marco 2 (27 Nov) - Sincronização
- [ ] Memória thread-safe
- [ ] Cache privada por núcleo
- [ ] Sem race conditions
- [ ] Métricas básicas coletadas

### Marco 3 (04 Dez) - Completo
- [ ] Segmentação implementada
- [ ] Políticas de substituição
- [ ] Comparação baseline
- [ ] Métricas completas

### Marco 4 (06 Dez) - Entrega
- [ ] Artigo IEEE finalizado
- [ ] Código no GitHub
- [ ] README com instruções
- [ ] Testes validados

## 📖 Documentação Disponível

### Criados (✅)

| Arquivo | Conteúdo | Status |
|---------|----------|--------|
| `index.html` | Página Docsify | ✅ |
| `README.md` | Página inicial | ✅ |
| `01-introducao.md` | Introdução completa | ✅ |
| `02-requisitos.md` | Todos requisitos | ✅ |
| `03-arquitetura-atual.md` | Análise detalhada | ✅ |
| `04-roadmap.md` | Plano completo | ✅ |
| `08-round-robin.md` | Round Robin detalhado | ✅ |

### A Criar (⏳)

Você pode expandir a documentação criando:

- `05-divisao-tarefas.md` - Divisão entre membros
- `06-cronograma.md` - Cronograma detalhado
- `07-estrutura-multicore.md` - Implementação Core
- `09-memoria.md` - Segmentação
- `10-sincronizacao.md` - Mutexes e locks
- `11-metricas.md` - Coleta de métricas
- `12-testes.md` - Estratégia de testes
- `15-estrutura-artigo.md` - Template artigo
- `18-bibliografia.md` - Referências

## 💡 Dicas Importantes

### ✅ Faça

- ✅ **Comece cedo** - 3 semanas passa rápido
- ✅ **Teste incrementalmente** - A cada mudança
- ✅ **Use Git** - Commits frequentes
- ✅ **Documente** - Anote decisões
- ✅ **Peça ajuda** - Professor e colegas
- ✅ **Revise código** - Entre membros da equipe

### ❌ Não Faça

- ❌ Deixar para última semana
- ❌ Tentar fazer tudo de uma vez
- ❌ Ignorar testes
- ❌ Modificar tudo ao mesmo tempo
- ❌ Trabalhar sem comunicação

## 🔧 Comandos Úteis

### Compilar
```bash
cd build
cmake ..
make
```

### Executar
```bash
./simulador
```

### Limpar
```bash
make clean
```

### Ver Documentação
```bash
cd docs
docsify serve .
```

### Git
```bash
# Status
git status

# Commit
git add .
git commit -m "feat: implementa Core básico"

# Push
git push origin main

# Pull
git pull origin main
```

## 📚 Referências Rápidas

### Código Base Importante

**main.cpp (linha ~120):**
```cpp
// Loop principal - PRECISA SER MODIFICADO
while (finished_processes < total_processes) {
    PCB* current_process = ready_queue.front();
    Core(memManager, *current_process, ...);  // ← Tornar multicore
}
```

**PCB.hpp:**
```cpp
struct PCB {
    int pid;
    State state;
    int quantum;  // ← Quantum configurável
    hw::REGISTER_BANK regBank;
    std::atomic<uint64_t> pipeline_cycles{0};
    // ... adicionar métricas de escalonamento
};
```

**MemoryManager.cpp:**
```cpp
uint32_t MemoryManager::read(uint32_t address, PCB& process) {
    // ← Adicionar mutex aqui
    // std::lock_guard<std::mutex> lock(memory_mutex);
    // ...
}
```

## 🆘 Suporte

### Problemas?

1. **Consulte a documentação:**
   - `DOCS_README.md` - Instruções completas
   - Capítulos específicos para cada tópico

2. **Revise o código base:**
   - `README.md` principal do projeto
   - Código existente comentado

3. **Use as ferramentas:**
   - `make check` - Verificação rápida
   - `make test-all` - Testes completos

### Contatos

- **Professor:** Michel Pires da Silva
- **Repositório:** https://github.com/PedroAugusto08/SO-SimuladorVonNeumann
- **Issues:** Use GitHub Issues para dúvidas

## 🎓 Conclusão

Você tem em mãos:

1. ✅ **Documentação completa** do trabalho
2. ✅ **Análise detalhada** do código base
3. ✅ **Roadmap passo a passo** de implementação
4. ✅ **Código exemplo** do Round Robin
5. ✅ **Guias de teste** e validação

**Próximos passos:**

1. 📖 Leia toda a documentação
2. 👥 Reúna sua equipe
3. 📅 Planeje as 3 semanas
4. 💻 Comece a implementar
5. 🧪 Teste continuamente
6. 📝 Escreva o artigo
7. 🎉 Entregue no prazo!

---

<div align="center">

**Sucesso no projeto! 🚀**

*Data de entrega: 06/12/2025*

[⬆ Voltar ao início](#-guia-rápido-de-início)

</div>
