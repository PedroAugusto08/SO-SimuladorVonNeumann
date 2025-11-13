#include "src/cpu/Core.hpp"
#include "src/memory/MemoryManager.hpp"
int main() {
    MemoryManager mm(1024, 8192);
    Core c(0, &mm);
    return 0;
}
