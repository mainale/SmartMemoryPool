# SmartMemoryPool

一个高性能、线程安全、支持自动扩展和STL兼容的内存池库。

## 特性

- **固定大小内存池**：用于高效管理同大小对象。
- **分级内存池**：支持 8 到 1536 字节的多种大小类，减少内部碎片。
- **自动扩展**：池满时可动态增加块数，避免分配失败。
- **线程安全**：提供原子操作和锁保护，支持多线程并发。
- **STL 兼容分配器**：可直接用于 `std::vector`、`std::list`、`std::map` 等容器。
- **大对象回退**：超过大小类阈值的分配自动转交系统分配器，并单独统计。
- **详细统计**：记录分配/释放次数、内存使用率、峰值、扩展次数等。

## 快速开始

### 构建

要求：CMake 3.21+，C++17 编译器。

```bash
git clone https://github.com/mainale/SmartMemoryPool.git
cd SmartMemoryPool
mkdir build && cd build
cmake ..
cmake --build .
```



### 示例

#### 1. 基础分配

cpp

```
#include "FixedMemoryPool.h"
#include <iostream>

struct Point { int x, y; };

int main() {
    FixedMemoryPool pool(sizeof(Point), 10);
    Point* p = static_cast<Point*>(pool.allocate());
    p->x = 10; p->y = 20;
    // 使用...
    pool.deallocate(p);
    return 0;
}
```



#### 2. 与 `std::vector` 配合

cpp

```
#include "SizeClassMemoryPool.h"
#include "MemoryPoolAllocator.h"
#include <vector>

int main() {
    SizeClassMemoryPool pool(1000);
    MemoryPoolAllocator<int> alloc(pool);
    std::vector<int, MemoryPoolAllocator<int>> vec(alloc);
    vec.push_back(42);
    return 0;
}
```



#### 3. 多线程安全

cpp

```
#include "GlobalMemoryPool.h"
#include <thread>
#include <vector>

void worker() {
    auto& pool = GlobalMemoryPool::get_instance().get_pool();
    for (int i = 0; i < 1000; ++i) {
        void* p = pool.allocate(64);
        pool.deallocate(p, 64);
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads) t.join();
    return 0;
}
```



更多示例请见 `examples/` 目录。



## 配置选项

编译时可定义以下宏（通过 `-D` 或 `target_compile_definitions`）：

| 宏                    | 默认值                      | 说明                                |
| :-------------------- | :-------------------------- | :---------------------------------- |
| `ENABLE_VERBOSE`      | `0` (Release) / `1` (Debug) | 启用详细输出                        |
| `THROW_ON_ALLOC_FAIL` | `0`                         | 分配失败时是否抛出 `std::bad_alloc` |

## 