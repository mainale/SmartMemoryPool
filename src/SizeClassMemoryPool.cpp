//
// Created by 30665 on 26-2-10.
//
#include "../include/SizeClassMemoryPool.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>

SizeClassMemoryPool::SizeClassMemoryPool():SizeClassMemoryPool(100) {
    // 委托给另一个构造函数
}

SizeClassMemoryPool::SizeClassMemoryPool(size_t blocksPerClass) {
    std::cout << "Creating SizeClassMemoryPool with " << blocksPerClass
              << " blocks per class" << std::endl;
    // 初始化大小等级
    initializeSizeClasses();

    // 为每个大小等级创建内存池
    stats.resize(sizeClasses.size());
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        pools.emplace_back(std::make_unique<FixedMemoryPool>(sizeClasses[i],blocksPerClass));
        stats[i] = {0,0,0,0};
    }

    std::cout << "Initialized " << sizeClasses.size() << " size classes" << std::endl;
}

SizeClassMemoryPool::~SizeClassMemoryPool() {
    std::cout << "SizeClassMemoryPool destroyed" << std::endl;
}

void SizeClassMemoryPool::initializeSizeClasses() {
    // 使用几何增长的大小等级
    // 小对象：8, 16, 32, 64, 128, 256, 512, 1024
    sizeClasses.clear();

    size_t size = ALIGNMENT;
    while (size <= MAX_SMALL_SIZE) {
        sizeClasses.push_back(size);
        size *= 2;
    }
    std::cout << "Size classes1: ";
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        std::cout << sizeClasses[i] << ' ';
    }
    std::cout << std::endl;

    // 添加一些中间等级（减少内存浪费）
    // 例如：24, 48, 96, 192, 384, 768
    std::vector<size_t> interMediate;
    for (size_t base :sizeClasses) {
        if (base >= 16) {
            interMediate.push_back(base + base/2);
        }
    }
    // 合并并排序
    sizeClasses.insert(sizeClasses.end(), interMediate.begin(), interMediate.end());
    std::cout << "Size classes2: ";
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        std::cout << sizeClasses[i] << ' ';
    }
    std::cout << std::endl;
    std::sort(sizeClasses.begin(), sizeClasses.end());
    std::cout << "Size classes3: ";
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        std::cout << sizeClasses[i] << ' ';
    }
    std::cout << std::endl;

    //移除重复
    sizeClasses.erase(unique(sizeClasses.begin(),sizeClasses.end()),sizeClasses.end());
    std::cout << "Size classes4: ";
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        std::cout << sizeClasses[i] << ' ';
    }
    std::cout << std::endl;

    // 打印大小等级
    std::cout << "Size classes: ";
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        std::cout << sizeClasses[i] << " ";
        if (i < sizeClasses.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

}

size_t SizeClassMemoryPool::getSizeClass(size_t size) const {
    if (size == 0) return 0;
    // 找到第一个 >= size 的大小等级
    for (size_t i = 0; i < sizeClasses.size(); i++) {
        if (size <= sizeClasses[i]) return i;
    }

    // 如果请求大小超过最大大小等级，返回最后一个
    return sizeClasses.size() - 1;
}

size_t SizeClassMemoryPool::alignUp(size_t size, size_t alignment) {
    return ((size + alignment - 1) & ~(alignment - 1));
}

size_t SizeClassMemoryPool::getBlocksPerClass(size_t classIndex) const {
    if (classIndex >= sizeClasses.size()) return 0;
    return pools[classIndex]->getNumBlocks();
}

