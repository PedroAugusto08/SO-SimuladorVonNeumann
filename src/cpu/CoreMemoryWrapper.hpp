#ifndef CORE_MEMORY_WRAPPER_HPP
#define CORE_MEMORY_WRAPPER_HPP

#include "../memory/MemoryManager.hpp"
#include "../memory/cache.hpp"
#include "PCB.hpp"

/**
 * CoreMemoryWrapper - Wrapper para MemoryManager que injeta cache L1 privada
 * 
 * Permite que CONTROL_UNIT use MemoryManager normalmente, mas automaticamente
 * passa a cache L1 privada do core para todas as operações.
 */
class CoreMemoryWrapper {
public:
    CoreMemoryWrapper(MemoryManager* mem_mgr, Cache* l1)
        : memory_manager(mem_mgr), l1_cache(l1) {}

    uint32_t read(uint32_t address, PCB& process) {
        return memory_manager->read(address, process, l1_cache);
    }

    void write(uint32_t address, uint32_t data, PCB& process) {
        memory_manager->write(address, data, process, l1_cache);
    }

    size_t getMainMemoryLimit() const {
        return memory_manager->getMainMemoryLimit();
    }

private:
    MemoryManager* memory_manager;
    Cache* l1_cache;
};

#endif // CORE_MEMORY_WRAPPER_HPP
