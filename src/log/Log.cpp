#include "log/Log.hpp"
#include <cstdlib>
#include <iostream>

static int LOG_LEVEL = 0;

void Log::init_from_env() {
    if (const char* lvl = std::getenv("SIM_LOG_LEVEL")) {
        try {
            LOG_LEVEL = std::stoi(lvl);
        } catch (...) {
            LOG_LEVEL = 0;
        }
    }
}

void Log::set_level(int level) {
    LOG_LEVEL = level;
}

int Log::get_level() {
    return LOG_LEVEL;
}
