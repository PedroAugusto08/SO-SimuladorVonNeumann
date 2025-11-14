#include <iostream>
#include <fstream>
int main() {
    std::ofstream log("test_log.txt");
    log << "TESTE" << std::endl;
    log.flush();
    log.close();
    return 0;
}
