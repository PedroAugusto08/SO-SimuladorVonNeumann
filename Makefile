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
TARGET_THROUGHPUT := test_multicore_throughput
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

# Fontes para teste de throughput multicore (confiável)
SRC_THROUGHPUT := test_multicore_throughput.cpp \
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
OBJ_THROUGHPUT := $(SRC_THROUGHPUT:.cpp=.o)

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

# Regra para o teste de throughput (medição confiável)
$(TARGET_THROUGHPUT): $(OBJ_THROUGHPUT)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_THROUGHPUT) $(LDFLAGS)
	@echo "✓ Teste de throughput compilado!"

# Regra para o teste de preempção
$(TARGET_PREEMPT): $(OBJ_PREEMPT)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_PREEMPT) $(LDFLAGS)
	@echo "✓ Teste de preempção compilado!"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Limpando arquivos antigos..."
	@rm -f $(OBJ) $(OBJ_HASH) $(OBJ_SIM) $(OBJ_MULTICORE) $(OBJ_THROUGHPUT) $(OBJ_PREEMPT) $(TARGET) $(TARGET_HASH) $(TARGET_BANK) $(TARGET_SIM) $(TARGET_MULTICORE) $(TARGET_THROUGHPUT) $(TARGET_PREEMPT)

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

# Teste de throughput - MEDIÇÃO CONFIÁVEL DE EFICIÊNCIA
test-throughput: $(TARGET_THROUGHPUT)
	@echo "🎯 Executando teste de throughput (medição confiável)..."
	@./$(TARGET_THROUGHPUT)

# Teste de preempção por quantum
test-preemption: $(TARGET_PREEMPT)
	@echo "🧪 Executando teste de preempção..."
	@./$(TARGET_PREEMPT)

# Testa ambos os programas
test-all: clean $(TARGET) $(TARGET_HASH)
	@echo "🚀 Executando programa principal..."
	@./$(TARGET)
	@echo ""
	@echo "🧪 Executando teste do Hash Register..."
	@./$(TARGET_HASH)
	@echo ""
	@echo "🧪 Executando teste do Register Bank..."
	@./$(TARGET_BANK)

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
