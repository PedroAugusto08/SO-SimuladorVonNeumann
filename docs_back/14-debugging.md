# Debugging e Troubleshooting

## 🎯 Objetivo

Fornecer técnicas e ferramentas para identificar e corrigir problemas no simulador multicore.

---

## 🔍 Estratégias de Debugging

### 1. Debugging Sistemático

```
1. Reproduzir o erro consistentemente
2. Isolar o componente problemático
3. Identificar a causa raiz
4. Implementar correção
5. Validar correção
6. Prevenir regressão
```

---

## 🐛 Problemas Comuns

### Problema 1: Deadlock

**Sintomas:**
- Sistema trava
- CPU usage baixo
- Processos em estado WAITING

**Diagnóstico:**
```cpp
// Adicionar timeout em locks
std::timed_mutex mutex;

if (!mutex.try_lock_for(std::chrono::seconds(5))) {
    std::cerr << "Possível deadlock detectado!\n";
    // Dump de estado
    dumpLockState();
}
```

**Soluções:**
- Sempre adquirir locks na mesma ordem
- Usar `std::scoped_lock` para múltiplos locks
- Implementar timeout em todas as operações bloqueantes

---

### Problema 2: Race Condition

**Sintomas:**
- Resultados inconsistentes
- Crashes aleatórios
- Corrupção de dados

**Diagnóstico:**
```bash
# Compilar com ThreadSanitizer
g++ -fsanitize=thread -g -O1 -o simulador main.cpp

# Executar
./simulador
```

**Exemplo de Saída:**
```
==================
WARNING: ThreadSanitizer: data race
  Write of size 4 at 0x7b0400000020 by thread T1:
    #0 Core::incrementCounter() core.cpp:42
    
  Previous read of size 4 at 0x7b0400000020 by thread T2:
    #0 Core::getCounter() core.cpp:47
```

**Soluções:**
- Proteger variáveis compartilhadas com mutex
- Usar std::atomic para contadores simples
- Implementar RAII para locks

---

### Problema 3: Memory Leak

**Sintomas:**
- Memória aumenta continuamente
- Sistema fica lento com tempo
- EventualmenteOut of Memory

**Diagnóstico:**
```bash
# Valgrind
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./simulador
```

**Exemplo de Saída:**
```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 4,096 bytes in 1 blocks
==12345==   total heap usage: 1,000 allocs, 999 frees, 1,048,576 bytes allocated
==12345== 
==12345== 4,096 bytes in 1 blocks are definitely lost
==12345==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x400537: Core::allocateBuffer() (core.cpp:25)
```

**Soluções:**
- Usar smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Sempre deletar memória alocada
- Implementar RAII para recursos

```cpp
// ERRADO
void processData() {
    int* data = new int[1000];
    // ... processar
    // ESQUECEU de deletar!
}

// CORRETO
void processData() {
    std::unique_ptr<int[]> data(new int[1000]);
    // ... processar
    // Deletado automaticamente
}

// AINDA MELHOR
void processData() {
    std::vector<int> data(1000);
    // ... processar
}
```

---

### Problema 4: Segmentation Fault

**Sintomas:**
- Crash imediato
- Core dump gerado
- Mensagem: "Segmentation fault"

**Diagnóstico:**
```bash
# Gerar core dump
ulimit -c unlimited
./simulador
# Se crashar, gera arquivo core

# Debugar com gdb
gdb ./simulador core
(gdb) bt  # Backtrace
(gdb) frame 0  # Examinar frame
(gdb) print variable  # Ver valor
```

**Causas Comuns:**
1. Acesso a ponteiro nulo
2. Array out of bounds
3. Double free
4. Stack overflow

**Soluções:**
```cpp
// Sempre verificar ponteiros
if (ptr != nullptr) {
    ptr->method();
}

// Usar .at() ao invés de []
vector[index];  // Não verifica bounds
vector.at(index);  // Lança exceção se inválido

// Verificar limites de array
if (index >= 0 && index < array_size) {
    array[index] = value;
}
```

---

### Problema 5: Performance Baixa

**Sintomas:**
- Execução muito lenta
- CPU usage alto mas pouco progresso
- Contenção em locks

**Diagnóstico:**
```bash
# Profiling com gprof
g++ -pg -o simulador main.cpp
./simulador
gprof simulador gmon.out > analysis.txt
```

**Análise:**
```
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  Ts/call  Ts/call  name    
 45.67      2.35     2.35  1000000     0.00     0.00  Core::lockMutex()
 23.45      3.56     1.21   500000     0.00     0.00  Scheduler::getNext()
```

**Soluções:**
- Reduzir seções críticas
- Usar locks mais granulares
- Implementar lock-free data structures
- Otimizar algoritmos

---

## 🛠️ Ferramentas de Debugging

### 1. GDB (GNU Debugger)

```bash
# Compilar com símbolos de debug
g++ -g -o simulador main.cpp

# Iniciar GDB
gdb ./simulador

# Comandos úteis
(gdb) break main.cpp:42        # Breakpoint
(gdb) run                       # Executar
(gdb) next                      # Próxima linha
(gdb) step                      # Entrar em função
(gdb) print variable            # Ver valor
(gdb) backtrace                 # Call stack
(gdb) watch variable            # Watchpoint
(gdb) continue                  # Continuar
```

---

### 2. Valgrind

```bash
# Memory leaks
valgrind --leak-check=full ./simulador

# Race conditions (Helgrind)
valgrind --tool=helgrind ./simulador

# Cache misses (Cachegrind)
valgrind --tool=cachegrind ./simulador
```

---

### 3. AddressSanitizer

```bash
# Compilar
g++ -fsanitize=address -g -o simulador main.cpp

# Executar (detecta automaticamente)
./simulador
```

