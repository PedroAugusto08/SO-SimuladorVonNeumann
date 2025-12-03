# Sincronização e Concorrência

## 🎯 Objetivo

Implementar mecanismos de sincronização para evitar race conditions, deadlocks e garantir acesso seguro a recursos compartilhados no sistema multicore.

---

## 🔒 Problemas de Concorrência

### 1. Race Conditions
Ocorre quando múltiplos cores acessam/modificam dados compartilhados simultaneamente.

**Exemplo Problemático:**
```cpp
// SEM sincronização - ERRADO!
int shared_counter = 0;

void Core::incrementCounter() {
    shared_counter++; // Não é atômico!
}
```

**Solução:**
```cpp
// COM sincronização - CORRETO!
std::mutex counter_mutex;
int shared_counter = 0;

void Core::incrementCounter() {
    std::lock_guard<std::mutex> lock(counter_mutex);
    shared_counter++;
}
```

---

### 2. Deadlocks
Ocorre quando dois ou mais cores esperam indefinidamente por recursos mantidos mutuamente.

**Exemplo Problemático:**
```cpp
// Core 1
lock(mutex_A);
lock(mutex_B);

// Core 2
lock(mutex_B); // Ordem diferente!
lock(mutex_A);
```

**Solução: Ordenação de Locks**
```cpp
// Sempre adquirir locks na mesma ordem
void acquireMultipleLocks() {
    std::scoped_lock lock(mutex_A, mutex_B); // C++17
}
```

---

### 3. Starvation
Um processo nunca consegue acesso a recursos.

**Solução: Fairness no Scheduler**
```cpp
void RoundRobinScheduler::checkStarvation() {
    for (auto& process : ready_queue) {
        if (process->waiting_time > MAX_WAIT_TIME) {
            // Aumentar prioridade
            process->priority++;
        }
    }
}
```

---

## 🔧 Mecanismos de Sincronização

### 1. Mutex (Mutual Exclusion)

```cpp
class SharedResource {
private:
    std::mutex resource_mutex;
    int data;
    
public:
    void write(int value) {
        std::lock_guard<std::mutex> lock(resource_mutex);
        data = value;
    }
    
    int read() {
        std::lock_guard<std::mutex> lock(resource_mutex);
        return data;
    }
};
```

**Tipos de Mutex:**

| Tipo | Descrição | Uso |
|------|-----------|-----|
| `std::mutex` | Básico, não recursivo | Proteção simples |
| `std::recursive_mutex` | Permite locks aninhados | Funções recursivas |
| `std::timed_mutex` | Permite timeout | Evitar deadlock |
| `std::shared_mutex` | Leitura compartilhada | Múltiplos leitores |

---

### 2. Condition Variables

Para notificação entre threads:

```cpp
class ProcessQueue {
private:
    std::queue<std::shared_ptr<PCB>> queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    
public:
    void enqueue(std::shared_ptr<PCB> process) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            queue.push(process);
        }
        cv.notify_one(); // Acordar um core esperando
    }
    
    std::shared_ptr<PCB> dequeue() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        // Esperar até haver processo disponível
        cv.wait(lock, [this]{ return !queue.empty(); });
        
        auto process = queue.front();
        queue.pop();
        return process;
    }
    
    void notifyAll() {
        cv.notify_all(); // Acordar todos os cores
    }
};
```

---

### 3. Atomic Operations

Para operações simples sem mutex:

```cpp
#include <atomic>

class SystemStats {
private:
    std::atomic<int> total_processes{0};
    std::atomic<int> completed_processes{0};
    std::atomic<bool> system_running{true};
    
public:
    void incrementProcesses() {
        total_processes.fetch_add(1, std::memory_order_relaxed);
    }
    
    void markCompleted() {
        completed_processes.fetch_add(1, std::memory_order_release);
    }
    
    bool isRunning() const {
        return system_running.load(std::memory_order_acquire);
    }
    
    void shutdown() {
        system_running.store(false, std::memory_order_release);
    }
};
```

**Memory Orders:**
- `memory_order_relaxed`: Sem ordenação
- `memory_order_acquire`: Sincroniza leituras
- `memory_order_release`: Sincroniza escritas
- `memory_order_seq_cst`: Ordem sequencial consistente (padrão)

