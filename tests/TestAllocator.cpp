//
// Created by 30665 on 26-2-21.
//
// tests/TestAllocator.cpp
#include <iostream>
#include "MemoryPoolAllocator.h"
#include "SizeClassMemoryPool.h"

int main() {
    std::cout << "Running tests...\n";
    SizeClassMemoryPool pool(100);
    MemoryPoolAllocator<int> alloc(pool);
    int* p = alloc.allocate(10);
    alloc.deallocate(p, 10);
    std::cout << "All tests passed!\n";
    return 0;
}