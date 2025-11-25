#ifndef CACHE_POLICY_HPP
#define CACHE_POLICY_HPP

#include "cache.hpp"
#include <queue>
#include <unordered_map>
#include <list>

enum class ReplacementPolicy {
    FIFO,  // First In First Out
    LRU    // Least Recently Used
};

class CachePolicy {
private:
    ReplacementPolicy policy;
    
    // Para LRU: tracking de acessos recentes
    std::list<size_t> lru_list;  // Lista de endereços (mais recente no início)
    std::unordered_map<size_t, std::list<size_t>::iterator> lru_map;  // Mapa para acesso rápido

public:
    CachePolicy(ReplacementPolicy p = ReplacementPolicy::FIFO);
    ~CachePolicy();

    // FIFO: Retorna o endereço a ser substituído
    size_t getAddressToReplace(std::queue<size_t>& fifo_queue);
    
    // LRU: Registra acesso a um endereço
    void access(size_t address);
    
    // LRU: Adiciona novo endereço ao tracking
    void add(size_t address);
    
    // LRU: Remove endereço do tracking
    void remove(size_t address);
    
    // LRU: Retorna o endereço menos recentemente usado
    size_t getLRUAddress();
    
    // Limpar estruturas
    void clear();
    
    // Obter política atual
    ReplacementPolicy getPolicy() const { return policy; }
    void setPolicy(ReplacementPolicy p) { policy = p; }
};

#endif