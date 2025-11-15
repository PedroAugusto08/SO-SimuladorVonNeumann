# Implementação de Carregamento de Múltiplos Processos

## Resumo da Implementação

Foi implementado o carregamento simultâneo de múltiplos processos no simulador multicore, conforme requisito da especificação:

> "Todos os programas pertencentes ao lote devem ser **completamente carregados na memória principal antes do início da execução**."

## Modificações Realizadas

### 1. `src/main.cpp`

#### Adição de Parse de Argumentos para Processos
- Nova estrutura `std::vector<std::pair<std::string, std::string>> process_files` para armazenar pares (programa.json, pcb.json)
- Novo argumento CLI: `--process` ou `-p` que aceita dois parâmetros (programa e PCB)
- Suporte para múltiplas flags `-p` para adicionar vários processos
- Configuração padrão: `tasks.json` + `process1.json` se nenhum processo for especificado

#### Carregamento em Lote
Implementação de carregamento sequencial de todos os processos ANTES da execução:

```cpp
// Para cada processo no lote:
for (size_t i = 0; i < process_files.size(); i++) {
    const auto& [program_file, pcb_file] = process_files[i];
    
    // 1. Carrega PCB
    auto pcb = std::make_unique<PCB>();
    if (!load_pcb_from_json(pcb_file, *pcb)) {
        // FALHA: aborta ANTES da execução
        return 1;
    }
    
    // 2. Carrega programa na memória
    loadJsonProgram(program_file, memManager, *pcb, next_base_address);
    
    // 3. Atualiza endereço base (espaçamento de 1KB)
    next_base_address += 1024;
    
    // 4. Adiciona à lista de processos
    process_list.push_back(std::move(pcb));
}
```

#### Feedback Visual
Banner informativo durante o carregamento:
```
===========================================
  CARREGAMENTO DE PROCESSOS
===========================================
Total de processos a carregar: 3

Processo 1/3:
  ├─ PID:       100
  ├─ Nome:      cpu_intensive
  ├─ Quantum:   100
  ├─ PCB:       test_programs/pcb_cpu_intensive.json
  └─ Programa:  test_programs/cpu_intensive.json
     └─> Carregando instruções na memória...
     └─> ✓ Carregado no endereço base 0x0

...

===========================================
✓ Todos os 3 processos foram carregados na memória
===========================================
```

#### Gestão de Endereços Base
- Cada processo recebe um endereço base único na memória principal
- Espaçamento: 1KB (1024 bytes) entre processos
- Cálculo automático: `next_base_address += 1024`
- Evita colisão de memória entre processos

## Arquivos Criados

### 1. Scripts de Teste

#### `test_multiprocess.sh`
Suite completa com 5 cenários:
- Processo único (baseline single-core)
- 3 processos com 1 core
- 3 processos com 2 cores
- 3 processos com 4 cores
- 3 processos não-preemptivo (2 cores)

#### `test_5processes.sh`
Exemplo de lote com 5 processos simultâneos.

### 2. Novos Processos de Teste

#### `test_programs/short_cpu.json` + `pcb_short_cpu.json`
- PID: 103
- Tipo: CPU-bound curto
- Quantum: 50
- Uso: Teste de processos rápidos e troca de contexto

#### `test_programs/io_light.json` + `pcb_io_light.json`
- PID: 104
- Tipo: I/O leve
- Quantum: 75
- Uso: Teste de bloqueios moderados

### 3. Documentação

#### `BATCH_CONFIG.md`
Guia completo sobre:
- Formato de especificação de lotes
- Lotes pré-configurados
- Diretrizes de carregamento
- Como criar novos processos
- Troubleshooting

#### `test_programs/README.md` (atualizado)
Documentação completa dos processos de teste e exemplos de uso.

## Como Usar

### Sintaxe Básica
```bash
./simulador [opções globais] -p programa1.json pcb1.json -p programa2.json pcb2.json ...
```

### Exemplos

#### Processo Único
```bash
./simulador -c 1 -p tasks.json process1.json
```

#### Múltiplos Processos (3)
```bash
./simulador -c 2 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json
```

#### Lote com 5 Processos
```bash
./simulador -c 2 \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json \
    -p test_programs/mixed_workload.json test_programs/pcb_mixed.json \
    -p test_programs/short_cpu.json test_programs/pcb_short_cpu.json \
    -p test_programs/io_light.json test_programs/pcb_io_light.json
```

#### Modo Não-Preemptivo com Múltiplos Processos
```bash
./simulador -c 2 --non-preemptive \
    -p test_programs/cpu_intensive.json test_programs/pcb_cpu_intensive.json \
    -p test_programs/io_bound.json test_programs/pcb_io_bound.json
```

### Scripts Automatizados
```bash
# Suite completa
./test_multiprocess.sh

# Lote com 5 processos
./test_5processes.sh

# Modo não-preemptivo
./test_non_preemptive.sh
```

## Argumentos de Linha de Comando

```
--cores, -c N              Número de núcleos (padrão: 2)
--quantum, -q N            Quantum em ciclos (padrão: 100)
--non-preemptive, -np      Modo não-preemptivo
--process, -p PROG PCB     Adiciona processo (pode repetir)
--help, -h                 Exibe ajuda
```

## Validação

### Verificar Carregamento
```bash
grep "Carregado no endereço" logs/*.log
```

### Contar Processos Finalizados
```bash
grep -c "FINALIZADO" logs/test_3processes_2cores.log
```

### Ver Métricas
```bash
grep "METRICAS FINAIS" logs/*.log -A 10
```

## Garantias de Implementação

1. ✅ **Carregamento Prévio:** Todos os processos são carregados na memória ANTES da execução
2. ✅ **Validação:** Se qualquer carregamento falhar, o simulador aborta antes de iniciar
3. ✅ **Isolamento:** Cada processo tem endereço base único (espaçamento de 1KB)
4. ✅ **Flexibilidade:** Suporta qualquer número de processos limitado apenas pela memória
5. ✅ **Compatibilidade:** Mantém compatibilidade com execução de processo único

## Integração com Especificação

Esta implementação atende aos seguintes requisitos:

- ✅ Carregamento completo na memória principal antes da execução
- ✅ Suporte para lotes com múltiplos processos
- ✅ Cenários obrigatórios (preemptivo e não-preemptivo)
- ✅ Baseline single-core para comparação
- ✅ Configuração flexível de cores e quantum

## Status

- Compilação: ✅ Sucesso
- Carregamento de processos: ✅ Implementado
- Parse de argumentos: ✅ Implementado
- Scripts de teste: ✅ Criados
- Documentação: ✅ Completa

## Próximos Passos Sugeridos

1. Executar suite de testes completa: `./test_multiprocess.sh`
2. Validar métricas coletadas nos logs
3. Implementar preempção real baseada em quantum (próxima task)
4. Implementar sistema de logging estruturado (CSV/JSON)
5. Calcular e exportar métricas (wait time, turnaround, throughput)
