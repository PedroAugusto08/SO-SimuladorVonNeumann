#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct DumpEntry {
    std::string policy;
    std::string reason;
    std::string timestamp;
    std::uint64_t cycles = 0;
    std::uint64_t max_cycles = 0;
    std::uint64_t finished = 0;
    std::uint64_t total = 0;
    std::uint64_t ready_queue = 0;
    std::uint64_t blocked = 0;
    std::uint64_t ready_count = 0;
    std::uint64_t idle_cores = 0;
    std::uint64_t context_switches = 0;
};

bool parse_header(const std::string& line, DumpEntry& entry) {
    if (line.empty() || line.front() != '[') {
        return false;
    }
    auto closing = line.find(']');
    if (closing == std::string::npos) {
        return false;
    }
    entry.policy = line.substr(1, closing - 1);

    auto reason_pos = line.find("reason=");
    if (reason_pos != std::string::npos) {
        auto space = line.find(' ', reason_pos);
        entry.reason = line.substr(reason_pos + 7, space - (reason_pos + 7));
    }

    auto cycles_pos = line.find("cycles=");
    if (cycles_pos != std::string::npos) {
        auto slash = line.find('/', cycles_pos);
        auto space = line.find(' ', cycles_pos);
        std::string current = line.substr(cycles_pos + 7, slash - (cycles_pos + 7));
        std::string maximum = line.substr(slash + 1, space - (slash + 1));
        entry.cycles = std::stoull(current);
        entry.max_cycles = std::stoull(maximum);
    }

    auto ts_pos = line.find("timestamp=");
    if (ts_pos != std::string::npos) {
        entry.timestamp = line.substr(ts_pos + 10);
        entry.timestamp.erase(entry.timestamp.find_last_not_of(" \t\r\n") + 1);
    }

    return true;
}

void parse_stats_line(const std::string& line, DumpEntry& entry) {
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        auto eq = token.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = token.substr(0, eq);
        std::string value = token.substr(eq + 1);
        auto slash = value.find('/');
        try {
            if (key == "finished" && slash != std::string::npos) {
                entry.finished = std::stoull(value.substr(0, slash));
                entry.total = std::stoull(value.substr(slash + 1));
            } else if (key == "ready_queue") {
                entry.ready_queue = std::stoull(value);
            } else if (key == "blocked") {
                entry.blocked = std::stoull(value);
            } else if (key == "ready_count") {
                entry.ready_count = std::stoull(value);
            } else if (key == "idle_cores") {
                entry.idle_cores = std::stoull(value);
            } else if (key == "context_switches") {
                entry.context_switches = std::stoull(value);
            }
        } catch (const std::exception&) {
            // Ignore malformed tokens
        }
    }
}

std::vector<DumpEntry> read_entries(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Não foi possível abrir " + path);
    }
    std::vector<DumpEntry> entries;
    std::string line;
    while (std::getline(in, line)) {
        DumpEntry entry;
        if (!parse_header(line, entry)) {
            continue;
        }
        std::string stats_line;
        if (std::getline(in, stats_line)) {
            parse_stats_line(stats_line, entry);
        }
        entries.push_back(std::move(entry));
    }
    return entries;
}

void print_entries(const std::vector<DumpEntry>& entries) {
    if (entries.empty()) {
        std::cout << "Nenhum dump encontrado.\n";
        return;
    }

    std::cout << std::left
              << std::setw(6) << "PID"
              << std::setw(20) << "Reason"
              << std::setw(14) << "Cycles"
              << std::setw(14) << "Finished"
              << std::setw(12) << "ReadyQ"
              << std::setw(12) << "Blocked"
              << std::setw(12) << "IdleCores"
              << "Timestamp" << '\n';
    std::cout << std::string(100, '-') << '\n';

    for (const auto& entry : entries) {
        std::ostringstream cycles_stream;
        cycles_stream << entry.cycles << '/' << entry.max_cycles;
        std::ostringstream finished_stream;
        finished_stream << entry.finished << '/' << entry.total;
        std::cout << std::left
                  << std::setw(6) << entry.policy
                  << std::setw(20) << entry.reason
                  << std::setw(14) << cycles_stream.str()
                  << std::setw(14) << finished_stream.str()
                  << std::setw(12) << entry.ready_queue
                  << std::setw(12) << entry.blocked
                  << std::setw(12) << entry.idle_cores
                  << entry.timestamp << '\n';
    }
}

int main(int argc, char** argv) {
    std::string log_path = "logs/scheduler_dumps.log";
    std::size_t tail = 5;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--log" || arg == "-l") && i + 1 < argc) {
            log_path = argv[++i];
        } else if ((arg == "--tail" || arg == "-t") && i + 1 < argc) {
            tail = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Uso: " << argv[0] << " [--log logs/scheduler_dumps.log] [--tail 5]\n";
            return 0;
        } else {
            std::cerr << "Argumento desconhecido: " << arg << '\n';
            return 1;
        }
    }

    std::vector<DumpEntry> entries;
    try {
        entries = read_entries(log_path);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    if (entries.size() > tail) {
        auto start = entries.begin() + static_cast<long>(entries.size() - tail);
        entries.erase(entries.begin(), start);
    }

    print_entries(entries);
    std::cout << "\nTotal de dumps analisados: " << entries.size()
              << " (arquivo: " << log_path << ")\n";
    return 0;
}
