# Guia de Implementação: Simulador Multicore Round Robin

## 🎯 Bem-vindo!

Este é um guia completo e prático para implementar o **Trabalho Final de Sistemas Operacionais**: um simulador de arquitetura multicore com escalonamento Round Robin e gerenciamento de memória segmentada.

> 🏆 **NOVO:** [**Ver Progresso do Projeto (Achievements)**](ACHIEVEMENTS.md) - Acompanhe o que já foi feito e o que falta!

### 📌 O que você encontrará aqui:

- ✅ **Análise detalhada** do código base atual
- ✅ **Roadmap passo a passo** para implementação
- ✅ **Exemplos de código** comentados e testados
- ✅ **Estratégias de teste** e validação
- ✅ **Guia de escrita** do artigo IEEE
- ✅ **Métricas e análises** de desempenho

## 🎓 Sobre o Trabalho

**Disciplina:** Sistemas Operacionais - CEFET-MG Campus V  
**Professor:** Michel Pires da Silva  
**Data de Entrega:** 06/12/2025  
**Valor:** 30 pontos (20 Implementação + 10 Artigo)

### Objetivo

Desenvolver um simulador de arquitetura multicore que:

1. **Expanda** o simulador Von Neumann single-core já existente
2. **Implemente** escalonamento Round Robin preemptivo
3. **Gerencie** memória segmentada com políticas de substituição
4. **Colete** métricas detalhadas de desempenho
5. **Compare** resultados com a baseline single-core

## 🚀 Como Usar Este Guia

### Para Leitura Linear
Siga a ordem dos capítulos na sidebar à esquerda. Recomendado para quem está começando.

### Para Consulta Rápida
Use a busca (🔍) no topo para encontrar tópicos específicos.

### Para Implementação Prática
Vá direto para a seção **"⚙️ Implementação"** se você já entende os conceitos.

## 📊 Status do Projeto Base

O simulador atual possui:

| Componente | Status | Descrição |
|------------|--------|-----------|
| **CPU MIPS Pipeline** | ✅ Completo | Pipeline de 5 estágios (IF, ID, EX, MEM, WB) |
| **Banco de Registradores** | ✅ Completo | 32 registradores MIPS + especiais |
| **ULA** | ✅ Completo | Operações aritméticas e lógicas |
| **Memória Principal** | ✅ Completo | RAM com vector linear |
| **Memória Secundária** | ✅ Completo | Disco com matriz 2D |
| **Cache L1** | ✅ Completo | FIFO, write-back, no-write-allocate |
| **Gerenciador de Memória** | ✅ Completo | Unifica acesso RAM/Disco/Cache |
| **PCB** | ✅ Completo | Métricas, estado, quantum |
| **Escalonador** | ⚠️ Básico | Round-robin single-core simples |
| **I/O Manager** | ✅ Completo | Simulação de dispositivos I/O |

## 🎯 O Que Precisa Ser Implementado

<div class="alert alert-info">
<strong>Foco do Trabalho:</strong> Expandir o simulador para arquitetura multicore com escalonamento Round Robin adequado.
</div>

### Componentes Novos/Modificados:

- [ ] **Arquitetura Multicore** (n núcleos)
- [ ] **Escalonador Round Robin** multicore
- [ ] **Fila de Processos Global** ou por núcleo
- [ ] **Sincronização** entre núcleos
- [ ] **Gerenciamento de Memória** com segmentação
- [ ] **Políticas de Substituição** (FIFO, LRU)
- [ ] **Sistema de Métricas** expandido
- [ ] **Comparação** single-core vs multicore

## 📝 Estrutura da Documentação

```mermaid
graph LR
    A[Visão Geral] --> B[Planejamento]
    B --> C[Implementação]
    C --> D[Testes]
    D --> E[Artigo IEEE]
    E --> F[Entrega]
```

### 1️⃣ Visão Geral
Entenda o trabalho, requisitos e o código base atual.

### 2️⃣ Planejamento
Roadmap detalhado, divisão de tarefas e cronograma.

### 3️⃣ Implementação
Código passo a passo para cada componente novo.

### 4️⃣ Testes e Validação
Estratégias para garantir correção e desempenho.

### 5️⃣ Artigo IEEE
Como estruturar, escrever e apresentar resultados.

## 💡 Dicas Importantes

> **⚠️ Não reinvente a roda!** Use o código base existente como fundação.

> **📊 Métricas desde o início!** Instrumente o código conforme implementa.

> **🧪 Teste incrementalmente!** Não deixe testes para o final.

> **📝 Documente tudo!** Facilita a escrita do artigo depois.

## 🤝 Organização da Equipe

Este guia pressupõe uma equipe de **4 alunos**. Sugestão de divisão:

| Membro | Responsabilidade Principal |
|--------|---------------------------|
| **Dev 1** | Arquitetura Multicore + Sincronização |
| **Dev 2** | Escalonador Round Robin |
| **Dev 3** | Gerenciamento de Memória |
| **Dev 4** | Métricas + Artigo IEEE |

<div class="alert alert-success">
<strong>Trabalho colaborativo:</strong> Todos devem entender todos os componentes, mas cada um lidera uma área.
</div>

## 📖 Começando

Pronto para começar? Vá para a próxima seção:

➡️ [**Introdução ao Trabalho**](01-introducao.md)

---

## 🆘 Precisa de Ajuda?

- 📖 Consulte o [FAQ](20-faq.md)
- 🐛 Veja [Troubleshooting](23-troubleshooting.md)
- 📚 Confira as [Referências](18-bibliografia.md)

---

<div align="center">

**Boa sorte com o projeto! 🚀**

*Desenvolvido com ❤️ para a turma de SO 2025*

</div>
