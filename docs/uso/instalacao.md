# Instalação e Compilação

## Requisitos

### Sistema Operacional

| SO | Suportado | Notas |
|----|-----------|-------|
| Linux (Ubuntu 20.04+) | ✅ Recomendado | Testado |
| WSL2 (Windows) | ✅ | Usar distro Ubuntu |
| macOS | ⚠️ | Pode funcionar com ajustes |
| Windows nativo | ❌ | Use WSL2 |

### Dependências

| Dependência | Versão Mínima | Instalação |
|-------------|---------------|------------|
| GCC/G++ | 9.0+ | `apt install g++` |
| Make | 4.0+ | `apt install make` |
| CMake | 3.16+ | `apt install cmake` |
| pthread | - | Incluído no sistema |

## Instalação Rápida

### Ubuntu/Debian

```bash
# Atualizar pacotes
sudo apt update

# Instalar dependências
sudo apt install -y build-essential g++ make cmake

# Verificar instalação
g++ --version
make --version
```

### WSL2 (Windows)

1. Abrir PowerShell como Administrador
2. Instalar WSL:
   ```powershell
   wsl --install -d Ubuntu
   ```
3. Reiniciar e configurar usuário
4. Seguir instruções do Ubuntu acima

## Compilação

### Clone do Repositório

```bash
git clone <repositorio>
cd SO-SimuladorVonNeumann
```

### Build Básico

```bash
# Compilar tudo
make

# Apenas o simulador
make simulador
```

### Build de Testes

```bash
# Compilar e executar teste de métricas
make test-metrics

# Teste single-core determinístico
make test-single-core

# Testes de componentes
make test-hash
make test-bank
```

### Limpeza

```bash
# Limpar objetos
make clean

# Limpar tudo
make distclean
```

## Makefile - Comandos Principais

| Comando | Descrição |
|---------|-----------|
| `make` ou `make all` | Limpa e compila o simulador principal |
| `make simulador` | Compila apenas o simulador multicore |
| `make run-sim` | Executa o simulador multicore |
| `make test-metrics` | Compila e executa testes de métricas |
| `make test-single-core` | Teste single-core sem threads |
| `make test-hash` | Teste do Hash Register |
| `make test-bank` | Teste do Register Bank |
| `make check` | Verificação rápida de componentes |
| `make clean` | Remove arquivos compilados |
| `make debug` | Build com símbolos de debug |
| `make help` | Mostra ajuda com todos os comandos |

## Verificação da Instalação

```bash
# Compilar
make simulador

# Verificar executável
ls -la bin/simulador

# Teste rápido
./bin/simulador --help
```

**Saída esperada:**
```
Simulador Von Neumann Multicore
Uso: ./bin/simulador [opções]
  --cores N        Número de núcleos (1-8)
  --policy POLICY  Política de escalonamento
  --quantum N      Quantum para Round Robin
  ...
```

## Estrutura de Diretórios

Após compilação:

```
SO-SimuladorVonNeumann/
├── bin/                   # Executáveis compilados
│   ├── simulador          # Simulador principal
│   ├── test_metrics       # Teste de métricas
│   ├── test_single_core_no_threads
│   ├── test_hash_register
│   └── test_register_bank
├── src/
│   └── *.o               # Objetos compilados
├── dados_graficos/        # Dados e relatórios de métricas
│   ├── csv/              # Arquivos CSV com métricas
│   ├── reports/          # Relatórios de texto
│   └── graficos/         # Gráficos gerados
├── test/output/           # Saída dos testes
└── ...
```
```

## Problemas Comuns

### Erro: g++ not found

```bash
sudo apt install g++
```

### Erro: pthread not found

```bash
# Já deve estar incluído, mas:
sudo apt install libc6-dev
```

### Erro: make not found

```bash
sudo apt install make
```

### Permissão negada ao executar

```bash
chmod +x simulador
```

## Configuração do IDE

### VS Code

1. Instalar extensão C/C++
2. Criar `.vscode/c_cpp_properties.json`:

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/src",
                "${workspaceFolder}/src/nlohmann"
            ],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17"
        }
    ]
}
```

### CLion

1. Abrir pasta do projeto
2. CLion detecta CMakeLists.txt automaticamente
3. Build → Build Project

## Próximos Passos

Após instalação bem-sucedida:

1. [Comandos de Uso](comandos.md) - Como executar o simulador
2. [Arquitetura](../guia/arquitetura.md) - Entender a estrutura
3. [Escalonadores](../escalonadores/fcfs.md) - Políticas disponíveis
