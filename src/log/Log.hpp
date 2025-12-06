// Minimal logging shim used by some tests
#pragma once

#include <string>

class Log {
public:
    // Initialize logging level from environment variable SIM_LOG_LEVEL (optional)
    static void init_from_env();
    static void set_level(int level);
    static int get_level();
};
