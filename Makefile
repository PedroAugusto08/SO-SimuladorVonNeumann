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
TARGET_MULTICORE := $(BIN_DIR)/test_multicore
TARGET_THROUGHPUT := $(BIN_DIR)/test_multicore_throughput
TARGET_COMPARATIVE := $(BIN_DIR)/test_multicore_comparative
TARGET_PREEMPT := $(BIN_DIR)/test_preemption
TARGET_METRICS := $(BIN_DIR)/test_metrics_complete
TARGET_CPU_METRICS := $(BIN_DIR)/test_cpu_metrics
TARGET_DEEP_INSPECT := $(BIN_DIR)/test_deep_inspection
TARGET_RACE_DEBUG := $(BIN_DIR)/test_race_debug
TARGET_VERIFY_EXEC := $(BIN_DIR)/test_verify_execution
TARGET_SINGLE_CORE := $(BIN_DIR)/test_single_core_no_threads
TARGET_UNIFIED := $(BIN_DIR)/test_complete_unified

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

# Fontes para teste de escalabilidade multicore
SRC_MULTICORE := test/test_multicore.cpp \
                 src/cpu/Core.cpp \
                 src/cpu/RoundRobinScheduler.cpp \
                 src/cpu/CONTROL_UNIT.cpp \
                 src/cpu/pcb_loader.cpp \
                 src/cpu/REGISTER_BANK.cpp \
                 src/cpu/ULA.cpp \
                 src/cpu/FCFSScheduler.cpp \
                 src/IO/IOManager.cpp \
                 src/memory/cache.cpp \
                 src/memory/cachePolicy.cpp \
                 src/memory/MAIN_MEMORY.cpp \
                 src/memory/MemoryManager.cpp \
                 src/memory/SECONDARY_MEMORY.cpp \
                 src/parser_json/parser_json.cpp
OBJ_MULTICORE := $(SRC_MULTICORE:.cpp=.o)

# Fontes para teste de throughput multicore (confiável)
SRC_THROUGHPUT := test/test_multicore_throughput.cpp \
                  src/cpu/Core.cpp \
                  src/cpu/RoundRobinScheduler.cpp \
                  src/cpu/CONTROL_UNIT.cpp \
                  src/cpu/pcb_loader.cpp \
                  src/cpu/REGISTER_BANK.cpp \
                  src/cpu/ULA.cpp \
                  src/cpu/FCFSScheduler.cpp \
                  src/IO/IOManager.cpp \
                  src/memory/cache.cpp \
                  src/memory/cachePolicy.cpp \
                  src/memory/MAIN_MEMORY.cpp \
                  src/memory/MemoryManager.cpp \
                  src/memory/SECONDARY_MEMORY.cpp \
                  src/parser_json/parser_json.cpp
OBJ_THROUGHPUT := $(SRC_THROUGHPUT:.cpp=.o)

# Fontes para teste comparativo de políticas multicore
SRC_COMPARATIVE := test/test_multicore_comparative.cpp \
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
				   src/parser_json/parser_json.cpp
OBJ_COMPARATIVE := $(SRC_COMPARATIVE:.cpp=.o)

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

# Fontes para teste de preempção
SRC_PREEMPT := test/test_preemption.cpp \
               src/cpu/Core.cpp \
               src/cpu/RoundRobinScheduler.cpp \
               src/cpu/CONTROL_UNIT.cpp \
               src/cpu/pcb_loader.cpp \
               src/cpu/REGISTER_BANK.cpp \
               src/cpu/ULA.cpp \
               src/cpu/FCFSScheduler.cpp \
               src/IO/IOManager.cpp \
               src/memory/cache.cpp \
               src/memory/cachePolicy.cpp \
               src/memory/MAIN_MEMORY.cpp \
               src/memory/MemoryManager.cpp \
               src/memory/SECONDARY_MEMORY.cpp \
               src/parser_json/parser_json.cpp
OBJ_PREEMPT := $(SRC_PREEMPT:.cpp=.o)

# Fontes para teste de métricas completas
SRC_METRICS := test/test_metrics_complete.cpp \
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
			   src/parser_json/parser_json.cpp
OBJ_METRICS := $(SRC_METRICS:.cpp=.o)

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

SRC_CPU_METRICS := test/test_cpu_metrics.cpp $(BASE_TEST_SRC)
OBJ_CPU_METRICS := $(SRC_CPU_METRICS:.cpp=.o)

# SRC_PRIORITY_PREEMPT removed (priority preemptive tests removed)

SRC_DEEP_INSPECT := test/test_deep_inspection.cpp $(BASE_TEST_SRC)
OBJ_DEEP_INSPECT := $(SRC_DEEP_INSPECT:.cpp=.o)

SRC_RACE_DEBUG := test/test_race_debug.cpp $(BASE_TEST_SRC)
OBJ_RACE_DEBUG := $(SRC_RACE_DEBUG:.cpp=.o)

