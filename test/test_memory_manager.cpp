#include <iostream>
#include <memory>
#include <string>
#include "memory/MemoryManager.hpp"
#include "cpu/PCB.hpp"

int main() {
    try {
        const size_t MAIN_WORDS = 8; // 8 words = 32 bytes
        const size_t SECONDARY_WORDS = 8; // 8 words
        MemoryManager mem(MAIN_WORDS, SECONDARY_WORDS);

        PCB pcb;
        pcb.pid = 1;
        pcb.segment_base_addr = 0; // treat addresses as physical
        pcb.segment_limit = static_cast<uint32_t>((MAIN_WORDS + SECONDARY_WORDS) * 4);
        pcb.set_state(State::Ready);

        // Write to main memory: address 0
        uint32_t addr_main = 0u;
        uint32_t val_main = 0xDEADBEEFu;
        mem.write_raw(addr_main, val_main);
        uint32_t read_back_main = mem.read(addr_main, pcb);
        if (read_back_main != val_main) {
            std::cerr << "Fail: main memory read mismatch: expected " << std::hex << val_main
                      << ", got " << read_back_main << std::dec << "\n";
            return 1;
        }

        // Write to secondary memory: first word in secondary region
        uint32_t addr_secondary = static_cast<uint32_t>(MAIN_WORDS * 4 + 4);
        uint32_t val_sec = 0xBEEFCAFEu;
        mem.write_raw(addr_secondary, val_sec);
        uint32_t read_back_sec = mem.read(addr_secondary, pcb);
        if (read_back_sec != val_sec) {
            std::cerr << "Fail: secondary memory read mismatch: expected " << std::hex << val_sec
                      << ", got " << read_back_sec << std::dec << "\n";
            return 1;
        }

        // Write at last valid index (last word)
        uint32_t last_index_addr = static_cast<uint32_t>((MAIN_WORDS + SECONDARY_WORDS - 1) * 4);
        uint32_t last_val = 0xCAFEBABEu;
        mem.write_raw(last_index_addr, last_val);
        uint32_t last_read = mem.read(last_index_addr, pcb);
        if (last_read != last_val) {
            std::cerr << "Fail: last index read mismatch" << std::endl;
            return 1;
        }

        // Out-of-range write should throw
        bool threw = false;
        try {
            uint32_t oob_addr = static_cast<uint32_t>((MAIN_WORDS + SECONDARY_WORDS) * 4); // one past last
            mem.write_raw(oob_addr, 0x1234u);
        } catch (const std::out_of_range& e) {
            threw = true;
        }
        if (!threw) {
            std::cerr << "Fail: out-of-range write_raw did not throw" << std::endl;
            return 1;
        }

        std::cout << "Memory manager basic tests passed" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception in test: " << e.what() << std::endl;
        return 1;
    }
}
