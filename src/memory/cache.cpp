#include "cache.hpp"
#include "cachePolicy.hpp"
#include "MemoryManager.hpp" // Necessário para a lógica de write-back

Cache::Cache() {
    this->capacity = CACHE_CAPACITY;
    this->cacheMap.reserve(CACHE_CAPACITY);
    this->cache_misses = 0;
    this->cache_hits = 0;
}

Cache::Cache(size_t custom_capacity) {
    this->capacity = custom_capacity;
    this->cacheMap.reserve(custom_capacity);
    this->cache_misses = 0;
    this->cache_hits = 0;
}

Cache::~Cache() {
    this->cacheMap.clear();
}

size_t Cache::get(size_t address) {
    if (cacheMap.count(address) > 0 && cacheMap[address].isValid) {
        cache_hits++;
        return cacheMap[address].data; // Cache hit
    }

    cache_misses++;
    return CACHE_MISS; // Cache miss
}

void Cache::put(size_t address, size_t data, MemoryManager* memManager) {
    // Se a cache está cheia, precisamos remover um item
    if (cacheMap.size() >= capacity) {
        CachePolicy cachepolicy;
        // A política de remoção nos dirá qual endereço remover
        size_t addr_to_remove = cachepolicy.getAddressToReplace(fifo_queue);

        if (addr_to_remove != -1) {
            CacheEntry& entry_to_remove = cacheMap[addr_to_remove];

            // NOTE: Write-back desabilitado na arquitetura multicore
            // Cache L1 é privada, write-back seria feito para memória compartilhada
            // Por simplicidade, usamos write-through (dados escritos imediatamente)
            // Em arquitetura real, isso seria otimizado com protocolo de coerência
            
            // Remove da cache
            cacheMap.erase(addr_to_remove);
        }
    }

    // Adiciona o novo item na cache
    CacheEntry new_entry;
    new_entry.data = data;
    new_entry.isValid = true;
    new_entry.isDirty = false; // Começa como "limpo"

    cacheMap[address] = new_entry;
    fifo_queue.push(address); // Adiciona na fila do FIFO
}

void Cache::update(size_t address, size_t data) {
    // Se o item não está na cache, primeiro o colocamos lá
    if (cacheMap.find(address) == cacheMap.end()) {
        // Para a simplicidade, assumimos que o `put` deve ser chamado pelo `MemoryManager`
        // em um cache miss de escrita. Aqui, focamos em atualizar.
        // Em um sistema real, aqui ocorreria um "write-allocate".
        // Por ora, vamos apenas atualizar se existir.
        return;
    }
    
    cacheMap[address].data = data;
    cacheMap[address].isDirty = true; // Marca como sujo
    cacheMap[address].isValid = true;
}

void Cache::invalidate() {
    for (auto &c : cacheMap) {
        c.second.isValid = false;
    }
    // Limpar a fila FIFO também, pois a cache foi invalidada
    std::queue<size_t> empty;
    fifo_queue.swap(empty);
}

std::vector<std::pair<size_t, size_t>> Cache::dirtyData() {
    std::vector<std::pair<size_t, size_t>> dirty_data;
    for (const auto &c : cacheMap) {
        if (c.second.isDirty) {
            dirty_data.emplace_back(c.first, c.second.data);
        }
    }
    return dirty_data;
}

int Cache::get_misses(){
       // Retorna o número de cache misses
    return cache_misses;
}
int Cache::get_hits(){
       // Retorna o número de cache hits
    return cache_hits;
}