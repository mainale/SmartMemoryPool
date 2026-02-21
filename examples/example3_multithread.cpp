//
// Created by 30665 on 26-2-21.
//
#include "../include/GlobalMemoryPool.h"
#include <thread>
#include <vector>
#include <iostream>

void worker(int id) {
    auto& global = GlobalMemoryPool::get_instance();
    auto& pool = global.get_pool();
    for (int i = 0; i < 500; ++i) {
        void* p = pool.allocate(64);
        if (p) {
            // 模拟使用
            int* data = static_cast<int*>(p);
            *data = id;
            pool.deallocate(p, 64);
        }
    }
}

int main() {
    std::cout << "Example 3: Multithreaded usage with global pool\n";
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    GlobalMemoryPool::get_instance().print_statistics();
    return 0;
}