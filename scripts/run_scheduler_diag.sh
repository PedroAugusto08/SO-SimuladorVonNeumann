#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/bin"
BINARY="$BUILD_DIR/test_metrics"
COMPILER="${CXX:-g++}"
CXXFLAGS="-Wall -Wextra -g -std=c++17 -Isrc"
LDFLAGS="-lpthread"
LOG_FILE="$ROOT_DIR/logs/scheduler_dumps.log"

SOURCES=(
  "test/test_metrics.cpp"
  "src/cpu/Core.cpp"
  "src/cpu/RoundRobinScheduler.cpp"
  "src/cpu/CONTROL_UNIT.cpp"
  "src/cpu/pcb_loader.cpp"
  "src/cpu/REGISTER_BANK.cpp"
  "src/cpu/ULA.cpp"
  "src/cpu/FCFSScheduler.cpp"
  "src/cpu/SJNScheduler.cpp"
  "src/cpu/PriorityScheduler.cpp"
  "src/IO/IOManager.cpp"
  "src/memory/cache.cpp"
  "src/memory/cachePolicy.cpp"
  "src/memory/MAIN_MEMORY.cpp"
  "src/memory/MemoryManager.cpp"
  "src/memory/SECONDARY_MEMORY.cpp"
  "src/parser_json/parser_json.cpp"
)

usage() {
  cat <<USAGE
Uso: $0 [policy-list] [core-list]

  policy-list  Lista separada por vírgula (ex: FCFS,SJN). Padrão: FCFS
  core-list    Lista separada por vírgula (ex: 4,6).     Padrão: 4

O script compila (se necessário) e executa test_metrics com os filtros
informados e exibe o final do arquivo logs/scheduler_dumps.log para facilitar a
depuração de estouros de MAX_CYCLES.
USAGE
}

if [[ "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

POLICY_FILTER_VALUE="${1:-FCFS}"
CORE_FILTER_VALUE="${2:-4}"

echo "[diag] POLICY_FILTER=$POLICY_FILTER_VALUE"
echo "[diag] CORE_FILTER=$CORE_FILTER_VALUE"

mkdir -p "$BUILD_DIR"

should_build=false
if [[ ! -x "$BINARY" ]]; then
  should_build=true
else
  for src in "${SOURCES[@]}"; do
    if [[ "$ROOT_DIR/$src" -nt "$BINARY" ]]; then
      should_build=true
      break
    fi
  done
fi

if [[ "$should_build" == true ]]; then
  echo "[diag] Compilando binário de diagnóstico..."
  "$COMPILER" $CXXFLAGS "${SOURCES[@]/#/$ROOT_DIR/}" -o "$BINARY" $LDFLAGS
else
  echo "[diag] Reaproveitando $BINARY"
fi

echo "[diag] Executando testes instrumentados..."
export POLICY_FILTER="$POLICY_FILTER_VALUE"
export CORE_FILTER="$CORE_FILTER_VALUE"
"$BINARY"

if [[ -f "$LOG_FILE" ]]; then
  echo "[diag] Últimas entradas de $LOG_FILE:"
  tail -n 80 "$LOG_FILE"
else
  echo "[diag] Nenhum dump encontrado em $LOG_FILE"
fi
