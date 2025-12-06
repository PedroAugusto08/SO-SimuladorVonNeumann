# Compilador e flags
CXX := g++
CXXFLAGS := -Wall -Wextra -g -std=c++17 -Isrc
LDFLAGS := -lpthread

# Alvos principais
BIN_DIR := bin
TARGET := $(BIN_DIR)/teste
TARGET_HASH := $(BIN_DIR)/test_hash_register
TARGET_BANK := $(BIN_DIR)/test_register_bank
TARGET_SIM := $(BIN_DIR)/simulador
TARGET_SINGLE_CORE := $(BIN_DIR)/test_single_core_no_threads

# Fontes principais
SRC := src/teste.cpp src/cpu/ULA.cpp
OBJ := $(SRC:.cpp=.o)

# Fontes para teste do hash register
SRC_HASH := src/test_hash_register.cpp
OBJ_HASH := $(SRC_HASH:.cpp=.o)

# Fontes para teste do register bank
SRC_BANK := src/test_register_bank.cpp src/cpu/REGISTER_BANK.cpp
OBJ_BANK := $(SRC_BANK:.cpp=.o)

SRC_SIM := src/main.cpp \
		src/cpu/Core.cpp \
		src/cpu/RoundRobinScheduler.cpp \
		src/cpu/CONTROL_UNIT.cpp \
		src/cpu/pcb_loader.cpp \
		src/cpu/REGISTER_BANK.cpp \
		src/cpu/ULA.cpp \
		src/cpu/FCFSScheduler.cpp \
		src/cpu/SJNScheduler.cpp \
		src/cpu/PriorityScheduler.cpp \
		src/IO/IOManager.cpp \
		src/memory/cache.cpp \
		src/memory/cachePolicy.cpp \
		src/memory/MAIN_MEMORY.cpp \
		src/memory/MemoryManager.cpp \
		src/memory/SECONDARY_MEMORY.cpp \
		src/parser_json/parser_json.cpp \
		src/memory/MemoryMetrics.cpp
OBJ_SIM := $(SRC_SIM:.cpp=.o)

	# Fontes para teste single-core sem threads
	SRC_SINGLE_CORE := test/test_single_core_no_threads.cpp \
			  src/cpu/CONTROL_UNIT.cpp \
			  src/cpu/pcb_loader.cpp \
			  src/cpu/REGISTER_BANK.cpp \
			  src/cpu/ULA.cpp \
			  src/memory/cache.cpp \
			  src/memory/cachePolicy.cpp \
			  src/memory/MAIN_MEMORY.cpp \
			  src/memory/MemoryManager.cpp \
			  src/memory/SECONDARY_MEMORY.cpp \
			  src/parser_json/parser_json.cpp \
			  src/IO/IOManager.cpp
	OBJ_SINGLE_CORE := $(SRC_SINGLE_CORE:.cpp=.o)

# Fontes para teste de métricas (arquivo test/test_metrics.cpp)
# (definido abaixo após BASE_TEST_SRC para garantir expansão correta)

# Fontes base para testes (reutilizáveis)
BASE_TEST_SRC := src/cpu/Core.cpp \
				 src/cpu/RoundRobinScheduler.cpp \
				 src/cpu/CONTROL_UNIT.cpp \
				 src/cpu/pcb_loader.cpp \
				 src/cpu/REGISTER_BANK.cpp \
				 src/cpu/ULA.cpp \
				 src/cpu/FCFSScheduler.cpp \
				 src/cpu/SJNScheduler.cpp \
				 src/cpu/PriorityScheduler.cpp \
				 src/IO/IOManager.cpp \
				 src/memory/cache.cpp \
				 src/memory/cachePolicy.cpp \
				 src/memory/MAIN_MEMORY.cpp \
				 src/memory/MemoryManager.cpp \
				 src/memory/SECONDARY_MEMORY.cpp \
				 src/parser_json/parser_json.cpp

				# Fontes para teste de métricas (arquivo test/test_metrics.cpp)
				SRC_METRICS_PLAIN := test/test_metrics.cpp $(BASE_TEST_SRC)
				OBJ_METRICS_PLAIN := $(SRC_METRICS_PLAIN:.cpp=.o)
				TARGET_METRICS_PLAIN := $(BIN_DIR)/test_metrics

# SRC_PRIORITY_PREEMPT removed (priority preemptive tests removed)

# Make clean -> make -> make run
all: clean $(TARGET_SIM)

# Alvo para compilar e executar
build-and-run: clean $(TARGET) run

# Regra para o simulador multicore
$(TARGET_SIM): $(OBJ_SIM)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_SIM) $(LDFLAGS)
	@echo "✓ Simulador multicore compilado com sucesso!"

# Atalho para compilar o simulador
simulador: $(TARGET_SIM)

