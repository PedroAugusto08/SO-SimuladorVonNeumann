# Bibliografia e Referências

## Livros

### Sistemas Operacionais

1. **Operating System Concepts** (Silberschatz, Galvin, Gagne)
   - 10ª Edição, 2018
   - Capítulos relevantes: 5-7 (Scheduling), 8-9 (Memory)
   - ISBN: 978-1119320913

2. **Modern Operating Systems** (Andrew Tanenbaum)
   - 4ª Edição, 2014
   - Referência para algoritmos de escalonamento
   - ISBN: 978-0133591620

3. **Operating Systems: Three Easy Pieces** (Remzi Arpaci-Dusseau)
   - Disponível gratuitamente: https://pages.cs.wisc.edu/~remzi/OSTEP/
   - Excelente para conceitos de virtualização e concorrência

### Arquitetura de Computadores

4. **Computer Organization and Design** (Patterson, Hennessy)
   - 5ª Edição, 2014
   - Arquitetura Von Neumann, cache, pipelines
   - ISBN: 978-0124077263

5. **Computer Architecture: A Quantitative Approach** (Hennessy, Patterson)
   - 6ª Edição, 2019
   - Referência para hierarquia de memória
   - ISBN: 978-0128119051

## Artigos Acadêmicos

### Escalonamento

- **Multilevel Feedback Queue Scheduling** - Corbato et al., 1962
  - Origem do escalonamento multi-nível

- **A Starvation-Free Scheduling Algorithm** - Various
  - Técnicas de aging e prevenção de starvation

### Cache

- **Working Set Model** - Denning, 1968
  - Conceito de working set e localidade

- **LRU Implementation** - Various
  - Implementações eficientes de LRU

## Documentação Online

### C++ e Concorrência

- [cppreference.com](https://en.cppreference.com/)
  - Documentação completa de C++17
  - `std::thread`, `std::mutex`, `std::atomic`

- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action)
  - Anthony Williams, 2019
  - Concorrência moderna em C++

### JSON

- [nlohmann/json](https://github.com/nlohmann/json)
  - Biblioteca JSON usada no projeto
  - Documentação completa

## Recursos Adicionais

### Simuladores Relacionados

- **MARS (MIPS Assembler and Runtime Simulator)**
  - Inspiração para interface e funcionalidades

- **SimOS**
  - Simulador de sistema operacional completo

### Ferramentas de Desenvolvimento

- **GDB** - https://www.gnu.org/software/gdb/
- **Valgrind** - https://valgrind.org/
- **CMake** - https://cmake.org/documentation/

## Referências do Código

### Estruturas de Dados

```cpp
// PCB - Process Control Block
// Baseado em: Silberschatz, cap. 3

// Cache com política LRU
// Baseado em: Hennessy & Patterson, cap. 5

// Round Robin com quantum
// Baseado em: Tanenbaum, cap. 2
```

### Algoritmos Implementados

| Algoritmo | Referência |
|-----------|------------|
| FCFS | Silberschatz, cap. 5.3.1 |
| SJN/SJF | Silberschatz, cap. 5.3.2 |
| Round Robin | Silberschatz, cap. 5.3.4 |
| Priority | Silberschatz, cap. 5.3.3 |
| LRU Cache | Patterson, cap. 5.4 |
| FIFO Cache | Patterson, cap. 5.4 |

## Citações

### Formato ABNT

SILBERSCHATZ, Abraham; GALVIN, Peter Baer; GAGNE, Greg. **Fundamentos de Sistemas Operacionais**. 9. ed. Rio de Janeiro: LTC, 2015.

TANENBAUM, Andrew S.; BOS, Herbert. **Sistemas Operacionais Modernos**. 4. ed. São Paulo: Pearson, 2016.

PATTERSON, David A.; HENNESSY, John L. **Organização e Projeto de Computadores**. 5. ed. Rio de Janeiro: Elsevier, 2017.

### Formato IEEE

[1] A. Silberschatz, P. B. Galvin, and G. Gagne, *Operating System Concepts*, 10th ed. Hoboken, NJ, USA: Wiley, 2018.

[2] A. S. Tanenbaum and H. Bos, *Modern Operating Systems*, 4th ed. Upper Saddle River, NJ, USA: Pearson, 2014.

[3] D. A. Patterson and J. L. Hennessy, *Computer Organization and Design*, 5th ed. Waltham, MA, USA: Morgan Kaufmann, 2014.
