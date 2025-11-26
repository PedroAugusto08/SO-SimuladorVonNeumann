# Código de Referência

## 🎯 Objetivo

Fornecer exemplos de código de referência e snippets úteis para o desenvolvimento do simulador multicore.

---

## 🏗️ Estrutura de Diretórios Recomendada

```
SO-SimuladorVonNeumann/
├── src/
│   ├── cpu/
│   │   ├── Core.hpp
│   │   ├── Core.cpp
│   │   ├── MultiCore.hpp
│   │   ├── MultiCore.cpp
│   │   ├── RoundRobinScheduler.hpp
│   │   └── RoundRobinScheduler.cpp
│   ├── memory/
│   │   ├── SegmentationManager.hpp
│   │   ├── SegmentationManager.cpp
│   │   ├── FIFOPolicy.hpp
│   │   └── LRUPolicy.hpp
│   ├── metrics/
│   │   ├── MetricsCollector.hpp
│   │   └── MetricsCollector.cpp
│   └── main.cpp
├── test/
│   ├── test_core.cpp
│   ├── test_scheduler.cpp
│   └── test_memory.cpp
├── include/
│   └── common.hpp
├── docs/
├── Makefile
├── CMakeLists.txt
└── README.md
```

---

## 💻 Exemplos de Código

### 1. Main.cpp Básico

```cpp
// main.cpp
#include "MultiCore.hpp"
#include "MetricsCollector.hpp"
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    // Configuração
    int num_cores = 4;
    int quantum = 20;
    
    if (argc >= 2) {
        num_cores = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        quantum = std::stoi(argv[2]);
    }
    
    std::cout << "=== Simulador Multicore ===" << std::endl;
    std::cout << "Cores: " << num_cores << std::endl;
    std::cout << "Quantum: " << quantum << std::endl;
    std::cout << "============================\n" << std::endl;
    
    // Criar sistema
    MultiCore system(num_cores, quantum);
    system.initialize();
    
    // Carregar processos
    std::vector<std::shared_ptr<PCB>> processes = 
        loadProcessesFromJSON("processes.json");
    
    std::cout << "Carregados " << processes.size() 
              << " processos" << std::endl;
    
    for (auto& process : processes) {
        system.addProcess(process);
    }
    
    // Executar simulação
    std::cout << "\nIniciando simulação..." << std::endl;
    system.start();
    
    // Aguardar conclusão
    while (system.hasRunningProcesses()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    system.stop();
    std::cout << "\nSimulação concluída!" << std::endl;
    
    // Exibir estatísticas
    system.printStatistics();
    
    // Exportar métricas
    system.exportMetrics("results.csv");
    
    return 0;
}
```

---

### 2. Makefile Completo

```makefile
# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -pthread
DEBUG_FLAGS = -g -DDEBUG
SANITIZE_FLAGS = -fsanitize=thread -fsanitize=address

# Diretórios
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = test
INCLUDE_DIR = include

# Arquivos fonte
SOURCES = $(wildcard $(SRC_DIR)/**/*.cpp) $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Testes
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/test_%.o,$(TEST_SOURCES))

# Executáveis
TARGET = simulador
TEST_TARGET = test_simulador

# Regra padrão
all: $(TARGET)

# Compilar simulador
$(TARGET): $(OBJECTS)
	@echo "Linkando $(TARGET)..."
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build concluído: $(TARGET)"

# Compilar objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compilando $<..."
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)
	@echo "Debug build concluído"

# Sanitizer build
sanitize: CXXFLAGS += $(SANITIZE_FLAGS)
sanitize: clean $(TARGET)
	@echo "Sanitizer build concluído"

# Testes
test: $(TEST_TARGET)
	@echo "Executando testes..."
	@./$(TEST_TARGET)

$(TEST_TARGET): $(filter-out $(BUILD_DIR)/main.o,$(OBJECTS)) $(TEST_OBJECTS)
	@echo "Linkando testes..."
	@$(CXX) $(CXXFLAGS) $^ -lgtest -lgtest_main -pthread -o $@

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compilando teste $<..."
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(SRC_DIR) -c $< -o $@

# Executar
run: $(TARGET)
	@./$(TARGET)

run-test: $(TARGET)
	@./$(TARGET) 4 20

# Valgrind
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Limpeza
clean:
	@echo "Limpando..."
	@rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)
	@echo "Limpeza concluída"

# Phony targets
.PHONY: all debug sanitize test run run-test valgrind clean

# Help
help:
	@echo "Alvos disponíveis:"
	@echo "  all       - Compilar simulador (padrão)"
	@echo "  debug     - Compilar com símbolos de debug"
	@echo "  sanitize  - Compilar com sanitizers"
	@echo "  test      - Compilar e executar testes"
	@echo "  run       - Executar simulador"
	@echo "  run-test  - Executar com parâmetros de teste"
	@echo "  valgrind  - Executar com valgrind"
	@echo "  clean     - Remover arquivos compilados"
```