# Regra para o programa principal
$(TARGET): $(OBJ)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Regra para o teste do hash register
$(TARGET_HASH): $(OBJ_HASH)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_HASH)

# Regra para o teste do register bank
$(TARGET_BANK): $(OBJ_BANK)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_BANK)

# Regra para teste single-core sem threads
$(TARGET_SINGLE_CORE): $(OBJ_SINGLE_CORE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_SINGLE_CORE) $(LDFLAGS)
	@echo "✓ Teste single-core (sem threads) compilado!"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Limpando arquivos antigos..."
	@rm -f $(OBJ) $(OBJ_HASH) $(OBJ_BANK) $(OBJ_SIM) $(OBJ_METRICS_PLAIN) $(OBJ_SINGLE_CORE)
	@rm -f $(BIN_DIR)/*

run:
	@echo "🚀 Executando o programa..."
	@./$(TARGET)

# Teste específico para hash register
test-hash: clean $(TARGET_HASH)
	@echo "🧪 Executando teste do Hash Register..."
	@./$(TARGET_HASH)

# Teste específico para register bank
test-bank: clean $(TARGET_BANK)
	@echo "🧪 Executando teste do Register Bank..."
	@./$(TARGET_BANK)

# Regra para o teste de métricas (test/test_metrics.cpp)
$(TARGET_METRICS_PLAIN): $(OBJ_METRICS_PLAIN)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_METRICS_PLAIN) $(LDFLAGS)
	@echo "✓ Teste de métricas (test_metrics.cpp) compilado!"

# Teste de métricas (arquivo test_metrics.cpp)
test-metrics: $(TARGET_METRICS_PLAIN)
	@echo "📊 Executando teste de métricas (test_metrics.cpp)..."
	@./$(TARGET_METRICS_PLAIN)

# Teste de prioridade preemptiva removido

# Teste single-core sem threads
test-single-core: $(TARGET_SINGLE_CORE)
	@echo "🧪 Executando teste single-core (sem threads)..."
	@./$(TARGET_SINGLE_CORE)

# Comando de ajuda
help:
	@echo "📋 SO-SimuladorVonNeumann - Comandos Disponíveis:"
	@echo ""
	@echo "  make simulador     - 🎯 Compila simulador multicore Round-Robin"
	@echo "  make run-sim       - 🚀 Executa simulador multicore"
	@echo "  make / make all    - Compila e executa programa principal"
	@echo "  make clean         - Remove arquivos gerados (.o, executáveis)"
	@echo "  make run          - Executa programa principal (sem recompilar)"
	@echo "  make teste        - Compila apenas o programa principal"
	@echo "  make test-hash    - Compila e testa sistema de registradores"
	@echo "  make test-bank    - Compila e testa o banco de registradores"
	@echo "  make test-metrics - Compila e executa métricas não-interativas"
	@echo "  make test-single-core - Executa modo single-core sem threads"
	@echo "  make check        - Verificação rápida de todos os componentes"
	@echo "  make debug        - Build com símbolos de debug (-g -O0)"
	@echo "  make help         - Mostra esta mensagem de ajuda"
	@echo ""
	@echo "📊 Informações do Projeto:"
	@echo "  Compilador: $(CXX)"
	@echo "  Flags: $(CXXFLAGS)"
	@echo "  Arquivos fonte: $(words $(SRC) $(SRC_HASH) $(SRC_SIM)) arquivos"

# Executar simulador multicore
run-sim: $(TARGET_SIM)
	@echo "🚀 Executando simulador multicore Round-Robin..."
	@./$(TARGET_SIM)

# Verificação rápida de todos os componentes
check: $(TARGET) $(TARGET_HASH) $(TARGET_BANK)
	@echo "✅ Executando verificações rápidas..."
	@echo -n "  Teste principal: "; ./$(TARGET) >/dev/null 2>&1 && echo "✅ PASSOU" || echo "❌ FALHOU"
	@echo -n "  Teste hash register: "; ./$(TARGET_HASH) >/dev/null 2>&1 && echo "✅ PASSOU" || echo "❌ FALHOU"
	@echo -n "  Teste register bank: "; ./$(TARGET_BANK) >/dev/null 2>&1 && echo "✅ PASSOU" || echo "❌ FALHOU"
	@echo "🎯 Verificação concluída!"

# Build com debug symbols
debug: CXXFLAGS += -DDEBUG -O0 -ggdb3
debug: clean $(TARGET)
	@echo "🐛 Build de debug criado com símbolos completos"
	@echo "   Use: gdb ./$(TARGET) para debug"

# Lista arquivos do projeto
list-files:
	@echo "📁 Arquivos do projeto:"
	@echo "  Fontes principais: $(SRC)"
	@echo "  Fontes de teste: $(SRC_HASH)"
	@echo "  Headers: $(shell find src -name '*.hpp' 2>/dev/null)"

.PHONY: all clean run test-hash help check debug list-files
