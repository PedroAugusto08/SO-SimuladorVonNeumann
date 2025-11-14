# 🔍 Investigação: Cache L1 Thread-Local não Funciona

**Data:** 15/11/2025  
**Problema:** Speedup negativo em arquitetura multicore devido à cache L1 não ser utilizada  
**Status:** 🔴 **EM INVESTIGAÇÃO CRÍTICA - POSSÍVEL PROBLEMA DO WSL**

---

## 📊 Sintomas Observados

### 1. **Speedup Negativo**
```
1 núcleo:  3.40ms (baseline 1.00x)
2 núcleos: 4.00ms (0.85x) ❌ 17% mais LENTO
4 núcleos: 13.7ms (0.25x) ❌ 302% mais LENTO  
8 núcleos: 18.0ms (0.19x) ❌ 429% mais LENTO
```

### 2. **Cache Nunca Utilizada**
```
Cache Hits: 0
Cache Misses: 0
RAM Accesses: 110 (1 core) → 220 (2) → 440 (4) → 880 (8)
```

### 3. **Thread-Local Sempre NULL**
```
[READ #1] l1_cache=(nil)
[READ #2] l1_cache=(nil)
[READ #150] l1_cache=(nil)
...todos os 1500+ reads com cache=NULL
```

---

## 🔬 Investigação Realizada

### **Fase 1: Verificação de Locks e Sincronização**

#### Tentativa 1: Mutex Recursivo
- **Implementação:** `std::recursive_mutex` no MemoryManager
- **Resultado:** ❌ Funcionou sem crashes, mas speedup negativo (-69%)
- **Causa:** Mutex global serializa todos os acessos

#### Tentativa 2: Shared Mutex
- **Implementação:** `std::shared_mutex` (leituras paralelas, escritas exclusivas)
- **Resultado:** ⚠️ Melhorou ligeiramente mas ainda negativo
- **Causa:** Contenção no lock mesmo com shared_lock

---

### **Fase 2: Cache L1 Privada por Core**

#### Arquitetura Implementada
```
Core 0 → Cache L1 privada (único dono)
Core 1 → Cache L1 privada (único dono)
Core N → Cache L1 privada (único dono)
         ↓ (apenas em cache miss)
      MemoryManager (RAM + Disco compartilhado)
```

#### Tentativa 3: Mapa Global thread_id → Cache*
```cpp
std::unordered_map<std::thread::id, Cache*> thread_caches;
std::mutex cache_map_mutex;

Cache* getCurrentCache() {
    std::lock_guard<std::mutex> lock(cache_map_mutex);
    return thread_caches[std::this_thread::get_id()];
}
```
- **Resultado:** ❌ Ainda speedup negativo
- **Causa:** Lock do mapa serializa tudo novamente

#### Tentativa 4: Thread-Local Storage (ATUAL)
```cpp
// MemoryManager.hpp
class MemoryManager {
    static thread_local Cache* current_thread_cache;
public:
    static void setThreadCache(Cache* cache);
    static Cache* getThreadCache();
};

// MemoryManager.cpp
thread_local Cache* MemoryManager::current_thread_cache = nullptr;
```

**Core registra sua cache:**
```cpp
void Core::run_process(PCB* process) {
    // Linha 72 em src/cpu/Core.cpp
    MemoryManager::setThreadCache(L1_cache.get());
    
    // ... executa instruções ...
}
```

---

### **Fase 3: Debugging Profundo**

#### Descoberta 1: Reads Acontecem, mas Cache é NULL
```
Total reads: 1500+
Reads com cache != NULL: 0
```

#### Descoberta 2: setThreadCache() Nunca é Chamado
```cpp
void MemoryManager::setThreadCache(Cache* l1_cache) {
    std::cout << "@@@ setThreadCache CHAMADO! @@@" << std::endl;
    current_thread_cache = l1_cache;
}
```
**Output esperado:** "@@@ setThreadCache CHAMADO! @@@"  
**Output real:** 🔴 **NADA!**

#### Descoberta 3: run_process() Aparentemente Não Executa Início
```cpp
void Core::run_process(PCB* process) {
    std::cout << "AAAAAAA run_process INICIOU!" << std::endl; // NUNCA APARECE
    std::cout << "BBBBBBB setando cache" << std::endl;        // NUNCA APARECE
    MemoryManager::setThreadCache(L1_cache.get());            // NUNCA EXECUTA?
    
    // ... resto do código ...
    
    control_unit.Fetch(); // ← ESTE EXECUTA! (vemos output "[FETCH] PC=")
}
```

