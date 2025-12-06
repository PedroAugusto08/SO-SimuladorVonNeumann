#ifndef IOMANAGER_HPP
#define IOMANAGER_HPP

#include "../cpu/PCB.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <fstream>
#include <chrono>

// Definição completa da estrutura IORequest
struct IORequest {
    std::string operation;
    std::string msg;
    PCB* process = nullptr; // Ponteiro para o PCB associado
    std::chrono::milliseconds cost_cycles;
    uint64_t remaining_cycles;
};

class IOManager {
public:
    IOManager();
    ~IOManager();

    // Método para um processo se registrar como "esperando por I/O"
    void registerProcessWaitingForIO(PCB* process);
<<<<<<< Updated upstream

private:
    void managerLoop();
    void addRequest(std::unique_ptr<IORequest> request);

    // Fila de requisições prontas para serem executadas
    std::vector<std::unique_ptr<IORequest>> requests;
    std::mutex queueLock;
=======
    bool is_idle() const;
    void do_work();

private:
    std::vector<std::unique_ptr<IORequest>> active_requests;
>>>>>>> Stashed changes

    // Fila de processos que estão no estado BLOCKED esperando por um dispositivo
    std::vector<PCB*> waiting_processes;
    std::mutex waiting_processes_lock;

    // Variáveis booleanas representando o estado de cada dispositivo (0/1)
    bool printer_requesting;
    bool disk_requesting;
    bool network_requesting;
    std::mutex device_state_lock;

    std::ofstream resultFile;
    std::ofstream outputFile;
};

#endif // IOMANAGER_HPP 