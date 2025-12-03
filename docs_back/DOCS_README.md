# 📚 Documentação do Projeto - Simulador Multicore Round Robin

> Guia completo para implementar o trabalho final de Sistemas Operacionais

## 🚀 Como Usar Esta Documentação

### Opção 1: Visualizar Online com Docsify (Recomendado)

1. **Instale o Docsify CLI:**
```bash
npm install -g docsify-cli
```

2. **Navegue até a pasta docs:**
```bash
cd docs
```

3. **Inicie o servidor:**
```bash
docsify serve .
```

4. **Abra no navegador:**
```
http://localhost:3000
```

### Opção 2: Ler os Arquivos Markdown

Você pode ler diretamente os arquivos `.md` em ordem:

1. [README.md](README.md) - Início
2. [01-introducao.md](01-introducao.md) - Introdução
3. [02-requisitos.md](02-requisitos.md) - Requisitos
4. [03-arquitetura-atual.md](03-arquitetura-atual.md) - Arquitetura Atual
5. [04-roadmap.md](04-roadmap.md) - Roadmap
6. [08-round-robin.md](08-round-robin.md) - Round Robin Detalhado

## 📂 Estrutura da Documentação

```
docs/
├── index.html              # Página HTML do Docsify
├── _coverpage.md           # Capa da documentação
├── _sidebar.md             # Menu lateral
├── README.md               # Página inicial
│
├── 01-introducao.md        # Introdução ao trabalho
├── 02-requisitos.md        # Requisitos detalhados
├── 03-arquitetura-atual.md # Análise do código base
├── 04-roadmap.md           # Plano de implementação
├── 05-divisao-tarefas.md   # (A criar) Divisão da equipe
├── 06-cronograma.md        # (A criar) Cronograma detalhado
│
├── 07-estrutura-multicore.md  # (A criar) Implementação multicore
├── 08-round-robin.md          # Escalonador Round Robin
├── 09-memoria.md              # (A criar) Gerência de memória
├── 10-sincronizacao.md        # (A criar) Sincronização
├── 11-metricas.md             # (A criar) Métricas
│
├── 12-testes.md            # (A criar) Estratégia de testes
├── 13-casos-teste.md       # (A criar) Casos de teste
├── 14-debugging.md         # (A criar) Debugging
│
├── 15-estrutura-artigo.md  # (A criar) Artigo IEEE
├── 16-resultados.md        # (A criar) Resultados
├── 17-escrita.md           # (A criar) Escrita científica
│
├── 18-bibliografia.md      # (A criar) Bibliografia
├── 19-codigo-referencia.md # (A criar) Código de referência
├── 20-faq.md               # (A criar) FAQ
│
├── 21-ambiente.md          # (A criar) Instalação
├── 22-comandos.md          # (A criar) Comandos úteis
└── 23-troubleshooting.md   # (A criar) Troubleshooting
```

## ✅ Status dos Capítulos

| Capítulo | Status | Descrição |
|----------|--------|-----------|
| index.html | ✅ Completo | Página Docsify |
| _coverpage.md | ✅ Completo | Capa |
| _sidebar.md | ✅ Completo | Menu lateral |
| README.md | ✅ Completo | Página inicial |
| 01-introducao.md | ✅ Completo | Introdução detalhada |
| 02-requisitos.md | ✅ Completo | Todos requisitos |
| 03-arquitetura-atual.md | ✅ Completo | Análise completa |
| 04-roadmap.md | ✅ Completo | Plano completo |
| 08-round-robin.md | ✅ Completo | Implementação RR |
| Demais capítulos | ⏳ Pendente | A ser criado |

## 🎯 Objetivos da Documentação

### 1. Guiar a Implementação
- ✅ Roadmap passo a passo
- ✅ Código comentado e testado
- ✅ Exemplos práticos

### 2. Facilitar o Aprendizado
- ✅ Fundamentos teóricos
- ✅ Diagramas explicativos
- ✅ Referências bibliográficas

### 3. Acelerar o Desenvolvimento
- ✅ Templates de código
- ✅ Checklist de validação
- ✅ Troubleshooting

### 4. Auxiliar na Escrita do Artigo
- ✅ Estrutura IEEE
- ✅ Métricas a coletar
- ✅ Gráficos sugeridos

## 💡 Como Contribuir

Se você está trabalhando em equipe:

1. **Clone o repositório:**
```bash
git clone https://github.com/PedroAugusto08/SO-SimuladorVonNeumann.git
cd SO-SimuladorVonNeumann/docs
```

2. **Crie um branch para sua seção:**
```bash
git checkout -b docs/memoria
```

3. **Edite os arquivos markdown:**
```bash
# Use seu editor preferido
code 09-memoria.md
```

4. **Visualize localmente:**
```bash
docsify serve .
```

