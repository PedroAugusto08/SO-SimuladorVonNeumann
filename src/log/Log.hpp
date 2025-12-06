#ifndef LOG_HPP
#define LOG_HPP

#include <cstdlib>
#include <string>
#include <iostream>

class Log {
public:
    static void init_from_env() {
        const char* lvl = std::getenv("SIM_LOG_LEVEL");
        if (lvl) {
            std::cerr << "[Log] SIM_LOG_LEVEL=" << lvl << "\n";
        }
    }
};

#endif // LOG_HPP
