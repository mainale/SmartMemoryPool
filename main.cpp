#include "include/FixedMemoryPool.h"
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>
#include <atomic>

struct Point {
    int x;
    int y;
    float data[10];
};

struct Data {
    int id;
    double value;
    char description[32];
};

void simpleTest(){
    std::cout << "=== Simple Test ===" << std::endl;

    FixedMemoryPool pool(sizeof(Point),5);

    //分配测试
    std::vector<Point*> points;
    for(int i = 0; i < 5; i++) {
        Point* p = static_cast<Point *>(pool.allocate());
        if (p) {
            p->x = i*10;
            p->y = i*20;
            points.push_back(p);
            std::cout << "Allocated point " << i << " at " << p << std::endl;
        }
    }

    //尝试超额分配
    Point* p = static_cast<Point *>(pool.allocate());
    if (!p) {
        std::cout << "Correctly rejected over-allocation" << std::endl;
    }

    for (int i = 0; i < 2; i++) {
        if (!points.empty()) {
            Point*  p = points.back();
            points.pop_back();
            pool.deallocate(p);
            std::cout << "Deallocated point" << std::endl;
        }
    }

    //重新分配
    Point* newPoint = static_cast<Point *>(pool.allocate());
    if (newPoint) {
        newPoint->x = 100;
        newPoint->y = 200;
        points.push_back(newPoint);
        std::cout << "Re-allocated point at " << newPoint << std::endl;
    }
    pool.printStatistics();
    //清理
    for (Point* p : points) {
        pool.deallocate(p);
    }

}

void performanceTest() {
    std::cout << "=== Performance Test ===" << std::endl;

    FixedMemoryPool pool(sizeof(Point),1000);

    //进行大量分配/释放操作
    std::vector<Point*> allocated;

    for (int i = 0; i < 1000; i++) {
        Point* p = static_cast<Point *>(pool.allocate());
        if (p) {
            p->x = rand()%100;
            p->y = rand()%100;
            allocated.push_back(p);
        }

        //随机释放一些
        if (i % 7 == 0 && !allocated.empty()) {
            int idx = rand()%allocated.size();
            Point* p = allocated[idx];
            pool.deallocate(p);
            allocated.erase(allocated.begin()+idx);
        }
    }
    //清理
    for (Point* p : allocated) {
        pool.deallocate(p);
    }
    pool.printStatistics();
}

void testMemoryReuse() {
    std::cout << "=== Memory Reuse Test ===" << std::endl;
    FixedMemoryPool pool(64,10);

    std::vector<void *> address;


    for (int i = 0; i < 10; i++) {
        void *ptr = pool.allocate();
        if (ptr) {
            address.push_back(ptr);
            std::cout << "Allocated block " << i << " at " << ptr << std::endl;
        }
    }
    //记录被分配过的地址
    std::vector<void*> originalAddress = address;

    //释放所有块
    for (int i = 0; i < 10; i++) {
        pool.deallocate(address.back());
    }
    address.clear();

    //重新分配，检查是否重用
    for (int i = 0; i < 10; i++) {
        void *ptr = pool.allocate();
        if (ptr) {
            address.push_back(ptr);
            bool found = false;
            for (void* ori: originalAddress) {
                if (ori == ptr) {
                    found = true;
                    break;
                }
            }
            std::cout << "Re-allocated block " << i << " at " << ptr
                     << " (reused: " << (found ? "YES" : "NO") << ")" << std::endl;
        }
    }
    for (void* ptr: address) {
        pool.deallocate(ptr);
    }

    pool.printStatistics();

}

// 测试1：基础线程安全测试
void basicThreadSafetyTest() {
    std::cout << "=== Basic Thread Safety Test ===" << std::endl;

    FixedMemoryPool pool(sizeof(Data),100);
    std::vector<std::thread> threads;
    std::vector<void*> allocatedPointers;
    std::mutex pointerMutex;

    auto worker = [&pool,&allocatedPointers,&pointerMutex](int threadId) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1,100);

        for (int i = 0; i < 20; i++) {
            //分配内存
            void *ptr = pool.allocateThreadSafe();

            if (ptr) {
                {
                    std::lock_guard<std::mutex> lock(pointerMutex);
                    allocatedPointers.push_back(ptr);
                }

                //使用内存
                Data* data = static_cast<Data*>(ptr);
                data->id = threadId*100 + i;
                data->value = threadId * 1.5 + i * 0.1;
                snprintf(data->description, 32, "Thread %d, Iteration %d", threadId, i);

                //模拟一些工作
                std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));


                // 50%的概率释放内存
                if (dis(gen) < 50) {
                    pool.deallocateThreadSafe(ptr);
                    {
                        std::lock_guard<std::mutex> lock(pointerMutex);
                        auto it = std::find(allocatedPointers.begin(),allocatedPointers.end(),ptr);

                        if (it != allocatedPointers.end()) {
                            allocatedPointers.erase(it);
                        }
                    }
                }
            }

            //短暂休眠
            std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
        }
    };

    //创建五个工作线程
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(worker,i+1);
    }

    //等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    pool.printStatistics();

    //清理剩余内存
    {
        std::lock_guard<std::mutex> lock(pointerMutex);
        for (void* ptr : allocatedPointers) {
            pool.deallocateThreadSafe(ptr);
        }
    }
}