---

### 4. Semáforos (C++20)

```cpp
#include <semaphore>

class ResourcePool {
private:
    std::counting_semaphore<10> semaphore{10}; // 10 recursos
    
public:
    void acquireResource() {
        semaphore.acquire(); // Decrementa contador
    }
    
    void releaseResource() {
        semaphore.release(); // Incrementa contador
    }
    
    bool tryAcquire() {
        return semaphore.try_acquire(); // Não-bloqueante
    }
};
```

---

## 🛡️ Padrões de Sincronização

### 1. RAII (Resource Acquisition Is Initialization)

```cpp
class LockGuard {
private:
    std::mutex& mutex_ref;
    
public:
    LockGuard(std::mutex& m) : mutex_ref(m) {
        mutex_ref.lock();
    }
    
    ~LockGuard() {
        mutex_ref.unlock();
    }
    
    // Prevent copying
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

// Uso
void criticalSection() {
    LockGuard guard(my_mutex);
    // Código crítico
    // Mutex é automaticamente liberado ao sair do escopo
}
```

---

### 2. Reader-Writer Lock

Permite múltiplos leitores ou um único escritor:

```cpp
class SharedData {
private:
    std::shared_mutex rw_mutex;
    std::vector<int> data;
    
public:
    // Múltiplos leitores podem executar simultaneamente
    int read(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(rw_mutex);
        return data[index];
    }
    
    // Apenas um escritor por vez
    void write(size_t index, int value) {
        std::unique_lock<std::shared_mutex> lock(rw_mutex);
        data[index] = value;
    }
};
```

---

### 3. Double-Checked Locking

Para inicialização lazy thread-safe:

```cpp
class Singleton {
private:
    static std::atomic<Singleton*> instance;
    static std::mutex init_mutex;
    
    Singleton() {}
    
public:
    static Singleton* getInstance() {
        Singleton* tmp = instance.load(std::memory_order_acquire);
        
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(init_mutex);
            tmp = instance.load(std::memory_order_relaxed);
            
            if (tmp == nullptr) {
                tmp = new Singleton();
                instance.store(tmp, std::memory_order_release);
            }
        }
        
        return tmp;
    }
};
```

---

## 🔄 Sincronização no Scheduler

### Fila de Processos Thread-Safe

```cpp
class ThreadSafeQueue {
private:
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::deque<std::shared_ptr<PCB>> queue;
    bool done = false;
    
public:
    void push(std::shared_ptr<PCB> process) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push_back(process);
        }
        cv.notify_one();
    }
    
    bool pop(std::shared_ptr<PCB>& process) {
        std::unique_lock<std::mutex> lock(mutex);
        
        cv.wait(lock, [this] {
            return !queue.empty() || done;
        });
        
        if (queue.empty()) {
            return false;
        }
        
        process = queue.front();
        queue.pop_front();
        return true;
    }
    
    void finish() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            done = true;
        }
        cv.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
};
```

---

## 🚦 Barreiras de Sincronização

Para coordenar múltiplos cores:

```cpp
class Barrier {
private:
    std::mutex mutex;
    std::condition_variable cv;
    size_t count;
    size_t waiting;
    size_t generation;
    
public:
    Barrier(size_t thread_count) 
        : count(thread_count), waiting(0), generation(0) {}
    
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        size_t gen = generation;
        
        if (++waiting == count) {
            // Último a chegar
            waiting = 0;
            generation++;
            cv.notify_all();
        } else {
            // Esperar até todos chegarem
            cv.wait(lock, [this, gen] {
                return gen != generation;
            });
        }
    }
};

// Uso
void Core::synchronizeAtBarrier() {
    // Executar até aqui
    barrier.wait();
    // Todos os cores chegaram, continuar
}
```

---

## 🐛 Detecção e Prevenção de Problemas

### 1. Detecção de Deadlock

