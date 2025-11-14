#!/bin/bash
# Script para testar o cenário não-preemptivo

echo "=========================================="
echo "  TESTE: CENÁRIO NÃO-PREEMPTIVO"
echo "=========================================="
echo ""

# Compila o simulador
echo "[1/3] Compilando simulador..."
make clean > /dev/null 2>&1
make simulador
if [ $? -ne 0 ]; then
    echo "ERRO: Falha na compilação"
    exit 1
fi
echo "✓ Compilação bem-sucedida"
echo ""

# Teste 1: Single-core não-preemptivo
echo "[2/3] Executando com 1 núcleo (baseline) - NÃO-PREEMPTIVO..."
./simulador --cores 1 --non-preemptive > logs/test_1core_nonpreemptive.log 2>&1
echo "✓ Log salvo em: logs/test_1core_nonpreemptive.log"
echo ""

# Teste 2: Multi-core não-preemptivo
echo "[3/3] Executando com 2 núcleos - NÃO-PREEMPTIVO..."
./simulador --cores 2 --non-preemptive > logs/test_2cores_nonpreemptive.log 2>&1
echo "✓ Log salvo em: logs/test_2cores_nonpreemptive.log"
echo ""

echo "=========================================="
echo "  TESTES CONCLUÍDOS"
echo "=========================================="
echo ""
echo "Verificar logs em:"
echo "  - logs/test_1core_nonpreemptive.log"
echo "  - logs/test_2cores_nonpreemptive.log"
echo ""
echo "Procurar por:"
echo "  - 'modo=NÃO-PREEMPTIVO' na inicialização"
echo "  - Processos executando sem interrupção por quantum"
echo ""