SRC_VERIFY_EXEC := test/test_verify_execution.cpp $(BASE_TEST_SRC)
OBJ_VERIFY_EXEC := $(SRC_VERIFY_EXEC:.cpp=.o)

# Fontes para teste unificado completo (multicore + métricas + cache)
SRC_UNIFIED := test/test_complete_unified.cpp $(BASE_TEST_SRC)
OBJ_UNIFIED := $(SRC_UNIFIED:.cpp=.o)

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

# Regra para o teste de escalabilidade multicore
$(TARGET_MULTICORE): $(OBJ_MULTICORE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_MULTICORE) $(LDFLAGS)
	@echo "✓ Teste de escalabilidade multicore compilado!"

# Regra para o teste de throughput (medição confiável)
$(TARGET_THROUGHPUT): $(OBJ_THROUGHPUT)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_THROUGHPUT) $(LDFLAGS)
	@echo "✓ Teste de throughput compilado!"

# Regra para o teste comparativo de políticas
$(TARGET_COMPARATIVE): $(OBJ_COMPARATIVE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_COMPARATIVE) $(LDFLAGS)
	@echo "✓ Teste comparativo de políticas compilado!"

# Regra para o teste de preempção
$(TARGET_PREEMPT): $(OBJ_PREEMPT)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_PREEMPT) $(LDFLAGS)
	@echo "✓ Teste de preempção compilado!"

# Regra para o teste de métricas completas
$(TARGET_METRICS): $(OBJ_METRICS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_METRICS) $(LDFLAGS)
	@echo "✓ Teste de métricas completas compilado!"

# Regra para teste de métricas de CPU
$(TARGET_CPU_METRICS): $(OBJ_CPU_METRICS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_CPU_METRICS) $(LDFLAGS)
	@echo "✓ Teste de métricas de CPU compilado!"

# Teste de prioridade preemptiva removido

# Regra para teste de inspeção profunda
$(TARGET_DEEP_INSPECT): $(OBJ_DEEP_INSPECT)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_DEEP_INSPECT) $(LDFLAGS)
	@echo "✓ Teste de inspeção profunda compilado!"

# Regra para teste de debug de race conditions
$(TARGET_RACE_DEBUG): $(OBJ_RACE_DEBUG)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_RACE_DEBUG) $(LDFLAGS)
	@echo "✓ Teste de race debug compilado!"

# Regra para teste de verificação de execução
$(TARGET_VERIFY_EXEC): $(OBJ_VERIFY_EXEC)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_VERIFY_EXEC) $(LDFLAGS)
	@echo "✓ Teste de verificação de execução compilado!"

# Regra para teste single-core sem threads
$(TARGET_SINGLE_CORE): $(OBJ_SINGLE_CORE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_SINGLE_CORE) $(LDFLAGS)
	@echo "✓ Teste single-core (sem threads) compilado!"

# Regra para teste unificado completo (multicore + métricas + cache)
$(TARGET_UNIFIED): $(OBJ_UNIFIED)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_UNIFIED) $(LDFLAGS)
	@echo "✓ Teste unificado completo compilado!"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Limpando arquivos antigos..."
	@rm -f $(OBJ) $(OBJ_HASH) $(OBJ_SIM) $(OBJ_MULTICORE) $(OBJ_THROUGHPUT) $(OBJ_COMPARATIVE) $(OBJ_PREEMPT) $(OBJ_METRICS) $(OBJ_CPU_METRICS) $(OBJ_DEEP_INSPECT) $(OBJ_RACE_DEBUG) $(OBJ_VERIFY_EXEC) $(OBJ_SINGLE_CORE) test_multicore.o
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

# Teste de escalabilidade multicore (1, 2, 4, 8 núcleos)
test_multicore: $(TARGET_MULTICORE)
	@echo "🧪 Executando teste de escalabilidade multicore..."
	@./$(TARGET_MULTICORE)
test-multicore: $(TARGET_MULTICORE)
	@echo "🧪 Executando teste de escalabilidade multicore..."
	@./$(TARGET_MULTICORE)

# Teste comparativo multicore
test-multicore-comparative: $(TARGET_COMPARATIVE)
	@echo "🔬 Executando teste comparativo multicore..."
	@./$(TARGET_COMPARATIVE)

# Teste de throughput - MEDIÇÃO CONFIÁVEL DE EFICIÊNCIA
test-throughput: $(TARGET_THROUGHPUT)
	@echo "🎯 Executando teste de throughput (medição confiável)..."
	@./$(TARGET_THROUGHPUT)

# Teste de preempção por quantum
test-preemption: $(TARGET_PREEMPT)
	@echo "🧪 Executando teste de preempção..."
	@./$(TARGET_PREEMPT)

# Teste de métricas completas
test-metrics-complete: $(TARGET_METRICS)
	@echo "📊 Executando teste de métricas completas..."
	@./$(TARGET_METRICS)

# Teste de métricas de CPU
test-cpu-metrics: $(TARGET_CPU_METRICS)
	@echo "🧪 Executando teste de métricas de CPU..."
	@./$(TARGET_CPU_METRICS)

