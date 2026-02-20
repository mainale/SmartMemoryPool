//
// Created by 30665 on 26-2-17.
//

#ifndef GLOBALMEMORYPOOL_H
#define GLOBALMEMORYPOOL_H

#include <mutex>
#include <memory>
#include "SizeClassMemoryPool.h"

// 全局内存池管理器（单例模式）
class GlobalMemoryPool {
    private:
    static std::unique_ptr<GlobalMemoryPool> instance_;
    static std::once_flag init_flag_;

    SizeClassMemoryPool pool_;
    std::mutex pool_mutex_;

    //私有构造函数
    GlobalMemoryPool()
        : pool_(2048) {     // 默认每个大小等级2048个块
        std::cout << "Global memory pool initialized" << std::endl;
    }
    public:
    // 禁止拷贝和移动
    GlobalMemoryPool(const GlobalMemoryPool&) = delete;
    GlobalMemoryPool& operator=(const GlobalMemoryPool&) = delete;

    // 获取单例实例
    static GlobalMemoryPool& get_instance() {
        std::call_once(init_flag_, []() {
            instance_ = std::unique_ptr<GlobalMemoryPool>(new GlobalMemoryPool());
        });
        return *instance_;
    }

    // 获取内存池引用
    SizeClassMemoryPool& get_pool() {
        return pool_;
    }

    // 线程安全的分配
    void* allocate(size_t size) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        return pool_.allocate(size);
    }

    // 线程安全的释放
    void deallocate(void* ptr,size_t size) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        return pool_.deallocate(ptr, size);
    }

    // 打印统计信息
    void print_statistics() {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        pool_.printStatistics();
    }
};
// 静态成员初始化
std::unique_ptr<GlobalMemoryPool> GlobalMemoryPool::instance_ = nullptr;
std::once_flag GlobalMemoryPool::init_flag_;

// 全局分配器（使用全局内存池）
template<typename T>
class GlobalAllocator {
    public:
    using value_type = T;

    GlobalAllocator() noexcept = default;

    template<typename U>
    GlobalAllocator(const GlobalAllocator<U>&) noexcept {}

    static T* allocate(std::size_t n) {
        return static_cast<T*>(
            GlobalMemoryPool::get_instance().allocate(n * sizeof(T))
            );
    }

    static void deallocate(T* p, std::size_t n) {
        GlobalMemoryPool::get_instance().deallocate(p, n * sizeof(T));
    }

    template<typename U>
    struct rebind {
        using other = GlobalAllocator<U>;
    };
};

template<typename T,typename U>
bool operator==(GlobalAllocator<T>&,GlobalAllocator<U>&) noexcept {
    return true;    // 全局分配器总是相等
}

template<typename T,typename U>
bool operator!=(GlobalAllocator<T>&,GlobalAllocator<U>&) noexcept {
    return false;
}
#endif //GLOBALMEMORYPOOL_H
