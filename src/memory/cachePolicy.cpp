#include "cachePolicy.hpp"

CachePolicy::CachePolicy(ReplacementPolicy p) : policy(p) {}

CachePolicy::~CachePolicy() {
    clear();
}

// Implementação da política FIFO
size_t CachePolicy::getAddressToReplace(std::queue<size_t>& fifo_queue) {
    if (fifo_queue.empty()) {
        return static_cast<size_t>(-1); // Retorna -1 se não houver nada para remover
    }

    // Pega o primeiro endereço que entrou na fila
    size_t address_to_remove = fifo_queue.front();
    // Remove da fila
    fifo_queue.pop();

    return address_to_remove;
}

// ============= Implementação LRU =============

// Registra acesso a um endereço (move para o início da lista)
void CachePolicy::access(size_t address) {
    auto it = lru_map.find(address);
    
    if (it != lru_map.end()) {
        // Endereço já existe, move para o início (mais recente)
        lru_list.erase(it->second);
        lru_list.push_front(address);
        lru_map[address] = lru_list.begin();
    } else {
        // Endereço novo, adiciona
        add(address);
    }
}

// Adiciona novo endereço ao tracking LRU
void CachePolicy::add(size_t address) {
    // Remove se já existe (não deveria, mas por segurança)
    if (lru_map.find(address) != lru_map.end()) {
        remove(address);
    }
    
    // Adiciona no início (mais recente)
    lru_list.push_front(address);
    lru_map[address] = lru_list.begin();
}

// Remove endereço do tracking
void CachePolicy::remove(size_t address) {
    auto it = lru_map.find(address);
    if (it != lru_map.end()) {
        lru_list.erase(it->second);
        lru_map.erase(it);
    }
}

// Retorna o endereço menos recentemente usado (último da lista)
size_t CachePolicy::getLRUAddress() {
    if (lru_list.empty()) {
        return static_cast<size_t>(-1);
    }
    
    // O menos recentemente usado está no final da lista
    size_t lru_address = lru_list.back();
    
    // Remove do tracking (será substituído)
    lru_list.pop_back();
    lru_map.erase(lru_address);
    
    return lru_address;
}

// Limpar todas as estruturas
void CachePolicy::clear() {
    lru_list.clear();
    lru_map.clear();
}