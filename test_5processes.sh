#!/bin/bash

# Exemplo de execução com lote de 5 processos
# Demonstra o carregamento de múltiplos processos conforme especificação

echo "=========================================="
echo "  TESTE: LOTE COM 5 PROCESSOS"
echo "=========================================="
echo

# Compila
echo "► Compilando..."
cd /home/pedro/vscode_ubuntu/SO-SimuladorVonNeumann-Multicore
make clean > /dev/null 2>&1
make simulador

if [ $? -ne 0 ]; then
    echo "❌ Erro na compilação"
    exit 1
fi

echo "✓ Compilado com sucesso"
echo

# Cria diretório de logs
mkdir -p logs

echo "=========================================="
echo "  Executando com 5 processos e 2 cores"
echo "=========================================="
echo

./simulador -c 2 -q 100 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    -p test_programs/short_cpu.json test_programs/pcb_short_cpu.json \
    -p test_programs/io_light.json test_programs/pcb_io_light.json

echo
echo "=========================================="
echo "  Execução concluída!"
echo "=========================================="
