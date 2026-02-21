//
// Created by 30665 on 26-2-21.
//
#include "../include/SizeClassMemoryPool.h"
#include "../include/MemoryPoolAllocator.h"
#include <vector>
#include <iostream>

int main() {
    std::cout << "Example 2: std::vector using MemoryPoolAllocator\n";
    SizeClassMemoryPool pool(100);  // 每个大小类 100 块
    MemoryPoolAllocator<int> alloc(pool);

    std::vector<int, MemoryPoolAllocator<int>> vec(alloc);
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(i);
    }

    std::cout << "Vector size: " << vec.size() << "\n";
    pool.printStatistics();
    return 0;
}