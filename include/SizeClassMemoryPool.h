//
// Created by 30665 on 26-2-10.
//

#ifndef SIZECLASSMEMORYPOOL_H
#define SIZECLASSMEMORYPOOL_H

#include <cstddef>
#include <vector>
#include <memory>
#include "FixedMemoryPool.h"

class SizeClassMemoryPool {
    private:
    //大小等级定义
    static constexpr size_t MAX_SMALL_SIZE = 1024;
    static constexpr size_t ALIGNMENT = 8;

    //大小等级表
    std::vector<size_t> sizeClasses;

    //每个大小对应的内存池
    std::vector<std::unique_ptr<FixedMemoryPool>> pools;

    //统计信息
    struct SizeClassStats {
        size_t allocations;
        size_t deallocations;
        size_t failedAllocations;
        size_t totalAllocationBytes;
    };
    std::vector<SizeClassStats> stats;

    // 大对象统计
    struct LargeObjectStats {
        size_t allocations = 0;
        size_t deallocations = 0;
        size_t totalBytes = 0;
        size_t failedAllocations = 0;
    } largeStats_;

    //根据请求大小找到合适的大小等级
    size_t getSizeClass(size_t size) const;

    // 向上对齐到指定边界
    static size_t alignUp(size_t size, size_t alignment);

    //初始化大小等级
    void initializeSizeClasses();

    // 大对象分配/释放函数
    void* allocateLarge(size_t size);
    void deallocateLarge(void* ptr, size_t size);

    public:
    SizeClassMemoryPool();
    explicit SizeClassMemoryPool(size_t blocksPerClass);
    ~SizeClassMemoryPool();

    // 内存分配/释放
    void* allocate(size_t size);
    void deallocate(void* ptr,size_t size);

    // 获取统计信息
    void printStatistics() const;

    // 获取池信息
    size_t getNumSizeClasses() const {return sizeClasses.size();}
    size_t getSizeClassForSize(size_t size) const {return getSizeClass(size);}
    size_t getBlocksPerClass(size_t classIndex) const;

    //禁止拷贝
    SizeClassMemoryPool(const SizeClassMemoryPool&) = delete;
    SizeClassMemoryPool& operator=(const SizeClassMemoryPool&) = delete;

    void setAutoExpand(bool enable, size_t expandBlocks = 0) {
        for (auto& pool : pools) {
            pool->setAutoExpand(enable, expandBlocks);
        }
    }

};
#endif //SIZECLASSMEMORYPOOL_H
