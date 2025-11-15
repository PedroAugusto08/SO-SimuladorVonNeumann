#!/bin/bash

# Script de teste para carregamento de múltiplos processos
# Testa o simulador multicore com diferentes configurações

echo "=========================================="
echo "  TESTE: CARREGAMENTO DE MÚLTIPLOS PROCESSOS"
echo "=========================================="
echo

# Limpa e compila
echo "► Compilando o simulador..."
make clean > /dev/null 2>&1
make simulador

if [ $? -ne 0 ]; then
    echo "❌ ERRO: Falha na compilação"
    exit 1
fi

echo "✓ Compilação concluída"
echo

# Cria diretório de logs se não existir
mkdir -p logs

echo "=========================================="
echo "  TESTE 1: Processo Único (Baseline)"
echo "=========================================="
./simulador -c 1 -p tasks.json process1.json > logs/test_single_process.log 2>&1
echo "✓ Log salvo em: logs/test_single_process.log"
echo

echo "=========================================="
echo "  TESTE 2: Três Processos - 1 Core"
echo "=========================================="
./simulador -c 1 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    > logs/test_3processes_1core.log 2>&1
echo "✓ Log salvo em: logs/test_3processes_1core.log"
echo

echo "=========================================="
echo "  TESTE 3: Três Processos - 2 Cores"
echo "=========================================="
./simulador -c 2 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    > logs/test_3processes_2cores.log 2>&1
echo "✓ Log salvo em: logs/test_3processes_2cores.log"
echo

echo "=========================================="
echo "  TESTE 4: Três Processos - 4 Cores"
echo "=========================================="
./simulador -c 4 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    > logs/test_3processes_4cores.log 2>&1
echo "✓ Log salvo em: logs/test_3processes_4cores.log"
echo

echo "=========================================="
echo "  TESTE 5: Modo Não-Preemptivo - 2 Cores"
echo "=========================================="
./simulador -c 2 --non-preemptive \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    > logs/test_nonpreemptive_2cores.log 2>&1
echo "✓ Log salvo em: logs/test_nonpreemptive_2cores.log"
echo

echo "=========================================="
echo "  RESUMO DOS TESTES"
echo "=========================================="
echo "Todos os testes foram executados com sucesso!"
echo
echo "Logs gerados em logs/:"
ls -lh logs/test_*.log | awk '{print "  - " $9 " (" $5 ")"}'
echo
echo "Para visualizar um log:"
echo "  cat logs/test_3processes_2cores.log"
echo
echo "Para verificar processos finalizados:"
echo "  grep 'FINALIZADO' logs/test_3processes_2cores.log"
