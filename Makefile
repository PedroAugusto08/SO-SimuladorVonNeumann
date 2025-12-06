# Compilador e flags
CXX := g++
CXXFLAGS := -Wall -Wextra -g -std=c++17 -Isrc
LDFLAGS := -lpthread

# Alvos principais
TARGET := teste
TARGET_HASH := test_hash_register
TARGET_BANK := test_register_bank
TARGET_SIM := simulador
TARGET_MULTICORE := test_multicore
TARGET_PREEMPT := test_preemption

# Fontes principais
SRC := src/teste.cpp src/cpu/ULA.cpp
OBJ := $(SRC:.cpp=.o)

# Fontes para teste do hash register
SRC_HASH := src/test_hash_register.cpp
OBJ_HASH := $(SRC_HASH:.cpp=.o)

# Fontes para teste do register bank
SRC_BANK := src/test_register_bank.cpp src/cpu/REGISTER_BANK.cpp
OBJ_BANK := $(SRC_BANK:.cpp=.o)

# Fontes para o simulador multicore
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
OBJ_SIM := $(SRC_SIM:.cpp=.o)

# Fontes para teste de escalabilidade multicore
SRC_MULTICORE := test_multicore.cpp \
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
OBJ_MULTICORE := $(SRC_MULTICORE:.cpp=.o)

# Fontes para teste de preempção
SRC_PREEMPT := test_preemption.cpp \
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
OBJ_PREEMPT := $(SRC_PREEMPT:.cpp=.o)

<<<<<<< Updated upstream
=======
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
		BASE_TEST_SRC := $(BASE_TEST_SRC) \
		                src/util/Log.cpp

SRC_CPU_METRICS := test/test_cpu_metrics.cpp $(BASE_TEST_SRC)
OBJ_CPU_METRICS := $(SRC_CPU_METRICS:.cpp=.o)

SRC_SCHED_PENDING := test/test_scheduler_pending.cpp $(BASE_TEST_SRC)
OBJ_SCHED_PENDING := $(SRC_SCHED_PENDING:.cpp=.o)

# Orphan detection test
SRC_ORPHAN := test/test_orphan_detection.cpp $(BASE_TEST_SRC)
OBJ_ORPHAN := $(SRC_ORPHAN:.cpp=.o)

# SRC_PRIORITY_PREEMPT removed (priority preemptive tests removed)

SRC_DEEP_INSPECT := test/test_deep_inspection.cpp $(BASE_TEST_SRC)
OBJ_DEEP_INSPECT := $(SRC_DEEP_INSPECT:.cpp=.o)

SRC_RACE_DEBUG := test/test_race_debug.cpp $(BASE_TEST_SRC)
OBJ_RACE_DEBUG := $(SRC_RACE_DEBUG:.cpp=.o)


# Fontes para teste de métricas com núcleos fixos
SRC_METRICS := test/test_metrics.cpp $(BASE_TEST_SRC)
OBJ_METRICS := $(SRC_METRICS:.cpp=.o)

# Fontes para teste de sanidade das métricas
SRC_SANITY := test/test_metrics_sanity.cpp $(BASE_TEST_SRC)
OBJ_SANITY := $(SRC_SANITY:.cpp=.o)

# Testes para MemoryManager
SRC_MEM_TEST := test/test_memory_manager.cpp $(BASE_TEST_SRC)
OBJ_MEM_TEST := $(SRC_MEM_TEST:.cpp=.o)

>>>>>>> Stashed changes
# Make clean -> make -> make run
all: clean $(TARGET) run

# Regra para o simulador multicore
$(TARGET_SIM): $(OBJ_SIM)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_SIM) $(LDFLAGS)
	@echo "✓ Simulador multicore compilado com sucesso!"

# Regra para o programa principal
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Regra para o teste do hash register
$(TARGET_HASH): $(OBJ_HASH)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_HASH)

# Regra para o teste do register bank
$(TARGET_BANK): $(OBJ_BANK)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_BANK)

