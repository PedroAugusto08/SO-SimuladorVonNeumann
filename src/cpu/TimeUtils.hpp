#pragma once

#include <chrono>
#include <cstdint>
#include "Constants.hpp"

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

// Convert simulation cycles to nanoseconds and seconds using CLOCK_FREQ_HZ
inline uint64_t cycles_to_ns(uint64_t cycles) {
    return static_cast<uint64_t>((static_cast<double>(cycles) * 1e9) / CLOCK_FREQ_HZ);
}
inline double cycles_to_seconds(uint64_t cycles) {
    return static_cast<double>(cycles) / CLOCK_FREQ_HZ;
}

} // namespace cpu_time
