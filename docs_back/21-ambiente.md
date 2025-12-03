# Instalação do Ambiente

## 🎯 Objetivo

Guia completo para configurar o ambiente de desenvolvimento para o simulador multicore.

---

## 💻 Requisitos de Sistema

### Mínimos
- **CPU:** 2 cores, 2.0 GHz
- **RAM:** 4 GB
- **Disco:** 2 GB livres
- **SO:** Linux, macOS, ou Windows (com WSL)

### Recomendados
- **CPU:** 4+ cores, 3.0+ GHz
- **RAM:** 8+ GB
- **Disco:** 5+ GB SSD
- **SO:** Ubuntu 22.04 LTS ou similar

---

## 🐧 Instalação no Linux (Ubuntu/Debian)

### 1. Atualizar Sistema

```bash
sudo apt update
sudo apt upgrade -y
```

---

### 2. Instalar Compiladores e Ferramentas

```bash
# GCC e G++
sudo apt install build-essential -y

# CMake
sudo apt install cmake -y

# Git
sudo apt install git -y

# Verificar versões
gcc --version    # Deve ser 11+
g++ --version    # Deve ser 11+
cmake --version  # Deve ser 3.14+
```

---

### 3. Instalar Bibliotecas de Desenvolvimento

```bash
# Threads POSIX
sudo apt install libpthread-stubs0-dev -y

# JSON library (nlohmann-json)
sudo apt install nlohmann-json3-dev -y
```

---

### 4. Instalar Ferramentas de Debugging

```bash
# GDB
sudo apt install gdb -y

# Valgrind
sudo apt install valgrind -y

# Google Test
sudo apt install libgtest-dev -y

# Build Google Test
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib
```

---

### 5. Instalar Python e Ferramentas de Análise

```bash
# Python 3
sudo apt install python3 python3-pip -y

# Bibliotecas para análise
pip3 install pandas matplotlib seaborn numpy
```

---

### 6. Instalar VS Code (Opcional)

```bash
# Baixar e instalar
wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
sudo install -o root -g root -m 644 packages.microsoft.gpg /etc/apt/trusted.gpg.d/
sudo sh -c 'echo "deb [arch=amd64] https://packages.microsoft.com/repos/vscode stable main" > /etc/apt/sources.list.d/vscode.list'

sudo apt update
sudo apt install code -y
```

**Extensões Recomendadas:**
- C/C++ (Microsoft)
- CMake Tools
- Git Graph
- Better C++ Syntax

---

## 🍎 Instalação no macOS

### 1. Instalar Homebrew

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

---

### 2. Instalar Ferramentas de Desenvolvimento

```bash
# Xcode Command Line Tools
xcode-select --install

# GCC/G++ (via Homebrew)
brew install gcc

# CMake
brew install cmake

# Git
brew install git
```

---

### 3. Instalar Bibliotecas

```bash
# JSON library
brew install nlohmann-json

# Google Test
brew install googletest
```

---

### 4. Instalar Python e Ferramentas

```bash
# Python 3
brew install python3

# Bibliotecas
pip3 install pandas matplotlib seaborn numpy
```

---

### 5. Instalar Ferramentas de Debugging

```bash
# LLDB já vem com Xcode
# Valgrind (não suportado em macOS recente)
# Use alternativas como Address Sanitizer

# GDB (opcional)
brew install gdb
```

---

## 🪟 Instalação no Windows (WSL)

### 1. Instalar WSL 2

```powershell
# No PowerShell como Administrador
wsl --install -d Ubuntu-22.04
```

Reinicie o computador.

---

### 2. Configurar Ubuntu no WSL

