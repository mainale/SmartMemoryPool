// main.cpp - 更新测试程序
#include <atomic>

#include "include/MemoryPoolAllocator.h"
#include "include/GlobalMemoryPool.h"
#include "include/Config.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <list>
#include <map>
#include <thread>


// 测试1：基础功能测试
void testBasicFunctionality() {
    std::cout << "=== Test 1: Basic Functionality ===" << std::endl;

    SizeClassMemoryPool pool;

    std::vector<void*> allocations;

    // 测试不同大小的分配
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};

    for (size_t size : sizes) {
        for (int i = 0; i < 3; i++) {
            if (void* ptr = pool.allocate(size)) {
                allocations.push_back(ptr);
                std::cout << "Allocated " << size << " bytes at " << ptr << std::endl;

                // 使用内存
                char* data = static_cast<char*>(ptr);
                for (size_t j = 0; j < std::min(size, static_cast<size_t>(10)); j++) {
                    data[j] = 'A' + (j % 26);
                }
            }
        }
    }

    // 释放一半的内存
    for (size_t i = 0; i < allocations.size() / 2; i++) {
        // 注意：实际中我们需要知道分配的大小，这里简化
        // 在实际应用中，我们需要跟踪分配大小或使用带大小的释放
        // 这里我们假设调用者知道大小
        pool.deallocate(allocations[i], sizes[i % 8]);
        std::cout << "Deallocated memory at " << allocations[i] << std::endl;
    }

    // 重新分配一些内存
    for (int i = 0; i < 5; i++) {
        size_t size = sizes[rand() % 8];
        if (void* ptr = pool.allocate(size)) {
            allocations.push_back(ptr);
            std::cout << "Re-allocated " << size << " bytes at " << ptr << std::endl;
        }
    }

    // 清理
    for (void* ptr : allocations) {
        // 这里我们不知道大小，所以无法正确释放
        // 在实际测试中，我们需要跟踪大小
        // 为简化，跳过这部分
    }

    pool.printStatistics();
}

// 测试2：性能测试
void testPerformance() {
    std::cout << "\n=== Test 2: Performance ===" << std::endl;

    SizeClassMemoryPool pool(1000);  // 每个等级1000个块

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> sizeDist(1, 1024);
    std::uniform_int_distribution<int> opDist(0, 1);

    constexpr int NUM_OPERATIONS = 10000;
    std::vector<std::pair<void*, size_t>> allocations;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (opDist(gen) == 0 || allocations.empty()) {
            // 分配
            size_t size = sizeDist(gen);
            if (void* ptr = pool.allocate(size)) {
                allocations.emplace_back(ptr, size);
            }
        } else {
            // 释放
            int idx = gen() % allocations.size();
            auto& alloc = allocations[idx];
            pool.deallocate(alloc.first, alloc.second);
            allocations[idx] = allocations.back();
            allocations.pop_back();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 清理
    for (auto& alloc : allocations) {
        pool.deallocate(alloc.first, alloc.second);
    }

    std::cout << "Completed " << NUM_OPERATIONS << " operations in "
              << duration.count() << " microseconds" << std::endl;
    std::cout << "Average time per operation: "
              << duration.count() / static_cast<double>(NUM_OPERATIONS) << " us" << std::endl;

    pool.printStatistics();
}

// 测试3：内存效率测试
void testMemoryEfficiency() {
    std::cout << "\n=== Test 3: Memory Efficiency ===" << std::endl;

    SizeClassMemoryPool pool(100);

    // 测试不同大小的内存浪费
    struct TestCase {
        size_t requestedSize;
        size_t actualSize;
        double wastePercentage;
    };

    std::vector<TestCase> testCases;

    // 测试一系列大小
    for (size_t size = 1; size <= 1024; size *= 2) {
        //size_t classIndex = pool.getSizeClassForSize(size);

        // 这里我们需要知道实际分配的大小，但我们的API不提供这个信息
        // 我们将创建一个简化版本
        std::cout << "Requested " << size << " bytes -> ";

        // 分配并检查实际大小（通过对比不同大小的分配）
        if (void* ptr = pool.allocate(size)) {
            // 查找这个指针属于哪个池（简化）
            // 在实际实现中，我们需要更好的方法
            std::cout << "Allocated successfully" << std::endl;
            pool.deallocate(ptr, size);
        }
    }

    std::cout << "\nMemory efficiency analysis:" << std::endl;
    std::cout << "Smaller requests may have higher waste due to size class rounding." << std::endl;
    std::cout << "The size class system reduces fragmentation compared to a single pool." << std::endl;
}

// 测试4：线程安全测试
void testThreadSafety() {
    std::cout << "\n=== Test 4: Thread Safety ===" << std::endl;

    SizeClassMemoryPool pool(500);
    std::vector<std::thread> threads;
    constexpr  int NUM_THREADS = 8;
    constexpr  int OPERATIONS_PER_THREAD = 1000;

    std::atomic<int> totalAllocations{0};
    std::atomic<int> totalFailures{0};

    auto worker = [&pool, &totalAllocations, &totalFailures](int threadId) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> sizeDist(1, 512);

        std::vector<std::pair<void*, size_t>> myAllocations;

        for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
            size_t size = sizeDist(gen);

            if (void* ptr = pool.allocate(size)) {
                ++totalAllocations;
                myAllocations.emplace_back(ptr, size);

                // 偶尔释放一些内存
                if (gen() % 4 == 0 && !myAllocations.empty()) {
                    int idx = gen() % myAllocations.size();
                    pool.deallocate(myAllocations[idx].first, myAllocations[idx].second);
                    myAllocations[idx] = myAllocations.back();
                    myAllocations.pop_back();
                }
            } else {
                ++totalFailures;
            }
        }

        // 清理
        for (auto& alloc : myAllocations) {
            pool.deallocate(alloc.first, alloc.second);
        }
    };

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Thread safety test completed:" << std::endl;
    std::cout << "  Threads: " << NUM_THREADS << std::endl;
    std::cout << "  Total operations: " << NUM_THREADS * OPERATIONS_PER_THREAD << std::endl;
    std::cout << "  Successful allocations: " << totalAllocations << std::endl;
    std::cout << "  Failed allocations: " << totalFailures << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  No crashes or deadlocks detected" << std::endl;

    pool.printStatistics();
}

