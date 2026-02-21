//
// Created by 30665 on 26-2-21.
//
#include "../include/FixedMemoryPool.h"
#include <iostream>

int main() {
    std::cout << "Example 4: Auto-expand feature\n";
    FixedMemoryPool pool(32, 3, true);
    pool.setAutoExpand(true, 2);  // 每次扩展 2 块

    void* ptrs[10];
    for (int i = 0; i < 10; ++i) {
        ptrs[i] = pool.allocate();
        if (ptrs[i]) {
            std::cout << "Allocated block " << i << "\n";
        } else {
            std::cout << "Allocation failed at " << i << "\n";
            break;
        }
    }

    std::cout << "Total chunks after allocations: " << pool.getTotalChunks() << "\n";
    std::cout << "Number of expansions: " << pool.getNumExpansions() << "\n";

    for (void* p : ptrs) {
        pool.deallocate(p);
    }
    return 0;
}