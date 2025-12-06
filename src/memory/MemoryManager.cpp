#include "MemoryManager.hpp"
#include <algorithm>

uint64_t MemoryManager::getUsedMainMemory() const {
    uint64_t used = 0;
    const auto& ram = mainMemory->getRam();
    for (size_t i = 0; i < ram.size(); ++i) {
        if (ram[i] != MEMORY_ACCESS_ERROR) ++used;
    }
    return used;
}

uint64_t MemoryManager::getUsedSecondaryMemory() const {
    uint64_t used = 0;
    const auto& storage = secondaryMemory->getStorage();
    for (size_t i = 0; i < storage.size(); ++i) {
        if (storage[i] != MEMORY_ACCESS_ERROR) ++used;
    }
    return used;
}

size_t MemoryManager::getSecondaryMemoryCapacity() const {
    return secondaryMemory->getStorage().size();
}
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

    // Cache L1 privada (thread_local, SEM LOCKS!)
    Cache* l1_cache = current_thread_cache;
    
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
        std::shared_lock<std::shared_mutex> lock(memory_mutex);
        
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