---

### 3. CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(SimuladorMulticore VERSION 1.0)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags de compilação
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Incluir diretórios
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Fontes
file(GLOB_RECURSE SOURCES "src/**/*.cpp" "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# Executável principal
add_executable(simulador src/main.cpp ${SOURCES})

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Testes
file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(test_simulador ${TEST_SOURCES} ${SOURCES})
target_link_libraries(test_simulador gtest_main pthread)

include(GoogleTest)
gtest_discover_tests(test_simulador)

# Instalação
install(TARGETS simulador DESTINATION bin)
```

---

### 4. JSON de Processos

```json
{
  "processes": [
    {
      "pid": 1,
      "name": "Process1",
      "arrival_time": 0,
      "burst_time": 100,
      "memory": {
        "code": 4096,
        "data": 8192,
        "stack": 4096
      }
    },
    {
      "pid": 2,
      "name": "Process2",
      "arrival_time": 10,
      "burst_time": 150,
      "memory": {
        "code": 4096,
        "data": 16384,
        "stack": 4096
      }
    },
    {
      "pid": 3,
      "name": "Process3",
      "arrival_time": 20,
      "burst_time": 200,
      "memory": {
        "code": 8192,
        "data": 8192,
        "stack": 4096
      }
    }
  ],
  "config": {
    "num_cores": 4,
    "quantum": 20,
    "memory_size": 10485760,
    "replacement_policy": "LRU"
  }
}
```

---

### 5. Script de Análise Python

```python
#!/usr/bin/env python3
# analysis.py

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys

def load_results(csv_file):
    """Carrega resultados do CSV"""
    return pd.read_csv(csv_file)

def calculate_metrics(df):
    """Calcula métricas agregadas"""
    metrics = {
        'total_processes': len(df),
        'avg_turnaround': df['TurnaroundTime'].mean(),
        'avg_waiting': df['WaitingTime'].mean(),
        'avg_response': df['ResponseTime'].mean(),
        'max_completion': df['CompletionTime'].max(),
    }
    metrics['throughput'] = metrics['total_processes'] / (metrics['max_completion'] / 1000.0)
    return metrics

def plot_turnaround(df, output='turnaround.png'):
    """Gráfico de turnaround time"""
    plt.figure(figsize=(10, 6))
    plt.bar(df['PID'], df['TurnaroundTime'])
    plt.xlabel('Process ID')
    plt.ylabel('Turnaround Time (ms)')
    plt.title('Turnaround Time per Process')
    plt.tight_layout()
    plt.savefig(output, dpi=300)
    print(f"Salvo: {output}")

def plot_core_distribution(df, output='cores.png'):
    """Gráfico de distribuição por core"""
    plt.figure(figsize=(8, 6))
    core_counts = df['AssignedCore'].value_counts().sort_index()
    plt.bar(core_counts.index, core_counts.values)
    plt.xlabel('Core ID')
    plt.ylabel('Number of Processes')
    plt.title('Process Distribution Across Cores')
    plt.tight_layout()
    plt.savefig(output, dpi=300)
    print(f"Salvo: {output}")

def generate_report(metrics, output='report.txt'):
    """Gera relatório textual"""
    report = f"""
===================================
   RELATÓRIO DE DESEMPENHO
===================================

Total de Processos: {metrics['total_processes']}

TEMPOS MÉDIOS:
  Turnaround Time: {metrics['avg_turnaround']:.2f} ms
  Waiting Time: {metrics['avg_waiting']:.2f} ms
  Response Time: {metrics['avg_response']:.2f} ms

THROUGHPUT: {metrics['throughput']:.2f} processos/segundo

===================================
"""
    
    with open(output, 'w') as f:
        f.write(report)
    
    print(report)
    print(f"Relatório salvo: {output}")

def main():
    if len(sys.argv) < 2:
        print("Uso: python analysis.py <results.csv>")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    # Carregar dados
    df = load_results(csv_file)
    
    # Calcular métricas
    metrics = calculate_metrics(df)
    
    # Gerar visualizações
    plot_turnaround(df)
    plot_core_distribution(df)
    
    # Gerar relatório
    generate_report(metrics)

if __name__ == '__main__':
    main()
```

---

### 6. Script de Build Automatizado

```bash
#!/bin/bash
# build.sh - Script de build automatizado

set -e  # Exit on error

echo "=== Build Automatizado do Simulador ==="

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Função para print colorido
print_status() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Limpar builds anteriores
echo "Limpando builds anteriores..."
make clean 2>/dev/null || true
print_status "Limpeza concluída"

# Compilar
echo "Compilando simulador..."
if make -j$(nproc); then
    print_status "Compilação concluída"
else
    print_error "Falha na compilação"
    exit 1
fi

# Executar testes
echo "Executando testes..."
if make test; then
    print_status "Testes passaram"
else
    print_warning "Alguns testes falharam"
fi

# Verificar memory leaks
echo "Verificando memory leaks..."
if command -v valgrind &> /dev/null; then
    valgrind --leak-check=summary --error-exitcode=1 ./simulador 2 10 > /dev/null
    if [ $? -eq 0 ]; then
        print_status "Sem memory leaks detectados"
    else
        print_warning "Memory leaks detectados"
    fi
else
    print_warning "Valgrind não instalado, pulando verificação"
fi

echo ""
echo "=== Build concluído com sucesso! ==="
echo "Execute './simulador' para iniciar"
```

---

## 📚 Recursos Adicionais

### .gitignore

```gitignore
# Compiled Object files
*.o
*.obj

# Executáveis
simulador
test_simulador
*.exe
*.out

# Build directories
build/
bin/

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# Logs e resultados
*.log
*.csv
*.txt
results/
logs/

# Dependencies
*.d

# Coverage
*.gcov
*.gcda
*.gcno
coverage/
```

---

### README.md Template

```markdown
# Simulador Multicore Round Robin

Simulador educacional de arquitetura multicore com escalonador Round Robin e gerenciamento de memória segmentada.

## 🚀 Compilação

### Requisitos
- GCC 11+ ou Clang 13+
- C++17
- CMake 3.14+ (opcional)
- Google Test (para testes)

### Build
```bash
# Usando Make
make

# Ou usando CMake
mkdir build && cd build
cmake ..
make
```

## 🏃 Execução

```bash
# Executar com parâmetros padrão
./simulador

# Especificar cores e quantum
./simulador <num_cores> <quantum>

# Exemplo: 4 cores, quantum 20
./simulador 4 20
```

## 🧪 Testes

```bash
make test
```

## 📊 Análise de Resultados

```bash
python3 analysis.py results.csv
```

## 📖 Documentação

Ver pasta `docs/` para documentação completa.

## 👥 Autores

- Autor 1
- Autor 2
- Autor 3
- Autor 4

## 📄 Licença

MIT License
```

---

## 🔗 Próximos Passos

- ➡️ [FAQ](20-faq.md)
- ➡️ [Instalação do Ambiente](21-ambiente.md)
- ➡️ [Comandos Úteis](22-comandos.md)