**Paradoxo Impossível:**
- ✅ `control_unit.Fetch()` executa (linha ~123)
- ❌ `std::cout << "AAA"` NÃO executa (linha 70)
- **Como é possível executar linha 123 sem passar pela linha 70?!**

#### Descoberta 4: Output de Debug Não Aparece
Testamos múltiplas formas de debug:
```cpp
std::cout << "DEBUG" << std::endl;           // ❌ Não aparece
std::cerr << "DEBUG" << std::endl;           // ❌ Não aparece
fprintf(stderr, "DEBUG\n");                  // ❌ Não aparece
printf("DEBUG\n"); fflush(stdout);           // ❌ Não aparece
std::ofstream("/tmp/log.txt") << "DEBUG";    // ❌ Arquivo não criado
```

**Testado com:**
- SilentMode desabilitado ✅
- Compilação -O0 -g (sem otimizações) ✅
- `make clean && make` ✅
- Verificação com `strings test_multicore | grep "AAA"` → **ZERO resultados**

#### Descoberta 5: CRÍTICA - Código Não Está no Binário
```bash
# Código está no arquivo fonte
$ grep "AAAAAAA" src/cpu/Core.cpp
70:    std::cout << "AAAAAAA run_process INICIOU!" << std::endl;

# MAS não está no executável compilado!
$ strings test_multicore | grep "AAAAAAA"
(nenhum resultado)

# Recompilação completa também falha
$ rm -f test_multicore && make clean && make
$ strings test_multicore | grep "AAAAAAA"
(nenhum resultado)
```

---

## 🤔 Hipóteses Testadas (e Descartadas)

| # | Hipótese | Teste Realizado | Resultado |
|---|----------|-----------------|-----------|
| 1 | Cache não inicializada | Verificar `Core::Core()` cria `L1_cache` | ✅ Cache criada |
| 2 | Ponteiro NULL passado | Debug `setThreadCache(nullptr)` | ❌ Nunca chamado |
| 3 | Thread errada | Verificar `std::this_thread::get_id()` | ✅ IDs corretos |
| 4 | Ordem de destruição | Usar scope explícito | ✅ Corrigido |
| 5 | SilentMode suprime output | Desabilitar SilentMode | ❌ Continua sem aparecer |
| 6 | Otimização remove código | Compilar com `-O0 -g` | ❌ Continua |
| 7 | Arquivo não recompilado | `make clean && make` | ❌ Continua |
| 8 | Buffer não flushed | `fflush(stdout)` após cada print | ❌ Continua |
| 9 | Código em outro arquivo | `find . -name "*.cpp" \| xargs grep run_process` | ✅ Só 1 definição |
| 10 | Binário antigo | `ls -lah test_multicore` + `strings` | ❌ Debug "AAA" NÃO está no binário |
| 11 | Cache corrompido | `rm -rf build/ *.o` + recompilação | ❌ Continua |

---

## 🚨 Problema Atual (CRÍTICO)

### **O Mistério Impossível**

O código fonte em `src/cpu/Core.cpp` contém:
```cpp
70:  std::cout << "AAAAAAA run_process INICIOU!" << std::endl;
71:  std::cout << "BBBBBBB setando cache" << std::endl;
72:  MemoryManager::setThreadCache(L1_cache.get());
...
123: control_unit.Fetch(); // ← ESTE EXECUTA (vemos "[FETCH] PC=")
```

**Observações:**
1. ✅ Linha 123 executa (provado pelo output `[FETCH] PC=`)
2. ❌ Linhas 70-72 NÃO executam (zero output)
3. ❌ `strings test_multicore | grep "AAAA"` retorna **ZERO**
4. ✅ `grep "AAAA" src/cpu/Core.cpp` retorna linha 70

**Conclusões possíveis:**
- O binário `test_multicore` **NÃO contém** o código das linhas 70-72
- Há uma **versão antiga** sendo linkada (mesmo após `make clean`)
- O WSL tem **cache de compilação corrompido** ou problema de filesystem
- Problema de **thread-local storage no WSL**
- **Compilação incremental quebrada** (Windows filesystem + WSL)

---

## 🧪 Testes a Realizar (ORDEM DE PRIORIDADE)

### **Teste 1: 🔴 URGENTE - Verificar se thread_local funciona no WSL**