5. **Commit e push:**
```bash
git add .
git commit -m "docs: adiciona capítulo de memória"
git push origin docs/memoria
```

6. **Crie Pull Request**

## 🎨 Convenções de Escrita

### Formatação

- Use **negrito** para termos importantes
- Use `código` para nomes de arquivos, funções, variáveis
- Use > para citações e alertas importantes

### Blocos de Código

````markdown
```cpp
// Código C++
class Example {
    // ...
};
```

```bash
# Comandos bash
make build
```
````

### Alertas

Use divs com classes:

```html
<div class="alert alert-info">
<strong>Dica:</strong> Isto é uma informação útil.
</div>

<div class="alert alert-warning">
<strong>Atenção:</strong> Cuidado com este ponto.
</div>

<div class="alert alert-danger">
<strong>Erro:</strong> Isto pode dar errado!
</div>

<div class="alert alert-success">
<strong>Sucesso:</strong> Isto funcionou!
</div>
```

### Diagramas Mermaid

```markdown
```mermaid
graph LR
    A[Início] --> B[Processo]
    B --> C[Fim]
```
```

## 📋 Checklist de Implementação

Use esta checklist para acompanhar o progresso:

### Semana 1: Estrutura Multicore
- [ ] Criar classe `Core`
- [ ] Modificar `main()` para multicore
- [ ] Atualizar `CMakeLists.txt`
- [ ] Testar com 2+ núcleos

### Semana 2: Round Robin
- [ ] Criar `RoundRobinScheduler`
- [ ] Implementar fila circular
- [ ] Adicionar context switch
- [ ] Testar distribuição entre núcleos

### Semana 3: Sincronização
- [ ] Adicionar mutexes ao `MemoryManager`
- [ ] Tornar cache privada
- [ ] Validar thread-safety
- [ ] Testar sem race conditions

### Semana 4: Memória
- [ ] Implementar `SegmentTable`
- [ ] Tradução de endereços
- [ ] Política de substituição (FIFO ou LRU)
- [ ] Testes de segmentação

### Semana 5: Métricas e Artigo
- [ ] Coletar todas métricas
- [ ] Comparação baseline
- [ ] Gerar gráficos
- [ ] Escrever artigo IEEE

## 🆘 Suporte

### Problemas com Docsify?

1. **Não instalou Node.js?**
```bash
# Ubuntu/Debian
sudo apt install nodejs npm

# Windows (Chocolatey)
choco install nodejs

# macOS (Homebrew)
brew install node
```

2. **Porta 3000 ocupada?**
```bash
docsify serve . -p 4000  # Usa porta 4000
```

3. **Não carrega os arquivos?**
- Verifique se está na pasta `docs/`
- Verifique se `index.html` existe

### Problemas com Markdown?

- Use um editor com preview: VS Code, Typora, etc.
- Teste a sintaxe em: https://dillinger.io/

### Dúvidas Técnicas?

- Consulte o [FAQ](20-faq.md) (quando disponível)
- Veja o [Troubleshooting](23-troubleshooting.md) (quando disponível)
- Abra uma issue no GitHub

## 📖 Recursos Adicionais

### Templates

- **Template IEEE:** https://pt.overleaf.com/latex/templates/ieee-conference-template/grfzhhncsfqn
- **Docsify Docs:** https://docsify.js.org/
- **Mermaid Docs:** https://mermaid.js.org/

### Referências Bibliográficas

1. **Tanenbaum, A. S.** - Modern Operating Systems
2. **Patterson & Hennessy** - Computer Organization and Design
3. **Silberschatz et al.** - Operating System Concepts

## 📝 Notas de Versão

### v1.0 (Atual)
- ✅ Estrutura básica da documentação
- ✅ Capítulos 01-04 completos
- ✅ Capítulo 08 (Round Robin) completo
- ✅ Configuração Docsify

### Próximas Versões
- ⏳ Capítulos 05-07 (Planejamento)
- ⏳ Capítulos 09-11 (Implementação avançada)
- ⏳ Capítulos 12-14 (Testes)
- ⏳ Capítulos 15-17 (Artigo)
- ⏳ Capítulos 18-23 (Referências e suporte)

## 🎓 Créditos

**Desenvolvido para:**
- Disciplina: Sistemas Operacionais
- Professor: Michel Pires da Silva
- Instituição: CEFET-MG Campus V
- Ano: 2025

**Equipe:**
- [Seu nome aqui]
- [Membro 2]
- [Membro 3]
- [Membro 4]

---

<div align="center">

**Boa sorte com o projeto! 🚀**

*Desenvolvido com ❤️ usando Docsify*

[⬆ Voltar ao topo](#-documentação-do-projeto---simulador-multicore-round-robin)

</div>