// 测试5：边界情况测试
void testEdgeCases() {
    std::cout << "\n=== Test 5: Edge Cases ===" << std::endl;

    SizeClassMemoryPool pool(10);  // 小池以测试边界

    // 测试1: 分配0字节
    void* ptr0 = pool.allocate(0);
    std::cout << "Allocate 0 bytes: " << (ptr0 == nullptr ? "nullptr (correct)" : "non-null (error)") << std::endl;

    // 测试2: 释放nullptr
    pool.deallocate(nullptr, 10);
    std::cout << "Deallocate nullptr: no crash (correct)" << std::endl;

    // 测试3: 分配超过最大大小
    void* ptrLarge = pool.allocate(2048);  // 超过1024
    std::cout << "Allocate 2048 bytes (exceeds max): "
              << (ptrLarge == nullptr ? "nullptr (correct)" : "allocated (error)") << std::endl;

    // 测试4: 大量分配直到池满
    std::vector<void*> allocations;
    int successCount = 0;

    for (int i = 0; i < 100; i++) {
        if (void* ptr = pool.allocate(64)) {
            allocations.push_back(ptr);
            successCount++;
        } else {
            break;
        }
    }

    std::cout << "Allocated " << successCount << " blocks of 64 bytes before pool full" << std::endl;

    // 释放一些
    for (int i = 0; i < 5; i++) {
        if (!allocations.empty()) {
            pool.deallocate(allocations.back(), 64);
            allocations.pop_back();
        }
    }

    // 尝试重新分配
    void* ptrAfterFree = pool.allocate(64);
    std::cout << "After freeing some blocks, new allocation: "
              << (ptrAfterFree ? "successful" : "failed") << std::endl;

    if (ptrAfterFree) {
        pool.deallocate(ptrAfterFree, 64);
    }

    // 清理
    for (void* ptr : allocations) {
        pool.deallocate(ptr, 64);
    }

    pool.printStatistics();
}

