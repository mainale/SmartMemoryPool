//
// Created by 30665 on 26-2-21.
//
#include "../include/FixedMemoryPool.h"
#include <iostream>

struct Point { int x, y; };

int main() {
    std::cout << "Example 1: Basic allocation with FixedMemoryPool\n";
    FixedMemoryPool pool(sizeof(Point), 5, true);

    Point* points[5];
    for (int i = 0; i < 5; ++i) {
        points[i] = static_cast<Point*>(pool.allocate());
        if (points[i]) {
            points[i]->x = i;
            points[i]->y = i * 10;
            std::cout << "Allocated point " << i << " at " << points[i] << "\n";
        }
    }

    // 释放所有
    for (int i = 0; i < 5; ++i) {
        pool.deallocate(points[i]);
    }

    return 0;
}