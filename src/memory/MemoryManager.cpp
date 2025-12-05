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
#include <sstream>

// Definição da variável thread_local
thread_local Cache* MemoryManager::current_thread_cache = nullptr;

// Definição das estatísticas globais
MemoryStats MemoryManager::global_stats;

namespace {
uint32_t translate_address_or_throw(uint32_t requested_address, PCB& process) {
    const uint32_t limit = process.segment_limit;
    const uint32_t base = process.segment_base_addr;
    if (limit == 0) {
        std::ostringstream oss;
        oss << "Segmentation Fault: Processo P" << process.pid
            << " sem segmento inicializado";
        throw std::runtime_error(oss.str());
    }
    const uint64_t segment_end = static_cast<uint64_t>(base) + limit;

    // Caso 1: endereço fornecido é um offset virtual
    if (requested_address < limit) {
        return base + requested_address;
    }

    // Caso 2: endereço já está no espaço físico do segmento
    if (requested_address >= base && requested_address < segment_end) {
        return requested_address;
    }

    std::ostringstream oss;
    oss << "Segmentation Fault: Processo P" << process.pid
        << " acessou endereco " << requested_address
        << " fora do segmento [" << base << ", " << segment_end << ")";
    throw std::runtime_error(oss.str());
}
}

MemoryManager::MemoryManager(size_t mainMemorySize, size_t secondaryMemorySize) {
    mainMemory = std::make_unique<MAIN_MEMORY>(mainMemorySize);
    secondaryMemory = std::make_unique<SECONDARY_MEMORY>(secondaryMemorySize);
    mainMemoryLimit = mainMemorySize;
}

void MemoryManager::reset_stats() {
    global_stats.reset();
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
    const uint32_t physical_address = translate_address_or_throw(address, process);

    // Cache L1 privada (thread_local, SEM LOCKS!)
    Cache* l1_cache = current_thread_cache;
    
    if (l1_cache) {
        size_t cache_data = l1_cache->get(physical_address);
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
        
        if (physical_address < mainMemoryLimit) {
            global_stats.ram_accesses.fetch_add(1);
            process.primary_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.primary);
            data_from_mem = mainMemory->ReadMem(physical_address);
        } else {
            global_stats.disk_accesses.fetch_add(1);
            process.secondary_mem_accesses.fetch_add(1);
            process.memory_cycles.fetch_add(process.memWeights.secondary);
            uint32_t secondaryAddress = physical_address - mainMemoryLimit;
            data_from_mem = secondaryMemory->ReadMem(secondaryAddress);
        }
    }

    // Armazena na cache L1 (sem locks!)
    if (l1_cache) {
        l1_cache->put(physical_address, data_from_mem, nullptr);
    }

    return data_from_mem;
}

void MemoryManager::write(uint32_t address, uint32_t data, PCB& process) {
    process.mem_accesses_total.fetch_add(1);
    process.mem_writes.fetch_add(1);
    const uint32_t physical_address = translate_address_or_throw(address, process);

    Cache* l1_cache = current_thread_cache;
    
    if (l1_cache) {
        size_t cache_data = l1_cache->get(physical_address);

        if (cache_data == CACHE_MISS) {
            contabiliza_cache(process, false);
            
            // Write-allocate: carrega na cache primeiro
            uint32_t data_from_mem;
            {
                std::shared_lock<std::shared_mutex> lock(memory_mutex);
                
                if (physical_address < mainMemoryLimit) {
                    process.primary_mem_accesses.fetch_add(1);
                    process.memory_cycles.fetch_add(process.memWeights.primary);
                    data_from_mem = mainMemory->ReadMem(physical_address);
                } else {
                    process.secondary_mem_accesses.fetch_add(1);
                    process.memory_cycles.fetch_add(process.memWeights.secondary);
                    uint32_t secondaryAddress = physical_address - mainMemoryLimit;
                    data_from_mem = secondaryMemory->ReadMem(secondaryAddress);
                }
            }
            
            l1_cache->put(physical_address, data_from_mem, nullptr);
        } else {
            contabiliza_cache(process, true);
        }

        // Atualiza cache (sem locks!)
        l1_cache->update(physical_address, data);
        process.cache_mem_accesses.fetch_add(1);
        process.memory_cycles.fetch_add(process.memWeights.cache);
        
    } else {
        // Sem cache, escreve direto na RAM/Disco
        std::unique_lock<std::shared_mutex> lock(memory_mutex);
        
        if (physical_address < mainMemoryLimit) {
            mainMemory->WriteMem(physical_address, data);
        } else {
            uint32_t secondaryAddress = physical_address - mainMemoryLimit;
            secondaryMemory->WriteMem(secondaryAddress, data);
        }
    }
}