# Teste de prioridade preemptiva removido

# Teste de inspeção profunda
test-deep-inspection: $(TARGET_DEEP_INSPECT)
	@echo "🧪 Executando teste de inspeção profunda..."
	@./$(TARGET_DEEP_INSPECT)

# Teste de race conditions
test-race-debug: $(TARGET_RACE_DEBUG)
	@echo "🧪 Executando teste de race debug..."
	@./$(TARGET_RACE_DEBUG)

# Teste de verificação de execução
test-verify-execution: $(TARGET_VERIFY_EXEC)
	@echo "🧪 Executando teste de verificação de execução..."
	@./$(TARGET_VERIFY_EXEC)

# Teste single-core sem threads
test-single-core: $(TARGET_SINGLE_CORE)
	@echo "🧪 Executando teste single-core (sem threads)..."
	@./$(TARGET_SINGLE_CORE)

# Teste unificado completo (RECOMENDADO PARA GUI)
test-unified: $(TARGET_UNIFIED)
	@echo "🎯 Executando teste unificado completo (multicore + métricas + cache)..."
	@./$(TARGET_UNIFIED)

# Alias para teste completo
test-complete: test-unified

# Executa TODOS os testes disponíveis em sequência
test-all: $(TARGET_HASH) $(TARGET_BANK) $(TARGET_MULTICORE) $(TARGET_THROUGHPUT) $(TARGET_COMPARATIVE) \
	  $(TARGET_PREEMPT) $(TARGET_METRICS) $(TARGET_CPU_METRICS) \
	  $(TARGET_DEEP_INSPECT) $(TARGET_RACE_DEBUG) $(TARGET_VERIFY_EXEC) $(TARGET_SINGLE_CORE)
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║  🧪 EXECUTANDO BATERIA COMPLETA DE TESTES                 ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "┌─ [1/12] Hash Register Test ────────────────────────────────┐"
	@./$(TARGET_HASH) || true
	@echo ""
	@echo "┌─ [2/12] Register Bank Test ─────────────────────────────────┐"
	@./$(TARGET_BANK) || true
	@echo ""
	@echo "┌─ [3/12] Multicore Scalability Test ─────────────────────────┐"
	@./$(TARGET_MULTICORE) || true
	@echo ""
	@echo "┌─ [4/12] Throughput Test ────────────────────────────────────┐"
	@./$(TARGET_THROUGHPUT) || true
	@echo ""
	@echo "┌─ [5/12] Multicore Comparative Test ─────────────────────────┐"
	@./$(TARGET_COMPARATIVE) || true
	@echo ""
	@echo "┌─ [6/12] Preemption Test ────────────────────────────────────┐"
	@./$(TARGET_PREEMPT) || true
	@echo ""
	@echo "┌─ [7/12] Metrics Complete Test ──────────────────────────────┐"
	@./$(TARGET_METRICS) || true
	@echo ""
	@echo "┌─ [8/12] CPU Metrics Test ───────────────────────────────────┐"
	@./$(TARGET_CPU_METRICS) || true
	@echo ""
	# Priority Preemptive Test removed from test-all
	@echo "┌─ [10/12] Deep Inspection Test ──────────────────────────────┐"
	@./$(TARGET_DEEP_INSPECT) || true
	@echo ""
	@echo "┌─ [11/12] Race Condition Debug Test ─────────────────────────┐"
	@./$(TARGET_RACE_DEBUG) || true
	@echo ""
	@echo "┌─ [12/12] Verify Execution Test ─────────────────────────────┐"
	@./$(TARGET_VERIFY_EXEC) || true
	@echo ""
	@echo "┌─ [13/13] Single-Core Serial Test ───────────────────────────┐"
	@./$(TARGET_SINGLE_CORE) || true
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║  ✅ BATERIA DE TESTES CONCLUÍDA                           ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "📊 Relatórios gerados em:"
	@echo "   - logs/multicore/*.csv"
	@echo "   - logs/memory/*.csv"
	@echo "   - logs/metrics/*.csv"

# Comando de ajuda
help:
	@echo "📋 SO-SimuladorVonNeumann - Comandos Disponíveis:"
	@echo ""
	@echo "  make simulador     - 🎯 Compila simulador multicore Round-Robin"
	@echo "  make run-sim       - 🚀 Executa simulador multicore"
	@echo "  make test-preemption - 🧪 Testa preempção por quantum"
	@echo "  make test-multicore - 🧪 Compila e executa teste de escalabilidade"
	@echo "  make / make all    - Compila e executa programa principal"
	@echo "  make clean         - Remove arquivos gerados (.o, executáveis)"
	@echo "  make run          - Executa programa principal (sem recompilar)"
	@echo "  make teste        - Compila apenas o programa principal"
	@echo "  make test-hash    - Compila e testa sistema de registradores"
	@echo "  make test-bank    - Compila e testa o banco de registradores"
	@echo "  make test-all     - Executa todos os testes disponíveis"
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

.PHONY: all clean run test-hash test-all help check debug list-files
