#pragma once

#include <chrono>
#include <cstdint>

namespace cpu_time {

inline uint64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline double ns_to_ms(uint64_t value_ns) {
    return static_cast<double>(value_ns) / 1'000'000.0;
}

inline double ns_to_seconds(uint64_t value_ns) {
    return static_cast<double>(value_ns) / 1'000'000'000.0;
}

inline double ns_to_ms(double value_ns) {
    return value_ns / 1'000'000.0;
}

inline double ns_to_seconds(double value_ns) {
    return value_ns / 1'000'000'000.0;
}

} // namespace cpu_time