// 测试2：性能对比 - 线程安全 vs 非线程安全
void performanceComparisonTest() {
    std::cout << "=== Performance Comparison Test ===" << std::endl;

    const int NUM_OPERATIONS = 10000;

    // 测试非线程安全版本
    {
        FixedMemoryPool pool(64,200);
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < NUM_OPERATIONS; i++) {
            void *ptr = pool.allocate();
            if (ptr) {
                pool.deallocate(ptr);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Non-thread-safe operations: " << duration.count() << " microseconds" << std::endl;
        std::cout << "Average time per operation: " << duration.count() / (double)NUM_OPERATIONS << " us" << std::endl;
    }

    //测试线程安全版本
    FixedMemoryPool pool(64,200);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        void *ptr = pool.allocateThreadSafe();
        if (ptr) {
            pool.deallocateThreadSafe(ptr);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Thread-safe operations: " << duration.count() << " microseconds" << std::endl;
    std::cout << "Average time per operation: " << duration.count() / (double)NUM_OPERATIONS << " us" << std::endl;

}

// 测试3：竞争条件测试
void raceConditionTest() {
    std::cout << "=== Race Condition Test ===" << std::endl;

    FixedMemoryPool pool(32,20);

    std::vector<std::thread> threads;
    std::atomic<int> allocationsCount{0};
    std::atomic<int> deallocationsCount{0};
    std::atomic<int> failedAllocations{0};

    auto worker = [&pool,&allocationsCount,&deallocationsCount,&failedAllocations](int threadId) {
        std::vector<void *> myPointers;
        for (int i = 0; i < 100; i++) {
            void *ptr = pool.allocateThreadSafe();
            if (ptr) {
                allocationsCount++;
                myPointers.push_back(ptr);

                // 在指针中存储线程和迭代信息
                int* data = static_cast<int *>(ptr);
                data[0] = threadId;
                data[1] = i;

                // 随机释放一些指针
                if (rand()%3 == 0 && !myPointers.empty()) {
                    int idx = rand()%myPointers.size();
                    pool.deallocateThreadSafe(myPointers[idx]);
                    deallocationsCount++;
                    myPointers.erase(myPointers.begin()+idx);
                }
            } else {
                failedAllocations++;
            }

        }

        // 释放所有剩余指针
        for (void* ptr : myPointers) {
            pool.deallocateThreadSafe(ptr);
            deallocationsCount++;
        }

    };

    //创建多个线程
    for (int i = 0; i < 8; i++) {
        threads.emplace_back(worker,i+1);
    }

    //等待所有线程
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "Total allocations: " << allocationsCount << std::endl;
    std::cout << "Total deallocations: " << deallocationsCount << std::endl;
    std::cout << "Failed allocations: " << failedAllocations << std::endl;
    std::cout << "Expected free blocks: 20" << std::endl;
    std::cout << "Actual free blocks: " << pool.getFreeBlocks() << std::endl;

    if (pool.getFreeBlocks() == 20) {
        std::cout << "SUCCESS: No memory leaks detected!" << std::endl;
    } else {
        std::cout << "FAILED: memory leaks detected!" << std::endl;
    }

    pool.printStatistics();
}

// 测试4：死锁预防测试
void deadLockPreventionTest() {
    std::cout << "=== Dead Lock Prevention Test ===" << std::endl;

    FixedMemoryPool pool1(64,10);
    FixedMemoryPool pool2(128,10);

    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> deadLockAvoidedCount{0};

    // 模拟可能发生死锁的情况
    auto worker = [&pool1,&pool2,&successCount,&deadLockAvoidedCount](int threadId,bool orderlyLocking) {
        for (int i = 0; i < 50; i++) {
            if (orderlyLocking) {
                void *ptr1 = pool1.allocateThreadSafe();
                void *ptr2 = pool2.allocateThreadSafe();

                if (ptr1 && ptr2) {
                    successCount++;
                }

                if (ptr1) pool1.deallocateThreadSafe(ptr1);
                if (ptr2) pool2.deallocateThreadSafe(ptr2);

            } else {
                // 随机顺序（可能死锁，但我们的锁粒度小，实际可能不会）
                if (rand()%2 == 0) {
                    void* ptr1 = pool1.allocateThreadSafe();
                    void* ptr2 = pool2.allocateThreadSafe();

                    if (ptr1 && ptr2) {
                        successCount++;
                    }

                    if (ptr1) pool1.deallocateThreadSafe(ptr1);
                    if (ptr2) pool2.deallocateThreadSafe(ptr2);
                } else {
                    void* ptr2 = pool2.allocateThreadSafe();
                    void* ptr1 = pool1.allocateThreadSafe();

                    if (ptr1 && ptr2) {
                        successCount++;
                    }
                    if (ptr1) pool1.deallocateThreadSafe(ptr1);
                    if (ptr2) pool2.deallocateThreadSafe(ptr2);
                }
            }
        }
    };

    // 创建线程
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(worker, i + 1, true);  // 有序加锁
    }

    for (int i = 0; i < 4; i++) {
        threads.emplace_back(worker, i + 1, false); // 无序加锁
    }

    // 等待所有线程
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Successful allocations from both pools: " << successCount << std::endl;
    std::cout << "Test completed without deadlock!" << std::endl;
}

int main() {
    //std::cout << "=== Smart Memory Pool with Statistics ===\n" << std::endl;
    //simpleTest();
    //performanceTest();
    //testMemoryReuse();
    std::cout << "=== Thread-Safe Memory Pool Tests ===\n" << std::endl;

    // 设置随机种子
    srand(static_cast<unsigned>(time(nullptr)));

    // 运行测试
    basicThreadSafetyTest();
    performanceComparisonTest();
    raceConditionTest();
    deadLockPreventionTest();
    return 0;

}