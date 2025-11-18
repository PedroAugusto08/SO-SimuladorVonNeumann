# 📋 Comandos do Makefile - SO-SimuladorVonNeumann# 📋 Comandos do Makefile - SO-SimuladorVonNeumann



Referência completa dos comandos disponíveis no Makefile do projeto.## 🎯 **Comandos Disponíveis**



---### **Comandos Básicos**

- `make` ou `make all` - Compila e executa o programa principal

## 🎯 Comandos Principais- `make clean` - Remove arquivos gerados (.o, executáveis)

- `make run` - Executa programa principal (sem recompilar)

### Simulador Multicore (PRINCIPAL)

### **Comandos de Teste**

```bash- `make test-hash` - Compila e testa sistema de registradores MIPS

make simulador     # 🎯 Compila o simulador multicore Round-Robin  - `make test-bank` - Compila e testa sistema de banco registradores

make run-sim       # 🚀 Executa o simulador multicore- `make test-all` - Executa todos os testes disponíveis

```- `make check` - Verificação rápida (✅ PASSOU/❌ FALHOU)



**O que faz:**### **Comandos de Build**

- Compila todos os módulos: Core, RoundRobinScheduler, CONTROL_UNIT, MemoryManager, IOManager- `make teste` - Compila apenas o programa principal

- Gera o executável `./simulador`- `make test_hash_register` - Compila apenas o teste do hash register

- Usa GCC com flags: `-Wall -Wextra -g -std=c++17 -Isrc -lpthread`- `make debug` - Build com símbolos de debug (-DDEBUG -O0 -ggdb3)



### Programas de Teste### **Comandos de Informação**

- `make help` - Mostra todos os comandos com descrições

```bash- `make list-files` - Lista arquivos do projeto (fontes, headers)

make               # Compila e executa programa principal (teste ULA)

make teste         # Apenas compila o programa principal## � **Exemplos de Uso**

make run           # Executa programa principal (sem recompilar)

```### **Desenvolvimento Diário**

```bash

### Limpezamake help          # Ver comandos disponíveis

make               # Compilar e testar ULA

```bashmake test-hash     # Testar registradores MIPS

make clean         # 🧹 Remove todos os .o e executáveismake check         # Verificação rápida

``````



---### **Debug de Problemas**

```bash

## 🧪 Comandos de Testemake debug         # Build com símbolos

gdb ./teste        # Debugger

### Testes de Componentes```



```bash### **Informações do Projeto**

make test-hash     # 🧪 Testa sistema de registradores hash```bash

make test-bank     # 🧪 Testa banco de registradores completomake list-files    # Ver estrutura

make test-all      # 🧪 Executa todos os testes sequencialmentemake help          # Ver todos os comandos

``````



### Verificação Rápida## 📊 **Tabela de Comandos**



```bash| Comando | Função | Uso |

make check         # ✅ Verifica todos os componentes|---------|--------|-----|

                   # Output: ✅ PASSOU ou ❌ FALHOU| `make help` | Lista comandos | Primeiro uso |

```| `make` | Compila e executa | Desenvolvimento |

| `make test-hash` | Testa registradores | Validar MIPS |

---| `make test-bank` | Testa banco registradores | Validar MIPS |

| `make check` | Verificação rápida | Testes automáticos |

## 🐛 Desenvolvimento e Debug| `make debug` | Build debug | Debugging |

| `make clean` | Limpa arquivos | Rebuild |

### Build com Debug

---

```bash**Total: 10 comandos implementados e funcionando** ✅

make debug         # 🐛 Compila com símbolos de debug
                   # Flags adicionais: -DDEBUG -O0 -ggdb3
```

**Uso com GDB:**
```bash
make debug
gdb ./teste
# ou
gdb ./simulador
```

### Informações do Projeto

```bash
make help          # ℹ️ Mostra todos os comandos disponíveis
make list-files    # 📁 Lista arquivos fonte e headers
```

---

## 📊 Tabela de Referência Rápida

| Comando | Descrição | Target Gerado |
|---------|-----------|---------------|
| `make simulador` | 🎯 Compila simulador multicore | `./simulador` |
| `make run-sim` | 🚀 Executa simulador | - |
| `make` ou `make all` | Compila e executa teste principal | `./teste` |
| `make clean` | 🧹 Remove arquivos gerados | - |
| `make test-hash` | 🧪 Testa hash register | `./test_hash_register` |
| `make test-bank` | 🧪 Testa register bank | `./test_register_bank` |
| `make test-all` | 🧪 Executa todos os testes | - |
| `make check` | ✅ Verificação rápida | - |
| `make debug` | 🐛 Build com símbolos | `./teste` (debug) |
| `make help` | ℹ️ Ajuda completa | - |
| `make list-files` | 📁 Lista arquivos | - |

---

## 📦 Alvos (Targets) Disponíveis

### Executáveis

