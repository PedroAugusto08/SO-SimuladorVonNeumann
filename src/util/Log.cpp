#include "Log.hpp"
#include <cstdlib>
#include <atomic>
#include <string>
#include <algorithm>

static std::atomic<Log::Level> current_level{Log::Level::Warn};

namespace Log {
    void set_level(Level l) {
        current_level.store(l);
    }
    Level get_level() {
        return current_level.load();
    }
    void init_from_env() {
        const char* env = std::getenv("SIM_LOG_LEVEL");
        if (!env) return;
        std::string s(env);
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        if (s == "debug") set_level(Level::Debug);
        else if (s == "info") set_level(Level::Info);
        else if (s == "warn") set_level(Level::Warn);
        else set_level(Level::Warn);
    }
}