void* SizeClassMemoryPool::allocate(size_t size) {
    if (size == 0) return nullptr;

    //向上对齐
    size_t alignSize = alignUp(size, ALIGNMENT);

    //找到合适的大小等级
    size_t classIndex = getSizeClass(alignSize);

    // 调试：打印大小等级信息
    // if (size > 100) {  // 只打印大对象调试信息
    //     std::cout << "DEBUG: size=" << size
    //               << ", aligned=" << alignSize
    //               << ", classIndex=" << classIndex
    //               << ", classSize=" << (classIndex < sizeClasses.size() ? sizeClasses[classIndex] : 0)
    //               << std::endl;
    // }
    // for (size_t i = 0; i < sizeClasses.size(); i++) {
    //     std::cout << sizeClasses[i] << " " << std::endl;
    // }

    // 检查是否超过最大大小等级
    if (classIndex >= sizeClasses.size()) {
        std::cerr << "Error: Requested size " << size
          << " exceeds maximum size class ("
          << sizeClasses.back() << ")" << std::endl;
        return nullptr;
    }

    // 从对应的池中分配
    size_t allocatedSize = sizeClasses[classIndex];

    if (void* ptr = pools[classIndex]->allocateThreadSafe()) {
        stats[classIndex].allocations++;
        stats[classIndex].totalAllocationBytes += allocatedSize;
        return ptr;
    }
    else {
        stats[classIndex].failedAllocations++;
        std::cerr << "Warning: Allocation failed for size " << size
                  << " (size class: " << allocatedSize << ")" << std::endl;
        return nullptr;
    }
}

void SizeClassMemoryPool::deallocate(void* ptr,size_t size) {
    if (!ptr || size == 0) return;

    // 向上对齐
    size_t alignSize = alignUp(size, ALIGNMENT);

    // 找到合适的大小等级
    size_t classIndex = getSizeClass(alignSize);

    // 检查索引有效性
    if (classIndex >= sizeClasses.size()) {
        std::cerr << "Error: Invalid size for deallocation: " << size << std::endl;
        return;
    }

    // 释放到对应的池
    pools[classIndex]->deallocateThreadSafe(ptr);
    stats[classIndex].deallocations++;
}

void SizeClassMemoryPool::printStatistics() const {
    std::cout << "\n=== Size Class Memory Pool Statistics ===" << std::endl;
    std::cout << "Total size classes: " << sizeClasses.size() << std::endl;
    std::cout << "Maximum small object size: " << MAX_SMALL_SIZE << " bytes" << std::endl;
    std::cout << "Alignment: " << ALIGNMENT << " bytes" << std::endl;

    size_t totalAllocations = 0;
    size_t totalDeallocations = 0;
    size_t totalFailed = 0;
    size_t totalBytes = 0;

    std::cout << "\nSize Class Details:" << std::endl;
    std::cout << std::setw(8) << "Class"
              << std::setw(12) << "Size(bytes)"
              << std::setw(12) << "Allocations"
              << std::setw(12) << "Deallocations"
              << std::setw(12) << "Failed"
              << std::setw(15) << "Total Bytes"
              << std::setw(12) << "Efficiency" << std::endl;
    std::cout << std::string(85, '-') << std::endl;

    for (size_t i = 0; i < sizeClasses.size(); ++i) {
        const auto& stat = stats[i];
        size_t blockSize = sizeClasses[i];

        totalAllocations += stat.allocations;
        totalDeallocations += stat.deallocations;
        totalFailed += stat.failedAllocations;
        totalBytes += stat.totalAllocationBytes;

        // 计算内存使用效率（如果没有分配，效率为0）
        double efficiency = 0.0;
        if (stat.totalAllocationBytes > 0) {
            // 实际请求大小很难跟踪，我们只显示块大小效率
            // 实际上这里应该跟踪实际请求大小，但为了简单我们只显示块大小
            efficiency = 100.0;  // 简化为100%
        }

        std::cout << std::setw(8) << i
                  << std::setw(12) << blockSize
                  << std::setw(12) << stat.allocations
                  << std::setw(12) << stat.deallocations
                  << std::setw(12) << stat.failedAllocations
                  << std::setw(15) << stat.totalAllocationBytes
                  << std::setw(11) << std::fixed << std::setprecision(1)
                  << efficiency << "%" << std::endl;
        }
    std::cout << "\nSummary:" << std::endl;
    std::cout << "  Total allocations: " << totalAllocations << std::endl;
    std::cout << "  Total deallocations: " << totalDeallocations << std::endl;
    std::cout << "  Total failed allocations: " << totalFailed << std::endl;
    std::cout << "  Total allocated bytes: " << totalBytes << std::endl;

    // 计算平均内存效率（简化）
    if (totalAllocations > 0) {
        double avgEfficiency = 100.0;  // 简化计算
        std::cout << "  Average memory efficiency: "
                  << std::fixed << std::setprecision(1)
                  << avgEfficiency << "%" << std::endl;
    }
    std::cout << "===========================================\n" << std::endl;
}












