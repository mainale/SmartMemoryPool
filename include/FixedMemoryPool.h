//
// Created by 30665 on 26-2-7.
//

#ifndef FIXEDMEMORYPOOL_H
#define FIXEDMEMORYPOOL_H
#include <cstddef>
#include <mutex>
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


};
#endif //FIXEDMEMORYPOOL_H
