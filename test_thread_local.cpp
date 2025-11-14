#include <iostream>
#include <thread>
#include <vector>

thread_local int* my_ptr = nullptr;

void set_ptr(int* p) {
    std::cout << "SET: Thread " << std::this_thread::get_id() 
              << " ptr=" << (void*)p << std::endl;
    my_ptr = p;
    std::cout << "AFTER SET: my_ptr=" << (void*)my_ptr << std::endl;
}

int* get_ptr() {
    std::cout << "GET: Thread " << std::this_thread::get_id() 
              << " ptr=" << (void*)my_ptr << std::endl;
    return my_ptr;
}

void thread_func(int id) {
    int local_value = id * 100;
    std::cout << "\n=== Thread " << id << " START ===" << std::endl;
    
    set_ptr(&local_value);
    
    int* retrieved = get_ptr();
    
    if (retrieved == &local_value) {
        std::cout << "Thread " << id << ": ✅ SUCCESS! ptr matches" << std::endl;
    } else {
        std::cout << "Thread " << id << ": ❌ FAIL! ptr mismatch" << std::endl;
    }
}

int main() {
    std::cout << "Testing thread_local in native Linux..." << std::endl;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(thread_func, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nTest completed!" << std::endl;
    return 0;
}
