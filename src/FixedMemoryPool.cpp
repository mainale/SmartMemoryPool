//
// Created by 30665 on 26-2-7.
//
#include <iostream>
#include <cstring>
#include <chrono>
#include "../include/FixedMemoryPool.h"
#include <thread>
#include <filesystem>


FixedMemoryPool::FixedMemoryPool(size_t blockSize, size_t numBlocks,bool verbose)
    :memory(nullptr),freeList(nullptr),blockSize(blockSize),numBlocks(numBlocks),verboseMode(verbose){
    std::cout << "Creating FixedMemoryPool" << std::endl;
    std::cout << "Block size: " << blockSize << " bytes" << std::endl;
    std::cout << "Number of blocks: " << numBlocks << std::endl;

    if (blockSize < sizeof(Block)) {
        this->blockSize = sizeof(Block);
        std::cout << "Adjusted block size to: " << this->blockSize << " bytes" << std::endl;
    }

    size_t totalSize = this -> blockSize * numBlocks;
    memory = new char[totalSize];
    memset(memory, 0, totalSize);


    freeList = reinterpret_cast<Block*>(memory);
    Block* current = freeList;
    for (size_t i = 0; i < numBlocks - 1; i++) {
        char* nextBlock = reinterpret_cast<char*>(current) + this->blockSize;
        current->next = reinterpret_cast<Block*>(nextBlock);
        current = current->next;
    }
    current->next = nullptr;
    std::cout << "Memory pool initialized with total memory: "
              << (totalSize / 1024.0) << " KB" << std::endl;

}

FixedMemoryPool::~FixedMemoryPool() {
    if (memory != nullptr) {
        delete[] memory;
        memory = nullptr;
        freeList = nullptr;
    }
}

void* FixedMemoryPool::allocate() {
    auto start = std::chrono::high_resolution_clock::now();
    if (!freeList) {
        stats.recordFailedAllocations();
        if (verboseMode) std::cout << "Memory pool is empty!" << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double,std::micro>(end - start).count();
        return nullptr;
    }
    Block* block = freeList;
    freeList = freeList->next;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double,std::micro>(end - start).count();
    stats.recordAllocations(blockSize,duration);


    return static_cast<void *>(block);
}

void FixedMemoryPool::deallocate(void* ptr) {
    auto start = std::chrono::high_resolution_clock::now();
    if (ptr == nullptr) {
        std::cerr << "Warning: Trying to deallocate nullptr" << std::endl;
        return;
    }
    Block* block = reinterpret_cast<Block*>(ptr);
    block->next = freeList;
    freeList = block;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double,std::micro>(end - start).count();
    stats.recordDeallocations(duration);
}

void *FixedMemoryPool::allocateThreadSafe() {
    std::lock_guard<std::mutex> lock(poolMutex);
    return allocate();
}

void FixedMemoryPool::deallocateThreadSafe(void *ptr) {
    std::lock_guard<std::mutex> lock(poolMutex);
    deallocate(ptr);
}

size_t FixedMemoryPool::getFreeBlocks() const {
    size_t count = 0;
    Block* current = freeList;
    while (current != nullptr) {
        count++;
        current = current->next;
    }
    return count;
}

