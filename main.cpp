#include "include/FixedMemoryPool.h"
#include <iostream>
#include <vector>

struct Point {
    int x;
    int y;
    float data[10];
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

int main() {
    std::cout << "=== Smart Memory Pool with Statistics ===\n" << std::endl;
    simpleTest();
    //performanceTest();
    //testMemoryReuse();

    return 0;

}