Abra o Ubuntu e siga as mesmas instruções da [seção Linux](#-instalação-no-linux-ubuntudebian).

---

### 3. Instalar VS Code no Windows

Baixe e instale de: https://code.visualstudio.com/

**Extensões Essenciais:**
- Remote - WSL
- C/C++
- CMake Tools

---

### 4. Conectar VS Code ao WSL

```bash
# No terminal WSL, no diretório do projeto
code .
```

---

## 🔧 Configuração do Projeto

### 1. Clonar Repositório

```bash
git clone https://github.com/PedroAugusto08/SO-SimuladorVonNeumann.git
cd SO-SimuladorVonNeumann
```

---

### 2. Criar Estrutura de Build

```bash
# Criar diretório de build
mkdir -p build
cd build

# Configurar com CMake (se disponível)
cmake ..

# Ou usar Make diretamente
cd ..
make
```

---

### 3. Compilar e Testar

```bash
# Compilar
make

# Executar testes
make test

# Executar simulador
./simulador
```

---

## 🐳 Instalação com Docker (Alternativa)

### Dockerfile

```dockerfile
FROM ubuntu:22.04

# Evitar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependências
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    gdb \
    valgrind \
    nlohmann-json3-dev \
    libgtest-dev \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Instalar bibliotecas Python
RUN pip3 install pandas matplotlib seaborn numpy

# Diretório de trabalho
WORKDIR /workspace

# Comando padrão
CMD ["/bin/bash"]
```

---

### Usar Docker

```bash
# Build da imagem
docker build -t simulador-dev .

# Executar container
docker run -it -v $(pwd):/workspace simulador-dev

# Dentro do container
cd /workspace
make
./simulador
```

---

## ✅ Verificação da Instalação

Execute o seguinte script para verificar:

```bash
#!/bin/bash
# verify_install.sh

echo "=== Verificando Instalação ==="

# Função para verificar comando
check_command() {
    if command -v $1 &> /dev/null; then
        echo "✅ $1 encontrado"
        $1 --version | head -n 1
    else
        echo "❌ $1 NÃO encontrado"
    fi
    echo
}

# Verificar ferramentas
check_command gcc
check_command g++
check_command cmake
check_command git
check_command gdb
check_command valgrind
check_command python3

# Verificar bibliotecas Python
echo "=== Bibliotecas Python ==="
python3 -c "import pandas; print('✅ pandas')" 2>/dev/null || echo "❌ pandas"
python3 -c "import matplotlib; print('✅ matplotlib')" 2>/dev/null || echo "❌ matplotlib"
python3 -c "import seaborn; print('✅ seaborn')" 2>/dev/null || echo "❌ seaborn"
python3 -c "import numpy; print('✅ numpy')" 2>/dev/null || echo "❌ numpy"

echo
echo "=== Verificação Concluída ==="
```

Executar:
```bash
chmod +x verify_install.sh
./verify_install.sh
```

---

## 🐛 Troubleshooting

### Erro: comando não encontrado

**Solução:** Certifique-se de que os pacotes foram instalados:
```bash
sudo apt install build-essential cmake
```

---

### Erro de compilação: thread not found

**Solução:** Adicione flag `-pthread`:
```bash
g++ -std=c++17 -pthread main.cpp -o simulador
```

---

### Erro: Google Test não encontrado

**Solução:**
```bash
sudo apt install libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

---

### WSL muito lento

**Soluções:**
- Coloque o projeto dentro do filesystem WSL, não em /mnt/c/
- Desabilite antivírus temporariamente
- Use WSL 2 ao invés de WSL 1

---

## 📚 Recursos Adicionais

### Documentação Oficial
- GCC: https://gcc.gnu.org/onlinedocs/
- CMake: https://cmake.org/documentation/
- Google Test: https://google.github.io/googletest/

### Tutoriais
- C++17: https://en.cppreference.com/w/
- Threads: https://en.cppreference.com/w/cpp/thread
- CMake: https://cmake.org/cmake/help/latest/guide/tutorial/

---

## 🔗 Próximos Passos

- ➡️ [Comandos Úteis](22-comandos.md)
- ➡️ [Quickstart](QUICKSTART.md)
- ➡️ [WSL Quickstart](WSL_QUICKSTART.md)

---

**Última atualização:** Novembro 2025