- **`simulador`** - Simulador multicore Round-Robin (PRINCIPAL)
- **`teste`** - Programa de teste da ULA
- **`test_hash_register`** - Teste do sistema de registradores hash
- **`test_register_bank`** - Teste do banco de registradores

### Variáveis Importantes

```makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++17 -Isrc
LDFLAGS = -lpthread
```

---

## 🔧 Fluxo de Trabalho Recomendado

### Desenvolvimento Normal

```bash
# 1. Limpar build anterior
make clean

# 2. Compilar o simulador
make simulador

# 3. Executar
make run-sim
```

### Após Mudanças no Código

```bash
# Recompilar automaticamente apenas arquivos alterados
make simulador
```

### Debug de Problemas

```bash
# 1. Build com debug
make debug

# 2. Executar com GDB
gdb ./teste

# Ou para o simulador:
make clean
make CXXFLAGS="-Wall -Wextra -g -std=c++17 -Isrc -DDEBUG -O0 -ggdb3" simulador
gdb ./simulador
```

### Validação Completa

```bash
# Testar todos os componentes
make clean
make check
make simulador
make run-sim
```

---

## 🛠️ Estrutura de Compilação

### Arquivos Fonte do Simulador

O target `simulador` compila os seguintes arquivos (definidos em `SRC_SIM`):

```makefile
SRC_SIM := src/main.cpp \
           src/cpu/Core.cpp \
           src/cpu/RoundRobinScheduler.cpp \
           src/cpu/CONTROL_UNIT.cpp \
           src/cpu/pcb_loader.cpp \
           src/cpu/REGISTER_BANK.cpp \
           src/cpu/ULA.cpp \
           src/IO/IOManager.cpp \
           src/memory/cache.cpp \
           src/memory/cachePolicy.cpp \
           src/memory/MAIN_MEMORY.cpp \
           src/memory/MemoryManager.cpp \
           src/memory/SECONDARY_MEMORY.cpp \
           src/parser_json/parser_json.cpp
```

### Dependências

Cada `.cpp` gera um `.o` correspondente:
- `src/main.o`
- `src/cpu/Core.o`
- `src/cpu/RoundRobinScheduler.o`
- etc.

O Makefile usa regra genérica:
```makefile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
```

---

## 📝 Exemplos Práticos

### Exemplo 1: Primeira Compilação

```bash
cd /mnt/c/Users/Henrique/Documents/github/SO-SimuladorVonNeumann
make clean
make simulador
cp src/tasks/tasks.json .
./simulador
```

### Exemplo 2: Mudança no RoundRobinScheduler

```bash
# Editar src/cpu/RoundRobinScheduler.cpp
# ...

# Recompilar (apenas RoundRobinScheduler.o será recompilado)
make simulador

# Executar
make run-sim
```

### Exemplo 3: Debug de Segfault

```bash
make clean
make CXXFLAGS="-Wall -Wextra -g -std=c++17 -Isrc -O0 -ggdb3" simulador
gdb ./simulador
> run
> bt    # backtrace quando crashar
> quit
```

### Exemplo 4: Verificação Antes de Commit

```bash
make clean
make check
make simulador
make test-all
```

---

## ⚙️ Customização

### Adicionar Novo Arquivo ao Simulador

Edite o `Makefile`, adicione o arquivo em `SRC_SIM`:

```makefile
SRC_SIM := src/main.cpp \
           src/cpu/Core.cpp \
           src/cpu/RoundRobinScheduler.cpp \
           src/cpu/NovoModulo.cpp \     # ← NOVO
           ...
```

Depois recompile:
```bash
make clean
make simulador
```

### Alterar Flags de Compilação

Para adicionar flags temporariamente:

```bash
make CXXFLAGS="-Wall -Wextra -g -std=c++17 -Isrc -O2" simulador
```

Para mudança permanente, edite o `Makefile`:

```makefile
CXXFLAGS := -Wall -Wextra -g -std=c++17 -Isrc -O2
```

---

## 🚨 Troubleshooting

### Erro: "make: command not found"

```bash
sudo apt install make
```

### Erro: "g++: command not found"

```bash
sudo apt install build-essential
```

### Erro: "undefined reference to pthread_create"

Certifique-se que `LDFLAGS = -lpthread` está no Makefile.

### Recompilação Desnecessária

Se `make` recompila tudo sempre:

```bash
# Verificar timestamps
ls -la src/cpu/*.o

# Forçar limpeza
make clean
make simulador
```

### Warnings Persistentes

Para suprimir warnings específicos (não recomendado):

```bash
make CXXFLAGS="-std=c++17 -Isrc -Wno-unused-parameter" simulador
```

---

## 📚 Referências

- [GNU Make Manual](https://www.gnu.org/software/make/manual/)
- [GCC Options](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html)
- Arquivo principal: `Makefile` na raiz do projeto

---

**Última atualização:** 13/11/2025  
**Versão do Makefile:** Atualizada com `simulador` e `RoundRobinScheduler`
