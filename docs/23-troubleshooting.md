# Troubleshooting

## 🎯 Objetivo

Guia para resolução dos problemas mais comuns durante o desenvolvimento e execução do simulador.

---

## 🐛 Problemas de Compilação

### Erro: command not found (g++, make, etc.)

**Sintoma:**
```bash
bash: g++: command not found
```

**Causa:** Compilador não instalado

**Solução:**
```bash
# Ubuntu/Debian
sudo apt install build-essential

# macOS
xcode-select --install
brew install gcc

# Verificar instalação
g++ --version
```

---

### Erro: C++17 features not available

**Sintoma:**
```
error: 'optional' is not a member of 'std'
```

**Causa:** Compilador muito antigo ou flag incorreta

**Solução:**
```bash
# Verificar versão (precisa ser 11+)
g++ --version

# Compilar com flag correta
g++ -std=c++17 main.cpp -o simulador

# Atualizar compilador se necessário
sudo apt install g++-11
```

---

### Erro: undefined reference to pthread

**Sintoma:**
```
undefined reference to `pthread_create'
```

**Causa:** Falta flag `-pthread`

**Solução:**
```bash
g++ -std=c++17 -pthread main.cpp -o simulador

# No Makefile
CXXFLAGS = -std=c++17 -pthread
```

---

### Erro: Google Test not found

**Sintoma:**
```
fatal error: gtest/gtest.h: No such file or directory
```

**Causa:** Google Test não instalado

**Solução:**
```bash
# Ubuntu/Debian
sudo apt install libgtest-dev

# Compilar biblioteca
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib

# macOS
brew install googletest
```

---

### Erro: JSON library not found

**Sintoma:**
```
fatal error: nlohmann/json.hpp: No such file or directory
```

**Causa:** nlohmann-json não instalado

**Solução:**
```bash
# Ubuntu/Debian
sudo apt install nlohmann-json3-dev

# macOS
brew install nlohmann-json

# Ou copiar header-only
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
mkdir -p include/nlohmann
mv json.hpp include/nlohmann/
```

---

## 💥 Problemas de Execução

### Segmentation Fault

**Sintoma:**
```
Segmentation fault (core dumped)
```

**Causas Comuns:**
1. Acesso a ponteiro nulo
2. Array out of bounds
3. Double free
4. Stack overflow

**Debugging:**
```bash
# Executar com GDB
gdb ./simulador
(gdb) run
# Quando crashar:
(gdb) backtrace
(gdb) frame 0
(gdb) print variable_name

# Ou usar AddressSanitizer
g++ -fsanitize=address -g main.cpp -o simulador
./simulador
```

**Soluções:**
```cpp
// Sempre verificar ponteiros
if (ptr != nullptr) {
    ptr->method();
}

// Usar .at() ao invés de []
vector.at(index);  // Lança exceção se inválido

// Usar smart pointers
std::unique_ptr<Core> core = std::make_unique<Core>(0);
```

---

### Sistema Trava (Deadlock)

**Sintoma:**
- Programa não progride
- CPU usage baixo
- Não responde a Ctrl+C

**Debugging:**
```bash
# Obter PID
ps aux | grep simulador

# Ver stack de threads
gdb -p <PID>
(gdb) thread apply all bt

# Ou usar timeout
timeout 30s ./simulador || echo "Timeout! Possível deadlock"
```

**Soluções:**
```cpp
// 1. Sempre adquirir locks na mesma ordem
std::scoped_lock lock(mutex_A, mutex_B);  // C++17

// 2. Usar timeout
std::timed_mutex mutex;
if (mutex.try_lock_for(std::chrono::seconds(5))) {
    // Trabalho
    mutex.unlock();
} else {
    std::cerr << "Timeout - possível deadlock\n";
}

// 3. Evitar locks aninhados
```

---

### Race Conditions / Resultados Inconsistentes

**Sintoma:**
- Resultados diferentes a cada execução
- Crashes aleatórios
- Valores incorretos em variáveis compartilhadas

**Debugging:**
```bash
# ThreadSanitizer
g++ -fsanitize=thread -g -O1 main.cpp -o simulador
./simulador

# Helgrind
valgrind --tool=helgrind ./simulador
```

**Soluções:**
```cpp
// Proteger TODAS as variáveis compartilhadas
class SharedQueue {
private:
    std::queue<int> queue;
    std::mutex mutex;  // Sempre usar mutex!
    
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(value);
    }
};

// Usar atomic para contadores simples
std::atomic<int> counter{0};
counter++;  // Thread-safe
```

---

### Memory Leaks

**Sintoma:**
- Memória aumenta continuamente
- Sistema fica lento

**Debugging:**
```bash
# Valgrind
valgrind --leak-check=full ./simulador

# Com símbolos para melhor output
g++ -g main.cpp -o simulador
valgrind --leak-check=full --show-leak-kinds=all ./simulador
```

**Soluções:**
```cpp
// Usar smart pointers
std::unique_ptr<Core> core = std::make_unique<Core>(0);
std::shared_ptr<PCB> process = std::make_shared<PCB>();

// Ou containers STL
std::vector<int> data(1000);  // Automático

// NUNCA:
// int* data = new int[1000];
// ... sem delete[]
```

---

## 📊 Problemas de Performance

### Speedup Muito Baixo

**Sintoma:**
- Com 4 cores, speedup < 2x
- Sistema não escala

**Causas:**
1. Contenção excessiva em locks
2. Seções críticas muito grandes
3. Processos muito curtos

**Debugging:**
```bash
# Profiling
g++ -pg main.cpp -o simulador
./simulador
gprof simulador gmon.out > analysis.txt