Criar arquivo `test_thread_local.cpp`:
```cpp
#include <iostream>
#include <thread>
#include <vector>

thread_local int* my_ptr = nullptr;

void set_ptr(int* p) {
    std::cout << "SET: Thread " << std::this_thread::get_id() 
              << " ptr=" << (void*)p << std::endl;
    my_ptr = p;
    std::cout << "AFTER SET: my_ptr=" << (void*)my_ptr << std::endl;
}

int* get_ptr() {
    std::cout << "GET: Thread " << std::this_thread::get_id() 
              << " ptr=" << (void*)my_ptr << std::endl;
    return my_ptr;
}

void thread_func(int id) {
    int local_value = id * 100;
    std::cout << "\n=== Thread " << id << " START ===" << std::endl;
    
    set_ptr(&local_value);
    
    int* retrieved = get_ptr();
    
    if (retrieved == &local_value) {
        std::cout << "Thread " << id << ": ✅ SUCCESS! ptr matches" << std::endl;
    } else {
        std::cout << "Thread " << id << ": ❌ FAIL! ptr mismatch" << std::endl;
    }
}

int main() {
    std::cout << "Testing thread_local in WSL..." << std::endl;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(thread_func, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nTest completed!" << std::endl;
    return 0;
}
```

**Comando:**
```bash
cd /mnt/c/Users/Henrique/Documents/github/SO-SimuladorVonNeumann
g++ -std=c++17 -pthread test_thread_local.cpp -o test_thread_local
./test_thread_local
```

**Resultado Esperado:**
```
Thread 0: ✅ SUCCESS!
Thread 1: ✅ SUCCESS!
Thread 2: ✅ SUCCESS!
Thread 3: ✅ SUCCESS!
```

**Se falhar:** WSL tem problema com thread_local, precisaremos usar outra abordagem.

---

### **Teste 2: 🔴 URGENTE - Verificar símbolos no executável**

```bash
# Ver se run_process está linkado corretamente
nm test_multicore | grep "run_process"

# Ver se strings de debug estão no binário
strings test_multicore | grep -E "(AAAA|BBBB|setThreadCache)"

# Ver símbolos thread_local
objdump -t test_multicore | grep "current_thread_cache"

# Ver se há múltiplas versões de MemoryManager::read
nm test_multicore | grep "MemoryManager.*read"
```

---

### **Teste 3: 🟡 IMPORTANTE - Compilar em Linux puro (não-WSL)**

Opções:
1. Docker com Ubuntu
2. VM Linux
3. Linux nativo em dual-boot

```bash
# No ambiente Linux nativo
git clone https://github.com/PedroAugusto08/SO-SimuladorVonNeumann
cd SO-SimuladorVonNeumann
git checkout tetste
make clean
make test-multicore
./test_multicore 2>&1 | grep -E "(AAAA|setThreadCache|Cache Hits)"
```

---

### **Teste 4: 🟡 IMPORTANTE - GDB Step Debugging**

```bash
gdb ./test_multicore

# Dentro do GDB:
(gdb) break Core::run_process
(gdb) run
(gdb) info threads
(gdb) thread 2  # mudar para thread do worker
(gdb) step
(gdb) print L1_cache.get()
(gdb) print (void*)&current_thread_cache
(gdb) continue
```

---

### **Teste 5: 🟢 ALTERNATIVO - Strace System Calls**

```bash
strace -f -e trace=write,writev,clone,mmap ./test_multicore 2>&1 | head -200
```

Isso mostra se `write()` syscall é chamado mas output não chega no terminal.

---

### **Teste 6: 🟢 ALTERNATIVO - Copiar código para /tmp (fora do Windows filesystem)**

WSL pode ter problemas com arquivos em `/mnt/c/`:

```bash
# Copiar para filesystem nativo do Linux
cp -r /mnt/c/Users/Henrique/Documents/github/SO-SimuladorVonNeumann /tmp/test_sim
cd /tmp/test_sim
make clean
make
./test_multicore 2>&1 | grep "AAAA"
```

---

## 📝 Arquivos Relevantes

### **Código Atual (com bugs)**

**`src/memory/MemoryManager.hpp`** (linhas chave):
```cpp
class MemoryManager {
private:
    static thread_local Cache* current_thread_cache; // linha ~100
    mutable std::shared_mutex memory_mutex;
    
public:
    static void setThreadCache(Cache* cache);
    static Cache* getThreadCache();
    
    uint32_t read(uint32_t address, PCB& process);
    void write(uint32_t address, uint32_t data, PCB& process);
};
```