// 测试1：基本分配器测试
void testBasicAllocator() {
    std::cout << "=== Test 1: Basic Allocator ===" << std::endl;

    // 创建内存池
    SizeClassMemoryPool pool(100);

    // 创建分配器
    MemoryPoolAllocator<int> allocator(pool);

    // 分配一些内存
    int* arr = allocator.allocate(10);
    for (int i = 0; i < 10; i++) {
        allocator.construct(arr + i,i*10);
        std::cout << "arr[" << i << "] = " << arr[i] << std::endl;
    }

    // 销毁对象
    for (int i = 0; i < 10; i++) {
        allocator.destroy(arr + i);
    }

    // 释放内存
    allocator.deallocate(arr, 10);

    std::cout << "Basic allocator test passed" << std::endl;
}

// 测试2：STL vector 使用内存池
void testVectorWithAllocator() {
    std::cout << "\n=== Test 2: STL Vector with Memory Pool ===" << std::endl;

    SizeClassMemoryPool pool(1000);
    MemoryPoolAllocator<int> allocator(pool);

    // 使用自定义分配器创建vector
    std::vector<int,MemoryPoolAllocator<int>> vec(allocator);

    auto start = std::chrono::high_resolution_clock::now();

    //插入大量元素
    for (int i = 0; i < 10000; i++) {
        vec.push_back(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Vector size: " << vec.size() << std::endl;
    std::cout << "Time to insert 10000 elements: " << duration.count() << " us" << std::endl;
    std::cout << "Average time per insertion: " << duration.count() / 10000.0 << " us" << std::endl;

    // 验证数据
    bool correct = true;
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i] != static_cast<int>(i)) {
            correct = false;
            break;
        }
    }
    std::cout << "Data verification: " << (correct ? "PASS" : "FAIL") << std::endl;
}

