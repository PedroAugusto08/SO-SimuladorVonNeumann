# Troubleshooting

## Problemas de Compilação

### Erro: 'thread' is not a member of 'std'

**Causa:** Compilador não suporta C++11/17 ou falta flag.

**Solução:**
```bash
# Verificar versão do g++
g++ --version

# Deve ser 9.0+. Se não:
sudo apt install g++-9
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90

# Ou adicionar flag manualmente:
g++ -std=c++17 -pthread ...
```

### Erro: undefined reference to `pthread_create`

**Causa:** Falta linkagem com pthread.

**Solução:**
```bash
# Adicionar -pthread na compilação
g++ -std=c++17 -pthread main.cpp -o simulador
```

### Erro: nlohmann/json.hpp not found

**Causa:** Biblioteca JSON não está no include path.

**Solução:**
```bash
# Verificar se existe
ls src/nlohmann/json.hpp

# Se não existir, baixar:
mkdir -p src/nlohmann
curl -o src/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
```

### Erro: multiple definition of...

**Causa:** Header incluído múltiplas vezes sem include guard.

**Solução:**
Verificar se todos os headers têm:
```cpp
#ifndef HEADER_NAME_HPP
#define HEADER_NAME_HPP
// conteúdo
#endif
```

## Problemas de Execução

### Segmentation Fault

**Possíveis causas:**

1. **Acesso a ponteiro nulo:**
   ```bash
   # Debug com GDB
   gdb ./simulador
   (gdb) run --policy FCFS --cores 2 -p tasks.json proc.json
   (gdb) bt  # backtrace após crash
   ```

2. **Acesso fora dos limites:**
   ```bash
   # Compilar com AddressSanitizer
   g++ -std=c++17 -fsanitize=address -g main.cpp -o simulador
   ./simulador ...
   ```

3. **Stack overflow (recursão infinita):**
   ```bash
   # Aumentar limite de stack
   ulimit -s unlimited
   ```

### Deadlock (Programa trava)

**Sintomas:** Programa não termina, CPU em 0%.

**Diagnóstico:**
```bash
# Ver threads do processo
ps -eLf | grep simulador

# Anexar debugger
gdb -p <PID>
(gdb) info threads
(gdb) thread apply all bt
```

**Causas comuns:**
- Dois threads esperando locks um do outro
- Condição de término nunca satisfeita

### Race Condition

**Sintomas:** Resultados inconsistentes entre execuções.

**Diagnóstico:**
```bash
# Compilar com ThreadSanitizer
g++ -std=c++17 -fsanitize=thread -g main.cpp -o simulador
./simulador ...
```

**Soluções:**
- Adicionar mutex em seções críticas
- Usar `std::atomic` para contadores
- Verificar ordem de aquisição de locks

### Memória Insuficiente

**Sintomas:** `std::bad_alloc` ou sistema lento.

**Diagnóstico:**
```bash
# Verificar uso de memória
valgrind --tool=massif ./simulador ...
ms_print massif.out.*
```

**Soluções:**
- Reduzir número de processos
- Usar cache menor
- Verificar memory leaks com Valgrind

## Problemas de Métricas

### Métricas zeradas

**Causa:** Métricas não estão sendo coletadas.

**Verificar:**
```cpp
// Em PCB.hpp
struct PCB {
    uint64_t start_time = 0;  // Inicializado?
    uint64_t end_time = 0;
    // ...
};
```

### Throughput negativo ou muito alto

**Causa:** Overflow ou divisão por zero.

**Verificar:**
```cpp
double throughput = (cycles > 0) ? 
    (double)instructions / cycles : 0.0;
```

### CSV malformado

**Causa:** Caracteres especiais nos dados.

**Solução:**
```cpp
// Escapar valores
std::string escape_csv(const std::string& s) {
    if (s.find(',') != std::string::npos) {
        return "\"" + s + "\"";
    }
    return s;
}
```

## Problemas Específicos

### Cache Hit Rate muito baixo (<10%)

**Possíveis causas:**
1. Cache muito pequena para o working set
2. Acessos aleatórios (sem localidade)
3. Thrashing entre núcleos

**Soluções:**
- Aumentar tamanho da cache
- Verificar padrão de acesso
- Usar política LRU

### Round Robin com muitos context switches

**Causa:** Quantum muito pequeno.

**Solução:**
```bash
# Aumentar quantum
./simulador --policy RR --quantum 2000 ...
```

### SJN com starvation

**Sintoma:** Processos longos nunca executam.

**Solução:** Implementar aging ou usar limite de espera.

## Ferramentas de Debug

### GDB

```bash
# Compilar com debug symbols
g++ -g -std=c++17 main.cpp -o simulador

# Iniciar debugger
gdb ./simulador

# Comandos úteis
(gdb) break main.cpp:100   # Breakpoint
(gdb) run --policy FCFS    # Executar
(gdb) print variable       # Inspecionar variável
(gdb) next                 # Próxima linha
(gdb) step                 # Entrar em função
(gdb) continue             # Continuar
(gdb) bt                   # Backtrace
```

### Valgrind

```bash
# Memory leaks
valgrind --leak-check=full ./simulador ...

# Acessos inválidos
valgrind --tool=memcheck ./simulador ...

# Profiling
valgrind --tool=callgrind ./simulador ...
kcachegrind callgrind.out.*
```

### Sanitizers

```bash
# AddressSanitizer (buffer overflow, use-after-free)
g++ -fsanitize=address -g main.cpp -o simulador

# ThreadSanitizer (race conditions)
g++ -fsanitize=thread -g main.cpp -o simulador

# UndefinedBehaviorSanitizer
g++ -fsanitize=undefined -g main.cpp -o simulador
```

## Logs para Suporte

Ao reportar problemas, incluir:

```bash
# Informações do sistema
uname -a
g++ --version

# Comando executado
echo "./simulador --policy FCFS --cores 2 ..."

# Saída de erro
./simulador ... 2>&1 | tee error.log

# Core dump (se houver)
ulimit -c unlimited
./simulador ...
gdb ./simulador core -ex bt -ex quit
```
