//
// Created by 30665 on 26-2-7.
//

#ifndef FIXEDMEMORYPOOL_H
#define FIXEDMEMORYPOOL_H
#include <cstddef>
#include <mutex>
#include <vector>

#include "Statistics.h"
class FixedMemoryPool {
private:
    struct Block {
        Block* next;
    };
    char* memory;
    Block* freeList;
    size_t blockSize;
    size_t numBlocks;

    //是否开启错误显示
    bool verboseMode;

    //添加统计类
    Statistics stats;

    //添加互斥锁
    std::mutex poolMutex;

    // === 新增成员 ===
    bool autoExpandEnabled_;                 // 是否允许自动扩展
    size_t expandBlocks_;                     // 每次扩展的块数
    size_t totalChunks_;                       // 当前总块数（包括所有已分配块）
    size_t numExpansions_;                     // 扩展次数
    std::vector<char*> chunks_;                // 保存所有分配的大块内存（用于析构）

public:
    FixedMemoryPool(size_t blockSize, size_t numBlocks,bool verbose = false);
    ~FixedMemoryPool();

    //添加线程安全的分配/释放方法
    void* allocateThreadSafe();
    void deallocateThreadSafe(void* ptr);


    void* allocate();
    void deallocate(void* ptr);

    //添加统计相关方法
    const Statistics& getStatistics() const{return stats;}
    void printStatistics() const{ stats.printReport(numBlocks);}

    //获取池信息
    size_t getNumBlocks() const{ return numBlocks;}
    size_t getBlockSize() const{ return blockSize;}
    size_t getFreeBlocks() const;

    FixedMemoryPool(const FixedMemoryPool&) = delete;
    FixedMemoryPool& operator=(const FixedMemoryPool&) = delete;

    // 设置/获取详细模式
    void set_verbose(bool verbose) { verboseMode = verbose; }
    bool get_verbose() const { return verboseMode; }

    // === 新增接口 ===
    void setAutoExpand(bool enable, size_t expandBlocks = 0);
    bool autoExpandEnabled() const { return autoExpandEnabled_; }
    size_t getTotalChunks() const { return totalChunks_; }
    size_t getNumExpansions() const { return numExpansions_; }

private:
    bool expandPool();   // 执行扩展操作

    void* allocateImpl();               // 无锁实现
    bool expandPoolImpl();               // 无锁实现（调用者需持有锁）


};
#endif //FIXEDMEMORYPOOL_H
