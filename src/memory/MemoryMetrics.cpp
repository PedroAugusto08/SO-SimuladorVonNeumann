#include "MemoryMetrics.hpp"
#include <chrono>

MemoryMetrics::MemoryMetrics(const std::string& log_file)
    : log_file(log_file) {}

void MemoryMetrics::record(uint64_t used_main, uint64_t used_secondary, uint64_t cache_hits, uint64_t cache_misses) {
    std::lock_guard<std::mutex> lock(mtx);
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    snapshots.push_back({timestamp, used_main, used_secondary, cache_hits, cache_misses});
}

void MemoryMetrics::flush() {
    std::lock_guard<std::mutex> lock(mtx);
    std::ofstream out(log_file);
    out << "timestamp_ms,used_main_memory,used_secondary_memory,cache_hits,cache_misses\n";
    for (const auto& snap : snapshots) {
        out << snap.timestamp_ms << "," << snap.used_main_memory << "," << snap.used_secondary_memory << "," << snap.cache_hits << "," << snap.cache_misses << "\n";
    }
    out.close();
}
