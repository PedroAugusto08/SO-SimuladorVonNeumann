# Comandos Úteis

## 🎯 Objetivo

Referência rápida de comandos para desenvolvimento, compilação, teste e debugging do simulador.

---

## 🔨 Compilação

### Make

```bash
# Compilar projeto
make

# Compilar com debug
make debug

# Compilar com sanitizers
make sanitize

# Compilar em paralelo (mais rápido)
make -j$(nproc)

# Limpar arquivos compilados
make clean

# Ver ajuda
make help
```

---

### CMake

```bash
# Configurar projeto
mkdir build && cd build
cmake ..

# Com tipo de build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_BUILD_TYPE=Release ..

# Compilar
cmake --build .

# Compilar em paralelo
cmake --build . -j$(nproc)

# Instalar
sudo cmake --install .
```

---

### Compilação Manual (g++)

```bash
# Básico
g++ -std=c++17 main.cpp -o simulador

# Com otimização
g++ -std=c++17 -O3 main.cpp -o simulador

# Com threads
g++ -std=c++17 -pthread main.cpp -o simulador

# Com warnings
g++ -std=c++17 -Wall -Wextra -pthread main.cpp -o simulador

# Debug
g++ -std=c++17 -g -pthread main.cpp -o simulador

# Com sanitizers
g++ -std=c++17 -fsanitize=address -g main.cpp -o simulador
g++ -std=c++17 -fsanitize=thread -g main.cpp -o simulador
```

---

## 🏃 Execução

### Executar Simulador

```bash
# Padrão
./simulador

# Com argumentos
./simulador 4 20  # 4 cores, quantum 20

# Com redirecionamento de saída
./simulador > output.txt

# Com entrada de arquivo
./simulador < input.json

# Executar e salvar log
./simulador 2>&1 | tee simulation.log
```

---

### Executar Testes

```bash
# Todos os testes
make test

# Testes específicos
./test_simulador

# Com verbose
./test_simulador --gtest_verbose

# Filtrar testes
./test_simulador --gtest_filter=CoreTest.*

# Listar testes
./test_simulador --gtest_list_tests
```

---

## 🐛 Debugging

### GDB

```bash
# Iniciar GDB
gdb ./simulador

# Comandos dentro do GDB
(gdb) break main              # Breakpoint em função
(gdb) break main.cpp:42       # Breakpoint em linha
(gdb) run                     # Executar
(gdb) run 4 20               # Executar com argumentos
(gdb) next                    # Próxima linha
(gdb) step                    # Entrar em função
(gdb) continue                # Continuar execução
(gdb) print variable          # Ver valor
(gdb) backtrace               # Stack trace
(gdb) info threads            # Listar threads
(gdb) thread 2                # Mudar para thread 2
(gdb) quit                    # Sair
```

---

### Valgrind

```bash
# Memory leaks
valgrind --leak-check=full ./simulador

# Memory leaks detalhado
valgrind --leak-check=full --show-leak-kinds=all ./simulador

# Race conditions (Helgrind)
valgrind --tool=helgrind ./simulador

# Cache misses (Cachegrind)
valgrind --tool=cachegrind ./simulador

# Profiling (Callgrind)
valgrind --tool=callgrind ./simulador
```

---

### Sanitizers

```bash
# AddressSanitizer (memory errors)
g++ -fsanitize=address -g main.cpp -o simulador
./simulador

# ThreadSanitizer (race conditions)
g++ -fsanitize=thread -g -O1 main.cpp -o simulador
./simulador

# UndefinedBehaviorSanitizer
g++ -fsanitize=undefined -g main.cpp -o simulador
./simulador
```

---

## 📊 Profiling

### gprof

```bash
# Compilar com profiling
g++ -pg main.cpp -o simulador

# Executar
./simulador

# Gerar relatório
gprof simulador gmon.out > analysis.txt

# Ver top 10 funções
gprof simulador gmon.out | head -n 50
```

---

### perf

```bash
# Instalar (se necessário)
sudo apt install linux-tools-common linux-tools-generic

# Profile execution
perf record ./simulador

# Ver relatório
perf report

# Profile por função
perf record -g ./simulador

# Estatísticas
perf stat ./simulador
```

---

## 🧪 Testes

### Executar Testes Específicos

```bash
# Testes unitários
./test_simulador --gtest_filter=*Unit*

# Testes de integração
./test_simulador --gtest_filter=*Integration*

# Testes de sistema
./test_simulador --gtest_filter=*System*

# Um teste específico
./test_simulador --gtest_filter=CoreTest.Initialization
```

---

### Cobertura de Código

```bash
# Compilar com cobertura
g++ -coverage -fprofile-arcs -ftest-coverage test.cpp -o test

# Executar testes
./test

# Gerar relatório
gcov test.cpp

# Com lcov (HTML)
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report

# Ver no navegador
firefox coverage_report/index.html
```

