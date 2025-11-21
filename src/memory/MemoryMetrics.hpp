#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <mutex>

struct MemorySnapshot {
    uint64_t timestamp_ms;
    uint64_t used_main_memory;
    uint64_t used_secondary_memory;
    uint64_t cache_hits;
    uint64_t cache_misses;
};

class MemoryMetrics {
public:
    MemoryMetrics(const std::string& log_file);
    void record(uint64_t used_main, uint64_t used_secondary, uint64_t cache_hits, uint64_t cache_misses);
    void flush();
private:
    std::vector<MemorySnapshot> snapshots;
    std::string log_file;
    std::mutex mtx;
};
