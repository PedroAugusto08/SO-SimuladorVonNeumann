#pragma once
#include <string>
namespace Log {
    enum class Level { Error=0, Warn=1, Info=2, Debug=3 };
    void set_level(Level l);
    Level get_level();
    inline bool debug_enabled() { return get_level() == Level::Debug; }
    inline bool info_enabled() { return get_level() >= Level::Info; }
    void init_from_env();
}