# Ver onde tempo é gasto
cat analysis.txt | less
```

**Soluções:**
```cpp
// 1. Reduzir seções críticas
{
    std::lock_guard<std::mutex> lock(mutex);
    // Mínimo de código aqui
    data = calculate_outside_lock();
    shared_queue.push(data);
}  // Lock liberado o mais cedo possível

// 2. Usar locks mais granulares
// Ao invés de um mutex global, usar mutex por componente

// 3. Considerar lock-free data structures
std::atomic<int> counter{0};
```

---

### Alta Contenção em Locks

**Sintoma:**
- Profiler mostra muito tempo em `mutex.lock()`
- Threads passam muito tempo bloqueadas

**Soluções:**
```cpp
// 1. Reduzir frequência de locks
// Processar em lote ao invés de item por item

// 2. Reader-writer lock
std::shared_mutex rw_mutex;

// Múltiplos leitores simultâneos
void read() {
    std::shared_lock lock(rw_mutex);
    // Leitura
}

// Escritor exclusivo
void write() {
    std::unique_lock lock(rw_mutex);
    // Escrita
}

// 3. Lock-free quando possível
std::atomic<int> value{0};
value.fetch_add(1);  // Sem lock
```

---

## 🧪 Problemas de Testes

### Testes Falhando

**Sintoma:**
```
Expected: 10
  Actual: 8
```

**Debugging:**
```cpp
// Adicionar output de debug
TEST(SchedulerTest, ProcessCount) {
    Scheduler sched;
    sched.addProcess(p1);
    sched.addProcess(p2);
    
    int count = sched.getProcessCount();
    std::cout << "Count: " << count << std::endl;  // Debug
    
    EXPECT_EQ(count, 2);
}
```

---

### Testes Intermitentes

**Sintoma:**
- Testes passam às vezes, falham outras

**Causa:** Provável race condition

**Solução:**
```cpp
// Adicionar sincronização adequada
// Ou usar ThreadSanitizer
g++ -fsanitize=thread test.cpp -o test
./test
```

---

## 🔧 Problemas de Ambiente

### WSL muito lento

**Causas:**
1. Projeto em /mnt/c/ (filesystem Windows)
2. Antivírus escaneando WSL
3. WSL 1 ao invés de WSL 2

**Soluções:**
```bash
# 1. Mover projeto para filesystem nativo
mv /mnt/c/projeto ~/projeto

# 2. Verificar versão WSL
wsl -l -v

# 3. Converter para WSL 2
wsl --set-version Ubuntu-22.04 2

# 4. Configurar exclusões do antivírus
# Adicionar pasta WSL às exclusões
```

---

### Permissões negadas

**Sintoma:**
```
Permission denied
```

**Soluções:**
```bash
# Tornar executável
chmod +x script.sh
chmod +x simulador

# Problemas com sudo
sudo chown $USER:$USER arquivo
```

---

## 📝 Problemas Comuns de Lógica

### Processos não executam

**Verificações:**
```cpp
// 1. Processos estão na fila?
std::cout << "Queue size: " << scheduler->getQueueSize() << std::endl;

// 2. Cores estão executando?
for (auto& core : cores) {
    std::cout << "Core " << core->getId() << " busy: " 
              << core->isBusy() << std::endl;
}

// 3. Condição de parada correta?
while (hasRunningProcesses()) {  // Não travar aqui
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

---

### Métricas incorretas

**Verificações:**
```cpp
// Timestamps corretos?
process->start_time = getElapsedTime();  // Não esquecer de setar

// Calcular corretamente
process->turnaround_time = completion_time - arrival_time;
process->waiting_time = turnaround_time - burst_time;

// Testar com caso simples primeiro
// 1 processo, 1 core, valores conhecidos
```

---

## 🆘 Quando Pedir Ajuda

Antes de pedir ajuda, colete:

```bash
# 1. Versão do compilador
g++ --version

# 2. Sistema operacional
uname -a

# 3. Comando de compilação exato
# (copiar do terminal)

# 4. Mensagem de erro completa
# (copiar texto, não screenshot)

# 5. Código mínimo que reproduz o problema
# (criar teste isolado)
```

---

## 📚 Recursos Adicionais

### Documentação
- [FAQ](20-faq.md)
- [Comandos Úteis](22-comandos.md)
- [Instalação do Ambiente](21-ambiente.md)

### Ferramentas de Debug
- GDB Tutorial: https://www.gdbtutorial.com/
- Valgrind Manual: https://valgrind.org/docs/manual/
- ThreadSanitizer: https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual

---

## 💡 Dicas Gerais

### Para Evitar Problemas

✅ **DO:**
- Compilar com `-Wall -Wextra`
- Testar frequentemente
- Usar sanitizers em desenvolvimento
- Commitar código funcional
- Documentar decisões não-óbvias

❌ **DON'T:**
- Ignorar warnings
- Testar só no final
- Usar `sudo` desnecessariamente
- Copiar código sem entender

---

### Metodologia de Debug

1. **Reproduzir** o problema consistentemente
2. **Isolar** o componente problemático
3. **Simplificar** para caso mínimo
4. **Instrumentar** com logs/debug
5. **Corrigir** e validar
6. **Adicionar** teste de regressão

---

**Lembre-se:** Debugging é parte normal do desenvolvimento. Seja paciente e sistemático!

**Última atualização:** Novembro 2025