---

## 📈 Análise de Resultados

### Python Scripts

```bash
# Análise básica
python3 analysis.py results.csv

# Gerar todos os gráficos
python3 analysis.py results.csv --all-graphs

# Apenas gráfico de speedup
python3 analysis.py results.csv --speedup

# Comparar execuções
python3 compare.py results1.csv results2.csv
```

---

### Processamento de CSV

```bash
# Ver primeiras linhas
head -n 10 results.csv

# Ver últimas linhas
tail -n 10 results.csv

# Contar linhas
wc -l results.csv

# Filtrar colunas (usando awk)
awk -F',' '{print $1,$3}' results.csv

# Ordenar por coluna
sort -t',' -k2 -n results.csv

# Calcular média (coluna 2)
awk -F',' '{sum+=$2; n++} END {print sum/n}' results.csv
```

---

## 🔍 Busca e Navegação

### grep

```bash
# Buscar em arquivos
grep "TODO" src/*.cpp

# Buscar recursivamente
grep -r "Multicore" src/

# Buscar ignorando case
grep -i "scheduler" src/*.cpp

# Buscar com contexto
grep -C 3 "deadlock" src/*.cpp

# Contar ocorrências
grep -c "mutex" src/*.cpp
```

---

### find

```bash
# Encontrar arquivos por nome
find src/ -name "*.cpp"

# Encontrar e executar comando
find src/ -name "*.cpp" -exec wc -l {} \;

# Arquivos modificados recentemente
find src/ -mtime -1

# Arquivos maiores que 1MB
find . -size +1M
```

---

## 📦 Git

### Comandos Básicos

```bash
# Status
git status

# Adicionar arquivos
git add .
git add src/*.cpp

# Commit
git commit -m "Mensagem do commit"

# Push
git push origin main

# Pull
git pull origin main

# Ver log
git log --oneline
git log --graph --oneline --all

# Ver diferenças
git diff
git diff HEAD~1
```

---

### Branches

```bash
# Listar branches
git branch

# Criar branch
git branch feature-multicore

# Mudar para branch
git checkout feature-multicore

# Criar e mudar
git checkout -b feature-multicore

# Merge
git checkout main
git merge feature-multicore

# Deletar branch
git branch -d feature-multicore
```

---

## 🔧 Manutenção

### Limpeza

```bash
# Limpar builds
make clean

# Remover arquivos temporários
rm -f *.o *.out *.log

# Limpar cache do Git
git clean -fdx

# Encontrar arquivos grandes
du -h --max-depth=1 | sort -hr
```

---

### Backup

```bash
# Criar backup
tar -czf backup-$(date +%Y%m%d).tar.gz src/ docs/

# Extrair backup
tar -xzf backup-20251126.tar.gz

# Backup incremental
rsync -avz --progress src/ backup/src/
```

---

## 📊 Monitoramento

### Recursos do Sistema

```bash
# CPU usage
top
htop  # Se instalado

# Memória
free -h

# Disco
df -h

# Processos
ps aux | grep simulador

# Detalhes de processo
top -p $(pgrep simulador)
```

---

### Logs

```bash
# Ver log em tempo real
tail -f simulation.log

# Últimas 100 linhas
tail -n 100 simulation.log

# Buscar erros
grep "ERROR" simulation.log

# Contar erros por tipo
grep "ERROR" simulation.log | sort | uniq -c
```

---

## 🚀 Atalhos do Shell

### Bash Aliases

Adicione ao `~/.bashrc`:

```bash
# Aliases úteis
alias build='make clean && make -j$(nproc)'
alias buildrun='make clean && make -j$(nproc) && ./simulador'
alias test='make test'
alias valg='valgrind --leak-check=full'
alias gdb='gdb -tui'

# Recarregar .bashrc
alias reload='source ~/.bashrc'
```

Aplicar:
```bash
source ~/.bashrc
```

---

## 📝 Scripts Úteis

### Script de Build e Teste

```bash
#!/bin/bash
# build_and_test.sh

set -e

echo "=== Limpando ==="
make clean

echo "=== Compilando ==="
make -j$(nproc)

echo "=== Executando testes ==="
make test

echo "=== Verificando memory leaks ==="
valgrind --leak-check=summary --error-exitcode=1 ./simulador 2 10 > /dev/null

echo "=== Sucesso! ==="
```

---

### Script de Análise Rápida

```bash
#!/bin/bash
# quick_analysis.sh

./simulador > /dev/null
python3 analysis.py results.csv
xdg-open graphs/speedup.png
```

---

## 🔗 Recursos

- [Instalação do Ambiente](21-ambiente.md)
- [FAQ](20-faq.md)
- [Troubleshooting](23-troubleshooting.md)

---

**Dica:** Salve comandos frequentes em scripts ou aliases para economizar tempo!

**Última atualização:** Novembro 2025
