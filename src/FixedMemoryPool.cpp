//
// Created by 30665 on 26-2-7.
//
#include <iostream>
#include <cstring>
#include <chrono>
#include "../include/FixedMemoryPool.h"
#include <filesystem>


FixedMemoryPool::FixedMemoryPool(const size_t blockSize, const size_t numBlocks, const bool verbose)
    :memory(nullptr),freeList(nullptr),blockSize(blockSize),numBlocks(numBlocks),
    verboseMode(verbose), autoExpandEnabled_(false), expandBlocks_(0), totalChunks_(numBlocks),numExpansions_(0) {
    if(verboseMode){
        std::cout << "Creating FixedMemoryPool" << std::endl;
        std::cout << "Block size: " << blockSize << " bytes" << std::endl;
        std::cout << "Number of blocks: " << numBlocks << std::endl;
}

    if (blockSize < sizeof(Block)) {
        this->blockSize = sizeof(Block);
        std::cout << "Adjusted block size to: " << this->blockSize << " bytes" << std::endl;
    }

    size_t totalSize = this -> blockSize * numBlocks;
    memory = new char[totalSize];
    memset(memory, 0, totalSize);
    chunks_.push_back(memory);   // 记录初始内存块

    // 初始化空闲链表
    freeList = reinterpret_cast<Block*>(memory);
    Block* current = freeList;
    for (size_t i = 0; i < numBlocks - 1; i++) {
        char* nextBlock = reinterpret_cast<char*>(current) + this->blockSize;
        current->next = reinterpret_cast<Block*>(nextBlock);
        current = current->next;
    }
    current->next = nullptr;

    if (verboseMode) {
        std::cout << "Memory pool initialized with total memory: "
          << (totalSize / 1024.0) << " KB" << std::endl;
    }


}

FixedMemoryPool::~FixedMemoryPool() {
    // 释放所有通过 new char[] 分配的内存
    for (const char* chunk : chunks_) {
        delete[] chunk;
    }
    chunks_.clear();
    memory = nullptr;
    freeList = nullptr;

}

void* FixedMemoryPool::allocate() {
    auto start = std::chrono::high_resolution_clock::now();
    void* ptr = allocateImpl();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::micro>(end - start).count();
    if (ptr) {
        stats.recordAllocations(blockSize, duration);
    } else {
        stats.recordFailedAllocations();
        if (verboseMode) {
            std::cerr << "Allocation failed" << std::endl;
        }
    }
    return ptr;

}

void FixedMemoryPool::deallocate(void* ptr) {
    auto start = std::chrono::high_resolution_clock::now();
    if (ptr == nullptr) {
        std::cerr << "Warning: Trying to deallocate nullptr" << std::endl;
        return;
    }
    auto* block = static_cast<Block*>(ptr);
    block->next = freeList;
    freeList = block;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double,std::micro>(end - start).count();
    stats.recordDeallocations(duration);
}

void *FixedMemoryPool::allocateThreadSafe() {
    std::lock_guard<std::mutex> lock(poolMutex);
    auto start = std::chrono::high_resolution_clock::now();
    void* ptr = allocateImpl();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::micro>(end - start).count();
    if (ptr) {
        stats.recordAllocations(blockSize, duration);
    } else {
        stats.recordFailedAllocations();
        if (verboseMode) {
            std::cerr << "Allocation failed" << std::endl;
        }
    }
    return ptr;
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

void FixedMemoryPool::setAutoExpand(bool enable, size_t expandBlocks) {
    std::lock_guard<std::mutex> lock(poolMutex);
    autoExpandEnabled_ = enable;
    if (expandBlocks > 0) {
        expandBlocks_ = expandBlocks;
    }
    else {
        expandBlocks_ = numBlocks;
    }
}

bool FixedMemoryPool::expandPool() {
    // 注意：此函数应在已持有锁的情况下调用（或在 allocate 中已加锁）
    // 但 allocate 中未加锁（非线程安全版本），因此这里需要单独加锁？不，allocateThreadSafe 才会加锁。
    // 为了线程安全，我们可以在 allocateThreadSafe 中调用 expandPool，或在 expandPool 内部加锁。
    // 简单起见，我们先假设在 allocateThreadSafe 中调用，或在此处加锁。
    // 由于 allocate 本身不保证线程安全，如果用户使用 allocate 并启用了自动扩展，则可能出现竞态条件。
    // 这里我们选择在 expandPool 内部加锁，以确保多线程安全。
    std::lock_guard<std::mutex> lock(poolMutex);

    if (freeList != nullptr) {
        // 可能其他线程已经扩展过了，无需重复
        return true;
    }

    size_t expandSize = expandBlocks_;
    if (expandSize == 0) {
        expandSize = numBlocks;   // 默认扩展同等数量
    }

    size_t totalExpandBytes = expandSize * numBlocks;
    char* newMemory = nullptr;
    try {
        newMemory = new char[totalExpandBytes];
    } catch (const std::bad_alloc&) {
        return false;
    }

    memset(newMemory, 0, totalExpandBytes);
    chunks_.push_back(newMemory);

    // 将新内存块链接到空闲链表
    auto* newBlock = reinterpret_cast<Block*>(newMemory);
    Block* current = newBlock;
    for (size_t i = 0; i < expandSize - 1; i++) {
        char* next = reinterpret_cast<char*>(current) + blockSize;
        current->next = reinterpret_cast<Block*>(next);
        current = current->next;
    }
    current->next = freeList;   // 连接到原有空闲链表（可能为 nullptr）
    freeList = newBlock;
    totalChunks_ += expandSize;

    if (verboseMode) {
        std::cout << "Memory pool expanded by " << expandSize
                  << " blocks, total chunks now: " << totalChunks_ << std::endl;
    }
    return true;

}

void *FixedMemoryPool::allocateImpl() {
    if (!freeList) {
        if (autoExpandEnabled_) {
            if (!expandPoolImpl()) {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
    Block* block = freeList;
    freeList = freeList->next;
    return block;
}

bool FixedMemoryPool::expandPoolImpl() {
    // 假设已持有锁
    if (freeList != nullptr) {
        return true;   // 已经有空闲块，无需扩展
    }
    size_t expandSize = expandBlocks_ ? expandBlocks_ : numBlocks;
    char* newMemory = nullptr;
    try {
        newMemory = new char[expandSize * blockSize];
    } catch (...) {
        return false;
    }
    memset(newMemory, 0, expandSize * blockSize);
    chunks_.push_back(newMemory);

    auto* newBlock = reinterpret_cast<Block*>(newMemory);
    Block* current = newBlock;
    for (size_t i = 0; i < expandSize - 1; i++) {
        char* next = reinterpret_cast<char*>(current) + blockSize;
        current->next = reinterpret_cast<Block*>(next);
        current = current->next;
    }
    current->next = freeList;
    freeList = newBlock;

    totalChunks_ += expandSize;
    numExpansions_++;
    return true;
}


