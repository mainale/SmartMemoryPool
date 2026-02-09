//
// Created by 30665 on 26-2-8.
//
#include "../include/Statistics.h"
#include <iomanip>

Statistics::Statistics()
    :totalAllocations(0), totalDeallocations(0),currentUsage(0),
peakUsage(0),failedAllocations(0),totalBytesAllocated(0),totalAllocationTime(0.0),
totalDeallocationTime(0.0){
    startTime = std::chrono::steady_clock::now();

}


void Statistics::recordAllocations(size_t block_size, double duration) {
    totalAllocations++;
    currentUsage++;
    totalBytesAllocated += block_size;
    totalAllocationTime += duration;

    if (currentUsage > peakUsage) {
        peakUsage = currentUsage;
    }

}

void Statistics::recordDeallocations(double duration) {
    totalDeallocations++;
    if (currentUsage > 0) {
        currentUsage--;
    }
    totalDeallocationTime += duration;
}

void Statistics::recordFailedAllocations() {
    failedAllocations++;
}

double Statistics::getUtilizationRate(size_t totalBlocks) const {
    if (totalBlocks == 0) {
        return 0.0;
    }
    return static_cast<double>(currentUsage) / static_cast<double>(totalBlocks) * 100.0;
}

double Statistics::getAverageAllocationTime() const {
    if (totalAllocations == 0) return 0.0;
    return totalAllocationTime / static_cast<double>(totalAllocations);
}

double Statistics::getAverageDeallocationTime() const {
    if (totalDeallocations == 0) return 0.0;
    return totalDeallocationTime / static_cast<double>(totalDeallocations);
}

double Statistics::getOperationsPerSecond() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - startTime).count();
    if (elapsed < 0.000001) {  // 小于1微秒
        return 0.0;
    }
    return static_cast<double>(totalAllocations + totalDeallocations) / elapsed;
}

void Statistics::reset() {
    totalAllocations = 0;
    totalDeallocations = 0;
    currentUsage = 0;
    peakUsage = 0;
    totalBytesAllocated = 0;
    totalAllocationTime = 0.0;
    totalDeallocationTime = 0.0;
    startTime = std::chrono::steady_clock::now();
    return;
}

void Statistics::printReport(size_t totalBlocks) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

    std::cout << "\n=== Memory Pool Statistics Report ===" << std::endl;
    std::cout << "Running Time: " << elapsed << "ms" << std::endl;
    std::cout << "Total Blocks: " << totalBlocks << std::endl;
    std::cout << "Currently Used Blocks: " << currentUsage << std::endl;
    std::cout << "Free Blocks: " << totalBlocks - currentUsage << std::endl;
    std::cout << "Usage Rate: " << std::fixed << std::setprecision(2) << getUtilizationRate(totalBlocks) << '%' << std::endl;
    std::cout << "Peak Usage Blocks: " << peakUsage << std::endl;
    std::cout << "\nOperation Statistics:" << std::endl;
    std::cout << "  Allocations: " << totalAllocations << std::endl;
    std::cout << "  Deallocations: " << totalDeallocations << std::endl;
    std::cout << "  Failed Allocations: " << failedAllocations << std::endl;
    std::cout << "  Total Allocated Bytes: " << totalBytesAllocated << std::endl;
    std::cout << "\nPerformance Statistics:" << std::endl;
    std::cout << "  Average Allocation Time: " << getAverageAllocationTime() << " us" << std::endl;
    std::cout << "  Average Deallocation Time: " << getAverageDeallocationTime() << " us" << std::endl;
    std::cout << "  Operations Per Second: " << getOperationsPerSecond() << " ops/s" << std::endl;
    std::cout << "=====================================\n" << std::endl;
}






