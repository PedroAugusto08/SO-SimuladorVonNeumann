#ifndef CORE_HPP
#define CORE_HPP

#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include "CONTROL_UNIT.hpp"
#include "REGISTER_BANK.hpp"
#include "../memory/cache.hpp"

// Forward declarations
class MemoryManager;
struct PCB;
struct IORequest;

class Core {
public:
    /**
     * Construtor do núcleo
     * @param id Identificador único do núcleo
     * @param mem_manager Ponteiro para o gerenciador de memória compartilhada
     */
    Core(int id, MemoryManager* mem_manager);
    
    /**
     * Destrutor - aguarda término de threads
     */
    ~Core();
    
    /**
     * Executa um processo de forma assíncrona
     * @param process Ponteiro para o PCB do processo
     */
    void execute_async(PCB* process);
    
    /**
     * Verifica se o núcleo está ocioso
     * @return true se não está executando nenhum processo
     */
    bool is_idle() const { 
        return state.load() == CoreState::IDLE; 
    }
    
    /**
     * Verifica se o core pode receber um novo processo
     * @return true se está idle E não tem processo pendente de coleta
     */
    bool is_available_for_new_process() const {
        std::lock_guard<std::mutex> lock(core_mutex);
        return state.load() == CoreState::IDLE && current_process == nullptr;
    }
    
    /**
     * Verifica se a thread de execução ainda está rodando
     * @return true se a thread está ativa
     */
    bool is_thread_running() const {
        std::lock_guard<std::mutex> lock(core_mutex);
        return execution_thread.joinable();
    }
    
    /**
     * Aguarda a conclusão da execução atual
     */
    void wait_completion();
    
    /**
     * Obtém o processo atualmente em execução
     * @return Ponteiro para o PCB ou nullptr se idle
     */
    PCB* get_current_process() const { 
        std::lock_guard<std::mutex> lock(core_mutex);
        return current_process; 
    }
    
    /**
     * Limpa o ponteiro do processo atual (chamado após collect)
     */
    void clear_current_process() {
        std::lock_guard<std::mutex> lock(core_mutex);
        current_process = nullptr;
    }

    void request_stop();
    void join_thread();

    /**
     * Executa uma função protegida pelo mutex interno manipulando o ponteiro atual.
     * Útil para sincronizar coleta de processos com atualizações de métricas.
     */
    template <typename Func>
    void with_process_lock(Func&& func) {
        std::lock_guard<std::mutex> lock(core_mutex);
        func(current_process);
    }
    
    /**
     * Obtém o ID do núcleo
     * @return ID numérico do núcleo
     */
    int get_id() const { 
        return core_id; 
    }
    
    // 🆕 NOVOS MÉTODOS PARA RASTREAMENTO DE CICLOS
    uint64_t get_busy_cycles() const { return busy_cycles.load(); }
    uint64_t get_idle_cycles() const { return idle_cycles.load(); }
    uint64_t get_total_cycles() const { return busy_cycles.load() + idle_cycles.load(); }
    void increment_busy_cycles(uint64_t count = 1) { busy_cycles += count; }
    void increment_idle_cycles(uint64_t count = 1) { idle_cycles += count; }
    
    // ✅ CORREÇÃO 4: Reset de métricas entre execuções
    void reset_metrics() {
        busy_cycles.store(0);
        idle_cycles.store(0);
    }
    
private:
    // Estados possíveis do núcleo
    enum class CoreState {
        IDLE,       // Ocioso, pronto para receber processo
        BUSY,       // Executando um processo
        STOPPING    // Finalizando
    };
    
    // Identificação
    int core_id;
    std::atomic<CoreState> state{CoreState::IDLE};
    
    // Processo atual
    PCB* current_process{nullptr};
    
    // Memória compartilhada (gerenciada externamente)
    MemoryManager* memory_manager;
    
    // Cache L1 privada (cada núcleo tem a sua)
    std::unique_ptr<Cache> L1_cache;
    
    // Thread de execução
    std::thread execution_thread;
    mutable std::mutex core_mutex;  // mutable para permitir lock em métodos const
    std::atomic<bool> stop_requested{false};
    
    // 🆕 CONTADORES DE CICLOS
    std::atomic<uint64_t> busy_cycles{0};
    std::atomic<uint64_t> idle_cycles{0};
    
    /**
     * Função executada pela thread - roda o processo
     * @param process Ponteiro para o PCB do processo
     */
    void run_process(PCB* process);
};

#endif // CORE_HPP
