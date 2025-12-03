#pragma once
/**
 * PriorityScheduler - Versão NÃO-PREEMPTIVA (por prioridade)
 * 
 * Escalonador baseado em prioridade estática SEM preempção
 * - Processos com MAIOR prioridade são executados primeiro
 * - NÃO-PREEMPTIVO: processo roda até terminar ou bloquear, sem interrupção
 * - Prioridades: valores maiores = maior prioridade (0 = baixa, 10 = alta)
 * 
 * Características:
 * - Sem preempção (processo completa antes de outro começar no mesmo core)
 * - Não usa quantum de tempo
 * - Mantém fila ordenada por prioridade
 * - Similar ao FCFS, mas ordenado por prioridade
 */

#include <vector>
#include <deque>
#include <memory>
#include <algorithm>
#include "PCB.hpp"
#include "Core.hpp"
#include "../IO/IOManager.hpp"
#include "memory/MemoryManager.hpp"
#include "Constants.hpp"

class PriorityScheduler {
public:
    struct Statistics {
        double avg_wait_time{0.0};
        double avg_turnaround_time{0.0};
        double avg_response_time{0.0};
        double avg_cpu_utilization{0.0};
        double throughput{0.0};
        int total_context_switches{0};
        int total_processes{0};
    };

    PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager);
    void add_process(PCB* process);
    void schedule_cycle();
    bool all_finished() const;
    bool has_pending_processes() const;
    int get_finished_count() const;
    int get_total_count() const { return total_count; }
    Statistics get_statistics() const;
    std::vector<std::unique_ptr<Core>>& get_cores();
    std::deque<PCB*>& get_ready_queue();
    std::vector<PCB*>& get_blocked_list();
    
private:
    void sort_by_priority();
    
    int num_cores;
    MemoryManager* memManager;
    IOManager* ioManager;
    std::vector<std::unique_ptr<Core>> cores;
    std::deque<PCB*> ready_queue;  // Ordenada por prioridade (maior primeiro)
    std::vector<PCB*> blocked_list;
    std::vector<PCB*> finished_list;
    std::atomic<int> finished_count{0};
    std::atomic<int> total_count{0};
    int context_switches;  // Contador de trocas de contexto
    uint64_t total_execution_time;
    std::chrono::steady_clock::time_point simulation_start_time;  // 🆕 Tempo real
    uint64_t total_simulation_cycles;  // 🆕 Total de ciclos da simulação
};