**Detecta:**
- Use after free
- Heap buffer overflow
- Stack buffer overflow
- Memory leaks
- Use after return

---

### 4. ThreadSanitizer

```bash
# Compilar
g++ -fsanitize=thread -g -O1 -o simulador main.cpp

# Executar
./simulador
```

**Detecta:**
- Data races
- Deadlocks
- Thread leaks

---

## 📊 Logging Efetivo

### Sistema de Log

```cpp
// Logger.hpp
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <iomanip>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
private:
    static std::mutex log_mutex;
    static LogLevel current_level;
    static std::ofstream log_file;
    
public:
    static void setLevel(LogLevel level) {
        current_level = level;
    }
    
    static void setFile(const std::string& filename) {
        log_file.open(filename, std::ios::app);
    }
    
    template<typename... Args>
    static void log(LogLevel level, Args... args) {
        if (level < current_level) return;
        
        std::lock_guard<std::mutex> lock(log_mutex);
        
        // Timestamp
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << " [" << levelToString(level) << "] ";
        
        // Mensagem
        ((ss << args), ...);
        
        std::string message = ss.str();
        
        // Output
        std::cout << message << std::endl;
        if (log_file.is_open()) {
            log_file << message << std::endl;
        }
    }
    
private:
    static std::string levelToString(LogLevel level) {
        switch(level) {
            case DEBUG: return "DEBUG";
            case INFO: return "INFO";
            case WARNING: return "WARN";
            case ERROR: return "ERROR";
            case CRITICAL: return "CRIT";
        }
        return "UNKNOWN";
    }
};

// Macros convenientes
#define LOG_DEBUG(...) Logger::log(DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::log(INFO, __VA_ARGS__)
#define LOG_WARN(...) Logger::log(WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::log(CRITICAL, __VA_ARGS__)

#endif // LOGGER_HPP
```

**Uso:**
```cpp
LOG_DEBUG("Core ", core_id, " iniciado");
LOG_INFO("Processo ", pid, " completado em ", time, "ms");
LOG_WARN("Fila de processos vazia no core ", core_id);
LOG_ERROR("Falha ao alocar memória: ", size, " bytes");
LOG_CRITICAL("Sistema em deadlock!");
```

---

## 🔍 Técnicas Avançadas

### 1. Assertions

```cpp
#include <cassert>

void processData(int* data, int size) {
    assert(data != nullptr && "Data pointer is null!");
    assert(size > 0 && "Size must be positive!");
    
    // Processar...
}

// Em produção, desabilitar
// g++ -DNDEBUG ...
```

---

### 2. Instrumentation

```cpp
class InstrumentedMutex {
private:
    std::mutex mutex;
    std::atomic<int> contentions{0};
    std::atomic<int> acquisitions{0};
    
public:
    void lock() {
        if (!mutex.try_lock()) {
            contentions++;
            mutex.lock();
        }
        acquisitions++;
    }
    
    void unlock() {
        mutex.unlock();
    }
    
    double getContentionRate() {
        return (double)contentions / acquisitions;
    }
};
```

---

### 3. State Dumping

```cpp
void MultiCore::dumpState() {
    std::ofstream dump("system_dump.txt");
    
    dump << "===== SYSTEM STATE DUMP =====\n\n";
    
    // Cores
    dump << "CORES:\n";
    for (auto& core : cores) {
        dump << "  Core " << core->getCoreId() << ":\n";
        dump << "    State: " << (core->isBusy() ? "BUSY" : "IDLE") << "\n";
        if (core->getCurrentProcess()) {
            dump << "    Process: " << core->getCurrentProcess()->pid << "\n";
        }
    }
    
    // Scheduler
    dump << "\nSCHEDULER:\n";
    dump << "  Queue size: " << scheduler->getQueueSize() << "\n";
    
    // Memory
    dump << "\nMEMORY:\n";
    dump << "  Available: " << memory_manager->getAvailableMemory() << "\n";
    dump << "  Page faults: " << memory_manager->getPageFaults() << "\n";
    
    dump.close();
}
```

---

## 📝 Checklist de Debugging

Ao encontrar um bug:

- [ ] Reproduzir consistentemente
- [ ] Criar caso de teste mínimo
- [ ] Isolar componente problemático
- [ ] Adicionar logging relevante
- [ ] Usar ferramentas apropriadas (gdb, valgrind, etc.)
- [ ] Identificar causa raiz
- [ ] Implementar correção
- [ ] Adicionar teste de regressão
- [ ] Documentar solução

---

## 🆘 FAQ de Problemas

**Q: Sistema trava ao adicionar muitos processos**

A: Provável deadlock ou starvation. Verifique:
- Ordem de aquisição de locks
- Condition variables com timeout
- Fila de processos não está bloqueando

**Q: Resultados diferentes a cada execução**

A: Race condition. Use ThreadSanitizer para detectar.

**Q: Memória aumenta indefinidamente**

A: Memory leak. Use Valgrind e verifique smart pointers.

**Q: Performance muito baixa com mais cores**

A: Contenção excessiva ou overhead de sincronização.
- Profile com gprof
- Reduzir seções críticas
- Usar locks mais granulares

---

## 🔗 Recursos Úteis

### Documentação
- GDB Manual: https://sourceware.org/gdb/documentation/
- Valgrind User Manual: https://valgrind.org/docs/manual/
- C++ Threading Guide: https://en.cppreference.com/w/cpp/thread

### Livros
- KERNIGHAN, B. The Practice of Programming
- MCCONNELL, S. Code Complete

---

## 🔗 Próximos Passos

- ➡️ [Estrutura do Artigo IEEE](15-estrutura-artigo.md)
- ➡️ [FAQ](20-faq.md)
- ➡️ [Troubleshooting](23-troubleshooting.md)