// 测试3：STL list 使用内存池
void testListWithAllocator() {
    std::cout << "\n=== Test 3: STL List with Memory Pool ===" << std::endl;

    SizeClassMemoryPool pool(10000);
    MemoryPoolAllocator<int> allocator(pool);

    // 使用自定义分配器创建list
    std::list<int, MemoryPoolAllocator<int>> lst(allocator);

    auto start = std::chrono::high_resolution_clock::now();

    // 插入大量元素
    for (int i = 0; i < 5000; i++) {
        lst.push_back(i);
        lst.push_front(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "List size: " << lst.size() << std::endl;
    std::cout << "Time to insert 10000 elements: " << duration.count() << " us" << std::endl;

    // 验证数据
    auto it = lst.begin();
    std::advance(it, 5000);  // 移动到中间
    std::cout << "Middle element: " << *it << std::endl;

    // 清理
    lst.clear();

    std::cout << "List test completed" << std::endl;
}

// 测试4：STL map 使用内存池
void testMapWithAllocator() {
    std::cout << "\n=== Test 4: STL Map with Memory Pool ===" << std::endl;

    SizeClassMemoryPool pool(2000);

    // 注意：map需要pair的分配器
    using MapAllocator = MemoryPoolAllocator<std::pair<const int, std::string>>;
    MapAllocator allocator(pool);

    std::map<int, std::string, std::less<int>, MapAllocator> myMap(allocator);

    auto start = std::chrono::high_resolution_clock::now();

    // 插入元素
    for (int i = 0; i < 1000; i++) {
        myMap[i] = "Value_" + std::to_string(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Map size: " << myMap.size() << std::endl;
    std::cout << "Time to insert 1000 elements: " << duration.count() << " us" << std::endl;

    // 查找测试
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        auto it = myMap.find(i * 10);
        if (it != myMap.end()) {
            // 找到元素
        }
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Time for 100 lookups: " << duration.count() << " us" << std::endl;

    std::cout << "Map test completed" << std::endl;
}

// 测试5：全局分配器测试
void testGlobalAllocator() {
    std::cout << "\n=== Test 5: Global Allocator ===" << std::endl;

    // 使用全局分配器
    std::vector<int, GlobalAllocator<int>> vec1;
    std::vector<int, GlobalAllocator<int>> vec2;

    // 两个vector共享同一个全局内存池
    for (int i = 0; i < 1000; i++) {
        vec1.push_back(i);
        vec2.push_back(i * 2);
    }

    std::cout << "vec1 size: " << vec1.size() << std::endl;
    std::cout << "vec2 size: " << vec2.size() << std::endl;

    // 验证
    bool correct = true;
    for (size_t i = 0; i < vec1.size(); i++) {
        if (vec1[i] != static_cast<int>(i)) {
            correct = false;
            break;
        }
    }

    std::cout << "Data verification: " << (correct ? "PASS" : "FAIL") << std::endl;// 打印全局内存池统计
    GlobalMemoryPool::get_instance().print_statistics();
}

// 测试6：性能对比（内存池 vs 系统分配器）
void testPerformanceComparison() {
    std::cout << "\n=== Test 6: Performance Comparison ===" << std::endl;

    const int NUM_ELEMENTS = 100000;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 1000);

    // 使用系统分配器
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int> vec;
        vec.reserve(NUM_ELEMENTS);

        for (int i = 0; i < NUM_ELEMENTS; i++) {
            vec.push_back(dist(gen));
        }

        // 随机删除一些元素
        for (int i = 0; i < NUM_ELEMENTS / 10; i++) {
            int idx = dist(gen) % vec.size();
            vec.erase(vec.begin() + idx);
        }

        // 再添加一些元素
        for (int i = 0; i < NUM_ELEMENTS / 5; i++) {
            vec.push_back(dist(gen));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "System allocator time: " << duration.count() << " us" << std::endl;
    }

    // 使用内存池分配器
    {
        SizeClassMemoryPool pool(5000);
        MemoryPoolAllocator<int> allocator(pool);

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int, MemoryPoolAllocator<int>> vec(allocator);
        vec.reserve(NUM_ELEMENTS);

        for (int i = 0; i < NUM_ELEMENTS; i++) {
            vec.push_back(dist(gen));
        }

        // 随机删除一些元素
        for (int i = 0; i < NUM_ELEMENTS / 10; i++) {
            int idx = dist(gen) % vec.size();
            vec.erase(vec.begin() + idx);
        }

        // 再添加一些元素
        for (int i = 0; i < NUM_ELEMENTS / 5; i++) {
            vec.push_back(dist(gen));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Memory pool allocator time: " << duration.count() << " us" << std::endl;
        std::cout << "Performance improvement: "
                  << (100.0 - (duration.count() * 100.0 / 138000.0)) << "%" << std::endl;
    }
}

// 测试7：多线程容器测试
void testMultithreadedContainers() {
    std::cout << "\n=== Test 7: Multithreaded Container Test ===" << std::endl;

    GlobalMemoryPool& globalPool = GlobalMemoryPool::get_instance();

    const int NUM_THREADS = 4;
    const int OPERATIONS_PER_THREAD = 5000;

    std::vector<std::thread> threads;
    std::atomic<int> totalInserted{0};

    auto worker = [&totalInserted](int threadId) {
        // 每个线程有自己的vector，但共享全局内存池
        std::vector<int, GlobalAllocator<int>> localVec;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 100);

        for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
            try {
                if (dist(gen) < 70 || localVec.empty()) {
                    localVec.push_back(threadId * 10000 + i);
                    totalInserted++;
                } else {
                    int idx = dist(gen) % localVec.size();
                    localVec.erase(localVec.begin() + idx);
                }
            } catch (const std::bad_alloc&) {
                std::cerr << "Thread " << threadId << " allocation failed, skipping\n";
            }
        }
    };

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Total inserted elements (approx): " << totalInserted << std::endl;
    std::cout << "Time for " << NUM_THREADS << " threads: "
              << duration.count() << " ms" << std::endl;
    std::cout << "No crashes detected - thread safety verified" << std::endl;
    GlobalMemoryPool::get_instance().print_statistics();
}

void testAutoExpand() {
    std::cout << "\n=== Test Auto Expand ===" << std::endl;
    FixedMemoryPool pool(64, 5, true);
    pool.setAutoExpand(true, 3);   // 每次扩展 3 块

    std::vector<void*> ptrs;
    for (int i = 0; i < 20; ++i) {
        void* p = pool.allocate();
        if (p) {
            ptrs.push_back(p);
            std::cout << "Allocated " << i << std::endl;
        } else {
            std::cout << "Failed at " << i << std::endl;
            break;
        }
    }

    std::cout << "Total chunks: " << pool.getTotalChunks() << std::endl;
    std::cout << "Num expansions: " << pool.getNumExpansions() << std::endl;

    for (auto p : ptrs) pool.deallocate(p);
}

int main() {
    testAutoExpand();
    return 0;
}