# Regra para o teste de escalabilidade multicore
$(TARGET_MULTICORE): $(OBJ_MULTICORE)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_MULTICORE) $(LDFLAGS)
	@echo "✓ Teste de escalabilidade multicore compilado!"

# Regra para o teste de preempção
$(TARGET_PREEMPT): $(OBJ_PREEMPT)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_PREEMPT) $(LDFLAGS)
	@echo "✓ Teste de preempção compilado!"

<<<<<<< Updated upstream
=======
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

# Regra para teste single-core sem threads
$(TARGET_SINGLE_CORE): $(OBJ_SINGLE_CORE)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_SINGLE_CORE) $(LDFLAGS)
	@echo "✓ Teste single-core (sem threads) compilado!"

# Regra para teste de métricas com núcleos fixos
$(TARGET_METRICS): $(OBJ_METRICS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_METRICS) $(LDFLAGS)
	@echo "✓ Teste de métricas multicore (núcleos fixos) compilado!"

# Regra para teste de sanidade das métricas
$(BIN_DIR)/test_sanity: $(OBJ_SANITY)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_SANITY) $(LDFLAGS)
	@echo "✓ Teste de sanidade das métricas compilado!"

# Build + run test-metrics with AddressSanitizer enabled for debugging heap corruption
test-metrics-asan: CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer -O1 -g
test-metrics-asan: clean $(TARGET_METRICS)
	@echo "🧪 Executando test-metrics com AddressSanitizer..."
	@ASAN_OPTIONS=allocator_release_delay_ms=0:detect_leaks=1 ./$(TARGET_METRICS)

# Strict metric tests (fail on sanity check violations)
test-metrics-strict: clean $(TARGET_METRICS)
	@echo "🧪 Executando test-metrics em modo estrito (METRICS_STRICT=1)..."
	@METRICS_STRICT=1 ./$(TARGET_METRICS)

$(TARGET_ANALYZER): tools/scheduler_analyzer.cpp
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "✓ Analyzer compilado!"

>>>>>>> Stashed changes
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Limpando arquivos antigos..."
	@rm -f $(OBJ) $(OBJ_HASH) $(OBJ_SIM) $(OBJ_MULTICORE) $(OBJ_PREEMPT) $(TARGET) $(TARGET_HASH) $(TARGET_BANK) $(TARGET_SIM) $(TARGET_MULTICORE) $(TARGET_PREEMPT)

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
test-multicore: $(TARGET_MULTICORE)
	@echo "🧪 Executando teste de escalabilidade multicore..."
	@./$(TARGET_MULTICORE)

<<<<<<< Updated upstream
# Teste de preempção por quantum
test-preemption: $(TARGET_PREEMPT)
	@echo "🧪 Executando teste de preempção..."
	@./$(TARGET_PREEMPT)

# Testa ambos os programas
test-all: clean $(TARGET) $(TARGET_HASH)
	@echo "🚀 Executando programa principal..."
	@./$(TARGET)
=======
test-scheduler-pending: $(OBJ_SCHED_PENDING)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/test_scheduler_pending $(OBJ_SCHED_PENDING) $(LDFLAGS) || true
	@echo "🧪 Executando teste de 'has_pending_processes'..."
	@./$(BIN_DIR)/test_scheduler_pending || true

test-orphan-detection: $(OBJ_ORPHAN)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/test_orphan_detection $(OBJ_ORPHAN) $(LDFLAGS) || true
	@echo "🧪 Executando teste de 'orphan detection'..."
	@./$(BIN_DIR)/test_orphan_detection || true

# Teste single-core sem threads
test-single-core: $(TARGET_SINGLE_CORE)
	@echo "🧪 Executando teste single-core (sem threads)..."
	@./$(TARGET_SINGLE_CORE)

# Teste unificado simplificado (núcleos fixos)
test-metrics: $(TARGET_METRICS)
	@./$(TARGET_METRICS)

