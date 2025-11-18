#ifndef PCB_HPP
#define PCB_HPP
/*
  PCB.hpp
  Definição do bloco de controle de processo (PCB) usado pelo simulador da CPU.
  Centraliza: identificação do processo, prioridade, quantum, pesos de memória e
  contadores de instrumentação de pipeline/memória.
*/
#include <string>
#include <atomic>
#include <cstdint>
#include "memory/cache.hpp"
#include "REGISTER_BANK.hpp" // necessidade de objeto completo dentro do PCB


// Estados possíveis do processo (simplificado)
enum class State {
    Ready,
    Running,
    Blocked,
    Finished
};

struct MemWeights {
    uint64_t cache = 1;   // custo por acesso à memória cache
    uint64_t primary = 5; // custo por acesso à memória primária
    uint64_t secondary = 10; // custo por acesso à memória secundária
};

struct PCB {
    int pid = 0;
    std::string name;
    int quantum = 0;
    int priority = 0;

    State state = State::Ready;
    hw::REGISTER_BANK regBank;

    // Contadores de acesso à memória
    std::atomic<uint64_t> primary_mem_accesses{0};
    std::atomic<uint64_t> secondary_mem_accesses{0};
    std::atomic<uint64_t> memory_cycles{0};
    std::atomic<uint64_t> mem_accesses_total{0};
    std::atomic<uint64_t> extra_cycles{0};
    std::atomic<uint64_t> cache_mem_accesses{0};

    // Instrumentação detalhada
    std::atomic<uint64_t> pipeline_cycles{0};
    std::atomic<uint64_t> stage_invocations{0};
    std::atomic<uint64_t> mem_reads{0};
    std::atomic<uint64_t> mem_writes{0};

    // Novos contadores
    std::atomic<uint64_t> cache_hits{0};
    std::atomic<uint64_t> cache_misses{0};
    std::atomic<uint64_t> io_cycles{1};

    // Métricas de escalonamento (para Round Robin multicore)
    std::atomic<uint64_t> arrival_time{0};      // Quando entrou no sistema
    std::atomic<uint64_t> start_time{0};        // Primeira execução
    std::atomic<uint64_t> finish_time{0};       // Quando terminou
    std::atomic<uint64_t> total_wait_time{0};   // Tempo total em espera
    std::atomic<uint64_t> context_switches{0};  // Número de trocas de contexto
    std::atomic<int> assigned_core{-1};         // Núcleo atual (-1 = nenhum)
    std::atomic<int> last_core{-1};             // Último núcleo usado

    // Informações do programa carregado
    uint32_t program_start_addr = 0;             // Endereço de início do programa
    uint32_t program_size = 0;                   // Tamanho do programa em bytes

    MemWeights memWeights;
    
    // Funções auxiliares para cálculo de métricas
    uint64_t get_turnaround_time() const {
        if (finish_time == 0) return 0;
        return finish_time.load() - arrival_time.load();
    }
    
    uint64_t get_wait_time() const {
        return total_wait_time.load();
    }
    
    double get_cache_hit_rate() const {
        uint64_t hits = cache_hits.load();
        uint64_t misses = cache_misses.load();
        if (hits + misses == 0) return 0.0;
        return (double)hits / (hits + misses);
    }
};

// Contabilizar cache
inline void contabiliza_cache(PCB &pcb, bool hit) {
    if (hit) {
        pcb.cache_hits++;
    } else {
        pcb.cache_misses++;
    }
}

#endif // PCB_HPP