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
# Compilar todos os testes
make test

# Teste específico
make test_multicore
make test_metrics
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
| `make` | Compila o simulador principal |
| `make all` | Compila tudo (simulador + testes) |
| `make test` | Compila e executa testes |
| `make clean` | Remove arquivos compilados |
| `make run` | Compila e executa |

## Verificação da Instalação

```bash
# Compilar
make

# Verificar executável
ls -la simulador

# Teste rápido
./simulador --help
```

**Saída esperada:**
```
Simulador Von Neumann Multicore
Uso: ./simulador [opções]
  --cores N        Número de núcleos (1-8)
  --policy POLICY  Política de escalonamento
  --quantum N      Quantum para Round Robin
  ...
```

## Estrutura de Diretórios

Após compilação:

```
SO-SimuladorVonNeumann/
├── simulador              # Executável principal
├── test_multicore         # Teste multicore
├── test_metrics           # Teste de métricas
├── src/
│   ├── *.o               # Objetos compilados
│   └── ...
├── logs/
│   └── metrics/          # Logs de execução
└── ...
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