# Teste de métricas incluindo o workload pesado
test-metrics-heavy: $(TARGET_METRICS)
	@echo "🧪 Executando teste de métricas com workload pesado (loop_heavy)..."
	@USE_LOOP_HEAVY=1 ./$(TARGET_METRICS)

# Alias para teste completo
test-complete: test-metrics

# Teste de sanidade
test-sanity: $(BIN_DIR)/test_sanity
	@./$(BIN_DIR)/test_sanity

# Teste MemoryManager
test-mem: $(OBJ_MEM_TEST)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/test_memory_manager $(OBJ_MEM_TEST) $(LDFLAGS)
	@echo "🧪 Executando teste MemoryManager..."
	@./$(BIN_DIR)/test_memory_manager || true

# Executa TODOS os testes disponíveis em sequência (mantendo apenas os suportados)
test-all: $(TARGET_HASH) $(TARGET_BANK) $(TARGET_PREEMPT) $(TARGET_CPU_METRICS) \
	  $(TARGET_DEEP_INSPECT) $(TARGET_RACE_DEBUG) $(TARGET_SINGLE_CORE) $(TARGET_METRICS)
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║  🧪 EXECUTANDO BATERIA COMPLETA DE TESTES                 ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
>>>>>>> Stashed changes
	@echo ""
	@echo "🧪 Executando teste do Hash Register..."
	@./$(TARGET_HASH)
	@echo ""
<<<<<<< Updated upstream
	@echo "🧪 Executando teste do Register Bank..."
	@./$(TARGET_BANK)
=======
	@echo "┌─ [2/8] Register Bank Test ─────────────────────────────────┐"
	@./$(TARGET_BANK) || true
	@echo ""
	@echo "┌─ [3/8] Preemption Test ─────────────────────────────────────┐"
	@./$(TARGET_PREEMPT) || true
	@echo ""
	@echo "┌─ [4/8] CPU Metrics Test ────────────────────────────────────┐"
	@./$(TARGET_CPU_METRICS) || true
	@echo ""
	# Priority Preemptive Test removed from test-all
	@echo "┌─ [5/8] Deep Inspection Test ───────────────────────────────┐"
	@./$(TARGET_DEEP_INSPECT) || true
	@echo ""
	@echo "┌─ [6/8] Race Condition Debug Test ──────────────────────────┐"
	@./$(TARGET_RACE_DEBUG) || true
	@echo ""
	@echo "┌─ [7/8] Single-Core Serial Test ────────────────────────────┐"
	@./$(TARGET_SINGLE_CORE) || true
	@echo ""
	@echo "┌─ [8/8] Fixed-Core Metrics Test ───────────────────────────┐"
	@./$(TARGET_METRICS) || true
	@./$(BIN_DIR)/test_sanity || true
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║  ✅ BATERIA DE TESTES CONCLUÍDA                           ║"
	@echo "╚════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "📊 Relatórios gerados em:"
	@echo "   - logs/multicore/*.csv"
	@echo "   - logs/memory/*.csv"
	@echo "   - logs/metrics/*.csv"
>>>>>>> Stashed changes

# Comando de ajuda
help:
	@echo "📋 SO-SimuladorVonNeumann - Comandos Disponíveis:"
	@echo ""
	@echo "  make simulador     - 🎯 Compila simulador multicore Round-Robin"
	@echo "  make run-sim       - 🚀 Executa simulador multicore"
	@echo "  make test-preemption - 🧪 Testa preempção por quantum"
<<<<<<< Updated upstream
	@echo "  make test-multicore - 🧪 Compila e executa teste de escalabilidade"
=======
	@echo "  make test-sanity   - 🧪 Executa testes de sanidade para as métricas"
	@echo "  make test-metrics-heavy - 🧪 Executa métricas com workload pesado"
	@echo "  make test-metrics  - 🎯 Gera todos os CSVs/relatórios para a GUI"
>>>>>>> Stashed changes
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
check: $(TARGET) $(TARGET_HASH)
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
