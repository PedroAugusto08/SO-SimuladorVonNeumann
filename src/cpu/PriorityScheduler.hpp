#pragma once
/**
 * PriorityScheduler - Versão PREEMPTIVA (por prioridade)
 * 
 * Escalonador baseado em prioridade estática com preempção
 * - Processos com MAIOR prioridade são executados primeiro
 * - PREEMPTIVO: processo pode ser interrompido quando chega processo de maior prioridade
 * - Prioridades: valores maiores = maior prioridade (0 = baixa, 10 = alta)
 * 
 * Características:
 * - Preempção por prioridade (se chegar processo mais importante)
 * - Não usa quantum de tempo (diferente do Round Robin)
 * - Salva contexto do processo preemptado
 * - Mantém fila ordenada por prioridade
 */

#include <vector>
#include <deque>
#include <memory>
#include <algorithm>
#include "PCB.hpp"
#include "Core.hpp"
#include "../IO/IOManager.hpp"
#include "memory/MemoryManager.hpp"

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

    PriorityScheduler(int num_cores, MemoryManager* memManager, IOManager* ioManager, int quantum = 1000);
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
    void check_preemption();  // Verifica se precisa preemptar processos
    bool should_preempt(PCB* running, PCB* waiting);  // Decide se preempta
    void preempt_process(Core* core, PCB* process);  // Preempta processo
    
    int num_cores;
    int quantum;  // Quantum de tempo (ciclos) - usado apenas para compatibilidade
    MemoryManager* memManager;
    IOManager* ioManager;
    std::vector<std::unique_ptr<Core>> cores;
    std::deque<PCB*> ready_queue;  // Ordenada por prioridade (maior primeiro)
    std::vector<PCB*> blocked_list;
    std::vector<PCB*> finished_list;
    int finished_count;
    int total_count;
    int context_switches;  // Contador de trocas de contexto
    uint64_t total_execution_time;
    uint64_t simulation_start_time;
};