**`src/memory/MemoryManager.cpp`** (linhas chave):
```cpp
// linha ~11: definição da variável thread_local
thread_local Cache* MemoryManager::current_thread_cache = nullptr;

// linha ~20: setThreadCache
void MemoryManager::setThreadCache(Cache* l1_cache) {
    std::cout << "@@@ [setThreadCache] CHAMADO! @@@" << std::endl;
    current_thread_cache = l1_cache;
    std::cout << "@@@ [setThreadCache] SETADO! @@@" << std::endl;
}

// linha ~40: read() usa current_thread_cache
uint32_t MemoryManager::read(uint32_t address, PCB& process) {
    Cache* l1_cache = current_thread_cache; // SEMPRE NULL!
    
    if (l1_cache) {
        // nunca entra aqui
    }
}
```

**`src/cpu/Core.cpp`** (linhas chave):
```cpp
// linha 70-72: Deveria registrar cache mas NUNCA EXECUTA
void Core::run_process(PCB* process) {
    std::cout << "AAAAAAA run_process INICIOU!" << std::endl; // NÃO APARECE
    MemoryManager::setThreadCache(L1_cache.get());            // NUNCA EXECUTA
    
    // ... código ...
    
    control_unit.Fetch(); // ← ESTE EXECUTA (linha ~123)
}
```

---

## 🎯 Próximos Passos (AÇÃO IMEDIATA)

### **PASSO 1: Testar thread_local no WSL**
```bash
# Criar e executar test_thread_local.cpp
g++ -std=c++17 -pthread test_thread_local.cpp -o test_thread_local
./test_thread_local
```
- ✅ Se funcionar → problema é outro
- ❌ Se falhar → WSL não suporta thread_local corretamente

### **PASSO 2: Verificar binário**
```bash
strings test_multicore | grep -i "aaa"
nm test_multicore | grep "setThreadCache"
```

### **PASSO 3: Copiar para filesystem Linux**
```bash
cp -r . /tmp/sim_test
cd /tmp/sim_test
make clean && make
./test_multicore 2>&1 | head -100
```

### **PASSO 4: Se nada funcionar - Abordagem Alternativa**

Em vez de thread_local, passar Cache* como parâmetro:

```cpp
// MemoryManager.hpp
uint32_t read(uint32_t address, PCB& process, Cache* l1_cache = nullptr);
void write(uint32_t address, uint32_t data, PCB& process, Cache* l1_cache = nullptr);

// Core.cpp
void Core::run_process(PCB* process) {
    // Passar cache explicitamente
    memory_manager->read(addr, *process, L1_cache.get());
}
```

---

## 📚 Referências

- **Thread-local storage:** https://en.cppreference.com/w/cpp/language/storage_duration
- **WSL known issues:** https://github.com/microsoft/WSL/issues
- **GCC thread_local bugs:** https://gcc.gnu.org/bugzilla/
- **Shared mutex:** https://en.cppreference.com/w/cpp/thread/shared_mutex

---

## 💾 Estado Atual do Código

**Branch:** tetste  
**Arquivos modificados:**
- ✅ `src/memory/MemoryManager.hpp` - thread_local implementado
- ✅ `src/memory/MemoryManager.cpp` - thread_local implementado
- ⚠️ `src/cpu/Core.cpp` - debug não aparece no binário
- ✅ `test_multicore.cpp` - testes de escalabilidade

**Para reverter para versão estável:**
```bash
git stash
git checkout main
make clean && make
```

---

## 🐛 Issues Conhecidos do WSL

### Thread-Local Storage
- WSL1 tinha problemas conhecidos com `thread_local`
- WSL2 melhorou mas ainda pode ter bugs
- Filesystem do Windows (`/mnt/c/`) pode causar problemas de compilação

### Workarounds Possíveis
1. Usar filesystem nativo do Linux (`/home/` ou `/tmp/`)
2. Atualizar WSL: `wsl --update`
3. Usar `__thread` em vez de `thread_local` (GNU extension)
4. Compilar em Docker/VM Linux

---

**FIM DO DOCUMENTO**

---

**RESUMO PARA PRÓXIMA SESSÃO:**
1. 🔴 Executar `test_thread_local.cpp` para confirmar se WSL suporta thread_local
2. 🔴 Testar compilação em filesystem Linux puro (`/tmp/`)
3. 🔴 Verificar com `nm` e `strings` se símbolos estão no binário
4. 🟡 Se falhar tudo: implementar passagem explícita de Cache* como parâmetro
