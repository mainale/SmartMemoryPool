//
// Created by 30665 on 26-2-7.
//

#ifndef FIXEDMEMORYPOOL_H
#define FIXEDMEMORYPOOL_H
#include <cstddef>
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

    Statistics stats;


public:
    FixedMemoryPool(size_t blockSize, size_t numBlocks);
    ~FixedMemoryPool();

    void* allocate();
    void deallocate(void* ptr);

    //添加统计相关方法
    const Statistics& getStatistics() const{return stats;}
    void printStatistics() const{ stats.printReport(numBlocks);}

    FixedMemoryPool(const FixedMemoryPool&) = delete;
    FixedMemoryPool& operator=(const FixedMemoryPool&) = delete;


};
#endif //FIXEDMEMORYPOOL_H
