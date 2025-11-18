#include "MemoryManager.hpp"
#include "cache.hpp"
#include "../cpu/PCB.hpp"
#include <mutex>
#include <chrono>
#include <iostream>
#include <thread>
#include <fstream>
#include <cstdio>

// Definição da variável thread_local
thread_local Cache* MemoryManager::current_thread_cache = nullptr;

// Definição das estatísticas globais
MemoryStats MemoryManager::global_stats;

MemoryManager::MemoryManager(size_t mainMemorySize, size_t secondaryMemorySize) {
    mainMemory = std::make_unique<MAIN_MEMORY>(mainMemorySize);
    secondaryMemory = std::make_unique<SECONDARY_MEMORY>(secondaryMemorySize);
    mainMemoryLimit = mainMemorySize;
}

void MemoryManager::setThreadCache(Cache* l1_cache) {
    current_thread_cache = l1_cache;
}

Cache* MemoryManager::getThreadCache() {
    return current_thread_cache;
}

uint32_t MemoryManager::read(uint32_t address, PCB& process) {
    process.mem_accesses_total.fetch_add(1);
    process.mem_reads.fetch_add(1);

    // Acesso DIRETO à cache thread_local - SEM LOCKS!
    Cache* l1_cache = current_thread_cache;
    
    // DEBUG: CONTAR total de reads e com cache
    static std::atomic<int> total_reads{0};
    static std::atomic<int> reads_with_cache{0};
    int current_count = total_reads.fetch_add(1) + 1;
    if (l1_cache) reads_with_cache.fetch_add(1);
    
    if (current_count <= 10 || current_count % 50 == 0) {
        printf("[READ #%d] l1_cache=%p\n", current_count, (void*)l1_cache);
        fflush(stdout);
    }
    
    if (l1_cache) {
        size_t cache_data = l1_cache->get(address);
        if (cache_data != CACHE_MISS) {
            // Cache HIT - extremamente rápido!
            global_stats.cache_hits.fetch_add(1);
            process.cache_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.cache);
            contabiliza_cache(process, true);
            return cache_data;
        }
        
        // Cache MISS
        global_stats.cache_misses.fetch_add(1);
        contabiliza_cache(process, false);
    }

    // Lê da RAM/Disco (compartilhado, usa shared_lock)
    uint32_t data_from_mem;
    {
        // Medir tempo de espera no lock
        auto lock_start = std::chrono::high_resolution_clock::now();
        std::shared_lock<std::shared_mutex> lock(memory_mutex);
        auto lock_end = std::chrono::high_resolution_clock::now();
        
        auto wait_time = std::chrono::duration_cast<std::chrono::nanoseconds>(lock_end - lock_start).count();
        if (wait_time > 1000) { // > 1 microsegundo = contenção
            global_stats.lock_contentions.fetch_add(1);
            global_stats.total_lock_wait_ns.fetch_add(wait_time);
        }
        
        if (address < mainMemoryLimit) {
            global_stats.ram_accesses.fetch_add(1);
            process.primary_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.primary);
            data_from_mem = mainMemory->ReadMem(address);
        } else {
            global_stats.disk_accesses.fetch_add(1);
            process.secondary_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.secondary);
            uint32_t secondaryAddress = address - mainMemoryLimit;
            data_from_mem = secondaryMemory->ReadMem(secondaryAddress);
        }
    }

    // Armazena na cache L1 (sem locks!)
    if (l1_cache) {
        l1_cache->put(address, data_from_mem, nullptr);
    }

    return data_from_mem;
}

void MemoryManager::write(uint32_t address, uint32_t data, PCB& process) {
    process.mem_accesses_total.fetch_add(1);
    process.mem_writes.fetch_add(1);

    Cache* l1_cache = current_thread_cache;
    
    if (l1_cache) {
        size_t cache_data = l1_cache->get(address);

        if (cache_data == CACHE_MISS) {
            contabiliza_cache(process, false);
            
            // Write-allocate: carrega na cache primeiro
            uint32_t data_from_mem;
            {
                std::shared_lock<std::shared_mutex> lock(memory_mutex);
                
                if (address < mainMemoryLimit) {
                    process.primary_mem_accesses.fetch_add(1);
                    process.memory_cycles.fetch_add(process.memWeights.primary);
                    data_from_mem = mainMemory->ReadMem(address);
                } else {
                    process.secondary_mem_accesses.fetch_add(1);
                    process.memory_cycles.fetch_add(process.memWeights.secondary);
                    uint32_t secondaryAddress = address - mainMemoryLimit;
                    data_from_mem = secondaryMemory->ReadMem(secondaryAddress);
                }
            }
            
            l1_cache->put(address, data_from_mem, nullptr);
        } else {
            contabiliza_cache(process, true);
        }

        // Atualiza cache (sem locks!)
        l1_cache->update(address, data);
        process.cache_mem_accesses.fetch_add(1);
        process.memory_cycles.fetch_add(process.memWeights.cache);
        
    } else {
        // Sem cache, escreve direto na RAM/Disco
        std::unique_lock<std::shared_mutex> lock(memory_mutex);
        
        if (address < mainMemoryLimit) {
            mainMemory->WriteMem(address, data);
        } else {
            uint32_t secondaryAddress = address - mainMemoryLimit;
            secondaryMemory->WriteMem(secondaryAddress, data);
        }
    }
}
