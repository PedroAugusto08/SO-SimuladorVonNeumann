#ifndef CORE_MEMORY_ADAPTER_HPP
#define CORE_MEMORY_ADAPTER_HPP

#include <cstdint>
#include "../memory/cache.hpp"
#include "../memory/MemoryManager.hpp"
#include "PCB.hpp"

/**
 * CoreMemoryAdapter - Adaptador para acesso de memória com cache L1 privada
 * 
 * Cada Core tem seu próprio adapter com cache L1 privada.
 * Isso maximiza paralelismo pois apenas acessos à RAM/Disco são sincronizados.
 */
class CoreMemoryAdapter {
public:
    CoreMemoryAdapter(Cache* l1_cache, MemoryManager* memory_manager)
        : L1_cache(l1_cache), memory_manager(memory_manager) {}

    /**
     * Lê um dado da hierarquia de memória (Cache L1 → RAM → Disco)
     */
    uint32_t read(uint32_t address, PCB& process) {
        process.mem_accesses_total.fetch_add(1);
        process.mem_reads.fetch_add(1);

        // 1. Tenta ler da Cache L1 (privada, sem contenção!)
        size_t cache_data = L1_cache->get(address);
        if (cache_data != CACHE_MISS) {
            process.cache_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.cache);
            contabiliza_cache(process, true);
            return cache_data;
        }

        // 2. Cache Miss: busca na memória compartilhada
        contabiliza_cache(process, false);
        
        // Determina se é RAM ou Disco
        size_t mainMemoryLimit = memory_manager->getMainMemoryLimit();
        uint32_t data_from_mem;
        
        if (address < mainMemoryLimit) {
            process.primary_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.primary);
            data_from_mem = memory_manager->read(address);
        } else {
            process.secondary_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.secondary);
            data_from_mem = memory_manager->read(address);
        }

        // 3. Armazena na cache L1 privada
        L1_cache->put(address, data_from_mem, nullptr);  // nullptr = sem writeBack
        
        return data_from_mem;
    }

    /**
     * Escreve um dado na hierarquia de memória
     */
    void write(uint32_t address, uint32_t data, PCB& process) {
        process.mem_accesses_total.fetch_add(1);
        process.mem_writes.fetch_add(1);

        size_t cache_data = L1_cache->get(address);

        if (cache_data == CACHE_MISS) {
            contabiliza_cache(process, false);
            
            // Write-allocate: carrega na cache primeiro
            size_t mainMemoryLimit = memory_manager->getMainMemoryLimit();
            uint32_t data_from_mem;
            
            if (address < mainMemoryLimit) {
                process.primary_mem_accesses.fetch_add(1);
                process.memory_cycles.fetch_add(process.memWeights.primary);
                data_from_mem = memory_manager->read(address);
            } else {
                process.secondary_mem_accesses.fetch_add(1);
                process.memory_cycles.fetch_add(process.memWeights.secondary);
                data_from_mem = memory_manager->read(address);
            }
            
            L1_cache->put(address, data_from_mem, nullptr);
        } else {
            contabiliza_cache(process, true);
        }

        // Atualiza cache (marca como dirty)
        L1_cache->update(address, data);
        process.cache_mem_accesses.fetch_add(1);
        process.memory_cycles.fetch_add(process.memWeights.cache);
    }

    /**
     * Flush da cache - escreve dados dirty de volta para memória
     */
    void flush_cache() {
        // TODO: Implementar flush quando cache tiver dados dirty
        // Por enquanto, write-through é usado
    }

private:
    Cache* L1_cache;  // Cache L1 privada do core (não owned)
    MemoryManager* memory_manager;  // RAM/Disco compartilhado
};

#endif // CORE_MEMORY_ADAPTER_HPP
