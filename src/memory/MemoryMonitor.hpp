#pragma once

#include <fstream>
#include <chrono>
#include <string>
#include <mutex>

/**
 * @brief Monitor de utilização de memória ao longo do tempo
 * 
 * Registra snapshots periódicos do uso de memória principal,
 * secundária e cache para análise de desempenho.
 */
class MemoryMonitor {
public:
    struct MemorySnapshot {
        uint64_t timestamp_ms;
        size_t main_memory_used_bytes;
        size_t secondary_memory_used_bytes;
        uint64_t cache_hits;
        uint64_t cache_misses;
        double hit_rate;
    };

    MemoryMonitor(const std::string& output_file = "logs/memory/memory_utilization.csv")
        : output_file(output_file), start_time(std::chrono::steady_clock::now()) {
        // Criar diretório se não existir
        system("mkdir -p logs/memory");
        
        // Abrir arquivo e escrever cabeçalho
        file.open(output_file);
        if (file.is_open()) {
            file << "timestamp_ms,main_memory_bytes,secondary_memory_bytes,cache_hits,cache_misses,hit_rate\n";
        }
    }

    ~MemoryMonitor() {
        if (file.is_open()) {
            file.close();
        }
    }

    void record_snapshot(size_t main_mem, size_t sec_mem, uint64_t hits, uint64_t misses) {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (!file.is_open()) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        
        double hit_rate = (hits + misses > 0) ? (hits * 100.0 / (hits + misses)) : 0.0;
        
        file << elapsed.count() << ","
             << main_mem << ","
             << sec_mem << ","
             << hits << ","
             << misses << ","
             << hit_rate << "\n";
        
        file.flush(); // Garantir que dados sejam escritos
    }

private:
    std::string output_file;
    std::ofstream file;
    std::chrono::steady_clock::time_point start_time;
    std::mutex mutex;
};
