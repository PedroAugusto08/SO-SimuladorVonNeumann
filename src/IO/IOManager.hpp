#ifndef IOMANAGER_HPP
#define IOMANAGER_HPP

#include "../cpu/PCB.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <fstream>
#include <chrono>
#include <random>

// Definição completa da estrutura IORequest
struct IORequest {
    std::string operation;
    std::string msg;
    PCB* process = nullptr; // Ponteiro para o PCB associado
    std::chrono::milliseconds cost_cycles;
};

class IOManager {
public:
    IOManager();
    ~IOManager();

    // Método para um processo se registrar como "esperando por I/O"
    void registerProcessWaitingForIO(PCB* process);
    bool is_idle() const;

private:
    void managerLoop();
    void addRequest(std::unique_ptr<IORequest> request);

    // Fila de requisições prontas para serem executadas
    std::vector<std::unique_ptr<IORequest>> requests;
    mutable std::mutex queueLock;

    // Fila de processos que estão no estado BLOCKED esperando por um dispositivo
    std::vector<PCB*> waiting_processes;
    mutable std::mutex waiting_processes_lock;

    // Variáveis booleanas representando o estado de cada dispositivo (0/1)
    bool printer_requesting;
    bool disk_requesting;
    bool network_requesting;
    // Alterna entre disco e impressora para evitar starvation.
    bool last_request_was_disk = false;
    mutable std::mutex device_state_lock;

    bool shutdown_flag;
    std::thread managerThread;

    std::ofstream resultFile;
    std::ofstream outputFile;

    std::mt19937 rng;
    std::uniform_int_distribution<int> printer_trigger_dist;
    std::uniform_int_distribution<int> disk_trigger_dist;
    std::uniform_int_distribution<int> cost_multiplier_dist;
};

#endif // IOMANAGER_HPP 