```cpp
class DeadlockDetector {
private:
    std::map<int, std::set<int>> wait_for_graph;
    
public:
    void addEdge(int from, int to) {
        wait_for_graph[from].insert(to);
    }
    
    bool hasCycle() {
        std::set<int> visited;
        std::set<int> rec_stack;
        
        for (auto& pair : wait_for_graph) {
            if (hasCycleUtil(pair.first, visited, rec_stack)) {
                return true;
            }
        }
        
        return false;
    }
    
private:
    bool hasCycleUtil(int node, std::set<int>& visited, 
                      std::set<int>& rec_stack) {
        if (rec_stack.find(node) != rec_stack.end()) {
            return true; // Ciclo detectado
        }
        
        if (visited.find(node) != visited.end()) {
            return false;
        }
        
        visited.insert(node);
        rec_stack.insert(node);
        
        for (int neighbor : wait_for_graph[node]) {
            if (hasCycleUtil(neighbor, visited, rec_stack)) {
                return true;
            }
        }
        
        rec_stack.erase(node);
        return false;
    }
};
```

---

### 2. Timeout em Locks

```cpp
bool acquireWithTimeout(std::timed_mutex& mutex, int timeout_ms) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    
    if (mutex.try_lock_for(timeout)) {
        return true;
    } else {
        // Timeout! Possível deadlock
        std::cerr << "Lock timeout - possible deadlock\n";
        return false;
    }
}
```

---

## 🧪 Testes de Sincronização

### 1. Teste de Race Condition

```cpp
void testRaceCondition() {
    SharedCounter counter;
    std::vector<std::thread> threads;
    
    // Criar 10 threads incrementando 1000 vezes
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 1000; j++) {
                counter.increment();
            }
        });
    }
    
    // Aguardar conclusão
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verificar resultado
    assert(counter.get() == 10000); // Deve ser 10000
}
```

---

### 2. Teste de Deadlock

```cpp
void testDeadlock() {
    std::mutex mutex1, mutex2;
    bool deadlock_occurred = false;
    
    std::thread t1([&]() {
        if (!acquireWithTimeout(mutex1, 1000)) {
            deadlock_occurred = true;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!acquireWithTimeout(mutex2, 1000)) {
            deadlock_occurred = true;
        }
        mutex2.unlock();
        mutex1.unlock();
    });
    
    std::thread t2([&]() {
        if (!acquireWithTimeout(mutex2, 1000)) {
            deadlock_occurred = true;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!acquireWithTimeout(mutex1, 1000)) {
            deadlock_occurred = true;
        }
        mutex1.unlock();
        mutex2.unlock();
    });
    
    t1.join();
    t2.join();
    
    assert(!deadlock_occurred);
}
```

---

## 📊 Métricas de Sincronização

```cpp
struct SyncMetrics {
    std::atomic<int> lock_contentions{0};
    std::atomic<int> total_locks{0};
    std::atomic<long long> total_wait_time{0};
    
    void recordLock(bool contended, long long wait_time_ns) {
        total_locks++;
        if (contended) {
            lock_contentions++;
            total_wait_time += wait_time_ns;
        }
    }
    
    double getContentionRate() {
        return (double)lock_contentions / total_locks * 100.0;
    }
    
    double getAverageWaitTime() {
        if (lock_contentions == 0) return 0.0;
        return (double)total_wait_time / lock_contentions;
    }
};
```

---

## ⚠️ Boas Práticas

### ✅ DO
- Sempre proteger dados compartilhados
- Usar RAII para gerenciar locks
- Manter seções críticas pequenas
- Usar atomic para operações simples
- Documentar ordem de aquisição de locks
- Testar com ThreadSanitizer

### ❌ DON'T
- Não adquirir locks em ordem diferente
- Não manter locks por muito tempo
- Não fazer I/O dentro de seções críticas
- Não usar variáveis globais sem proteção
- Não assumir atomicidade de operações

---

## 🔗 Ferramentas de Debugging

### ThreadSanitizer (TSan)
```bash
g++ -fsanitize=thread -g -o program program.cpp
./program
```

### Helgrind (Valgrind)
```bash
valgrind --tool=helgrind ./program
```

---

## 📚 Referências

- C++ Concurrency in Action (Anthony Williams)
- HERLIHY, M. The Art of Multiprocessor Programming
- WILLIAMS, A. C++ Concurrency in Action. 2nd ed.

---

## 🔗 Próximos Passos

- ➡️ [Métricas de Desempenho](11-metricas.md)
- ➡️ [Estratégia de Testes](12-testes.md)
