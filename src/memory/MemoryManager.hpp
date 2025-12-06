#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <memory>
#include <stdexcept>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include "MAIN_MEMORY.hpp"
#include "SECONDARY_MEMORY.hpp"

const size_t MAIN_MEMORY_SIZE = 1024;

// Forward declarations
class Cache;
struct PCB;

/**
 * Estatísticas globais para análise de performance multicore
 */
struct MemoryStats {
    std::atomic<uint64_t> cache_hits{0};
    std::atomic<uint64_t> cache_misses{0};
    std::atomic<uint64_t> ram_accesses{0};
    std::atomic<uint64_t> disk_accesses{0};
    std::atomic<uint64_t> lock_contentions{0};
    std::atomic<uint64_t> total_lock_wait_ns{0};
    
    void reset() {
        cache_hits = 0;
        cache_misses = 0;
        ram_accesses = 0;
        disk_accesses = 0;
        lock_contentions = 0;
        total_lock_wait_ns = 0;
    }
    
    double get_cache_hit_rate() const {
        uint64_t total = cache_hits + cache_misses;
        return total > 0 ? (double)cache_hits / total * 100.0 : 0.0;
    }
    
    double get_avg_lock_wait_us() const {
        return lock_contentions > 0 ? (double)total_lock_wait_ns / lock_contentions / 1000.0 : 0.0;
    }
};

/**
 * MemoryManager - RAM/Disco compartilhados + Cache L1 privada thread_local
 */
class MemoryManager {
public:
    MemoryManager(size_t mainMemorySize, size_t secondaryMemorySize);

    uint64_t getUsedMainMemory() const;
    uint64_t getUsedSecondaryMemory() const;
    uint64_t getTotalCacheHits() const { return global_stats.cache_hits.load(); }
    uint64_t getTotalCacheMisses() const { return global_stats.cache_misses.load(); }
    size_t getMainMemoryCapacity() const { return mainMemoryLimit; }
    size_t getSecondaryMemoryCapacity() const;

    static void setThreadCache(Cache* l1_cache);
    static Cache* getThreadCache();

    uint32_t read(uint32_t address, PCB& process);
    void write(uint32_t address, uint32_t data, PCB& process);
    
    size_t getMainMemoryLimit() const { return mainMemoryLimit; }
    
    // Estatísticas globais
    static MemoryStats& getStats() { return global_stats; }
    static void resetStats() { global_stats.reset(); }

private:
    std::unique_ptr<MAIN_MEMORY> mainMemory;
    std::unique_ptr<SECONDARY_MEMORY> secondaryMemory;
    size_t mainMemoryLimit;
    
    static thread_local Cache* current_thread_cache;
    mutable std::shared_mutex memory_mutex;
    static MemoryStats global_stats;
    
    friend class Core;
};

#endif // MEMORY_MANAGER_HPP
