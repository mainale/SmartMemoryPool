//
// Created by 30665 on 26-2-8.
//

#ifndef STATISTICS_H
#define STATISTICS_H

#include <iostream>
#include <chrono>

class Statistics {
private:
    size_t totalAllocations;  //总分配次数
    size_t totalDeallocations;  //总回收次数
    size_t currentUsage;    //正在使用的块数
    size_t peakUsage;   //使用峰值
    size_t failedAllocations;   //分配失败次数
    size_t totalBytesAllocated;     //总分配字节数

    //时间统计
    std::chrono::steady_clock::time_point startTime;
    double totalAllocationTime;
    double totalDeallocationTime;

public:
    Statistics();

    //记录操作
    void recordAllocations(size_t block_size,double duration = 0.0);
    void recordDeallocations(double duration = 0.0);
    void recordFailedAllocations();

    //获取统计信息
    size_t getAllocations() const{return totalAllocations;}
    size_t getDeallocations() const{return totalDeallocations;}
    size_t getCurrentUsage() const{return currentUsage;}
    size_t getPeakUsage() const{return peakUsage;}
    size_t getFailedAllocations() const{return failedAllocations;}
    size_t getFreeBlocks(size_t totalBlocks) const{return totalBlocks - currentUsage;}

    //计算统计数据
    double getUtilizationRate(size_t totalBlocks) const;
    double getAverageAllocationTime() const;
    double getAverageDeallocationTime() const;
    double getOperationsPerSecond() const;

    //重置统计
    void reset();

    //打印报告
    void printReport(size_t totalBlocks) const;
};
#endif //STATISTICS_H
