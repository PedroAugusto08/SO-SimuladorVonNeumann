# Quickstart WSL — Compilação e Execução do Simulador Round Robin

Este guia contém os comandos práticos para compilar e executar o simulador multicore Round Robin no WSL.

## ✅ Status da Implementação

**COMPILAÇÃO FUNCIONANDO!** O simulador está compilando e executando corretamente no WSL.

### Correções Aplicadas

1. **Conflito de nomes resolvido**: A função `Core()` em `CONTROL_UNIT.hpp` foi renomeada para `CoreExecutionLoop()` para não conflitar com a classe `Core`.
2. **RoundRobinScheduler implementado**: Arquivos `src/cpu/RoundRobinScheduler.hpp` e `.cpp` criados e integrados ao Makefile.
3. **Makefile atualizado**: `RoundRobinScheduler.cpp` adicionado à lista de fontes (`SRC_SIM`).

## 1) Instalar ferramentas de build (apenas primeira vez)

```bash
sudo apt update
sudo apt install -y build-essential make g++
```

## 2) Compilar o simulador

Na raiz do projeto:

```bash
cd /mnt/c/Users/Henrique/Documents/github/SO-SimuladorVonNeumann

# Limpar build anterior
make clean

# Compilar o simulador multicore
make simulador
```

**Output esperado:**
```
✓ Simulador multicore compilado com sucesso!
```

## 3) Executar o simulador

```bash
# Certifique-se de que tasks.json está na raiz
cp src/tasks/tasks.json . 2>/dev/null || true

# Executar
./simulador
```

ou use o atalho do Makefile:

```bash
make run-sim
```

## 4) Servir a documentação Docsify localmente

### Opção A: Servidor HTTP simples (Python)

```bash
cd /mnt/c/Users/Henrique/Documents/github/SO-SimuladorVonNeumann
python3 -m http.server 8080 --directory docs
```

Abra no navegador: http://localhost:8080

### Opção B: Docsify CLI (com live-reload)

Instalar Node.js e docsify-cli (apenas primeira vez):

```bash
# Instalar Node.js
sudo apt install -y nodejs npm

# Instalar docsify-cli globalmente
sudo npm install -g docsify-cli
```

Servir a documentação:

```bash
cd /mnt/c/Users/Henrique/Documents/github/SO-SimuladorVonNeumann/docs
docsify serve . --port 8080
```

Abra no navegador: http://localhost:8080

## 5) Arquivos importantes criados/modificados

### Novos arquivos

- `src/cpu/RoundRobinScheduler.hpp` - Interface do escalonador Round Robin
- `src/cpu/RoundRobinScheduler.cpp` - Implementação do escalonador
- `docs/WSL_QUICKSTART.md` - Este guia

### Arquivos modificados

- `Makefile` - Adicionado `RoundRobinScheduler.cpp` ao `SRC_SIM`
- `src/cpu/CONTROL_UNIT.hpp` - Renomeado `Core()` → `CoreExecutionLoop()`
- `src/cpu/CONTROL_UNIT.cpp` - Atualizada implementação da função renomeada
- `src/test/test_cpu_metrics.cpp` - Atualizada chamada da função renomeada
- `src/main.cpp` - Ordem de includes ajustada

## 6) Estrutura do código

```
src/
├── cpu/
│   ├── Core.hpp/cpp              ✅ Núcleo de processamento (thread assíncrona)
│   ├── RoundRobinScheduler.hpp/cpp ✅ Escalonador Round Robin (NOVO)
│   ├── PCB.hpp                    ✅ Process Control Block (com métricas RR)
│   ├── CONTROL_UNIT.hpp/cpp       ✅ Pipeline MIPS
│   └── ...
├── memory/
│   ├── MemoryManager.hpp/cpp      ✅ Gerenciador de memória
│   ├── cache.hpp/cpp              ✅ Cache L1
│   └── ...
├── IO/
│   └── IOManager.hpp/cpp          ✅ Gerenciador de I/O
└── main.cpp                       ✅ Loop principal multicore
```

## 7) Troubleshooting

### Erro: "make: command not found"

```bash
sudo apt install make
```

### Erro: "g++: command not found"

```bash
sudo apt install g++
```

### Erro: "Não foi possível abrir: tasks.json"

```bash
cp src/tasks/tasks.json .
```

### WSL não expõe localhost

Se `http://localhost:8080` não funcionar, descubra o IP do WSL:

```bash
hostname -I
```

Use `http://<IP>:8080` no navegador do Windows.

## 8) Próximos passos

- Criar exemplos de entrada JSON com múltiplos processos
- Documentar formato de saída de métricas
- Adicionar páginas extras ao Docsify (exemplos, logs, métricas)
- Integrar o `RoundRobinScheduler` completamente ao `main.cpp`

---

**Última atualização:** Compilação validada em 13/11/2025
