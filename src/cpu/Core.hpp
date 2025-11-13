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
     * Verifica se a thread de execução ainda está rodando
     * @return true se a thread está ativa
     */
    bool is_thread_running() const {
        return state.load() == CoreState::BUSY;
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
        return current_process; 
    }
    
    /**
     * Obtém o ID do núcleo
     * @return ID numérico do núcleo
     */
    int get_id() const { 
        return core_id; 
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
    std::mutex core_mutex;
    
    /**
     * Função executada pela thread - roda o processo
     * @param process Ponteiro para o PCB do processo
     */
    void run_process(PCB* process);
};

#endif // CORE_HPP
