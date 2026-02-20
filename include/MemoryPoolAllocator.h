//
// Created by 30665 on 26-2-11.
//

#ifndef MEMORYPOOLALLOCATOR_H
#define MEMORYPOOLALLOCATOR_H

#include <cstddef>
#include <memory>
#include <type_traits>
#include "SizeClassMemoryPool.h"

// 前向声明
template<typename T>
class MemoryPoolAllocator;

// 特化 std::allocator_traits
namespace std {
    template<typename T,typename U>
    constexpr bool operator==(const MemoryPoolAllocator<T>&, const MemoryPoolAllocator<U>&) noexcept;

    template<typename T,typename U>
    constexpr bool operator!=(const MemoryPoolAllocator<T>&, const MemoryPoolAllocator<U>&) noexcept;
}

//主模板类
template<typename T>
class MemoryPoolAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    //构造函数
    MemoryPoolAllocator() noexcept : pool_(nullptr) {}

    explicit MemoryPoolAllocator(SizeClassMemoryPool& pool) noexcept : pool_(&pool) {}

    template<typename U>
    MemoryPoolAllocator(const MemoryPoolAllocator<U>& other) noexcept : pool_(other.pool_) {}

    //分配内存
    pointer allocate(size_type n) {
        if (!pool_) {
            throw std::bad_alloc();
        }

        //计算需要的内存大小
        size_t totalSize = n * sizeof(T);

        //调用内存池分配
        void* ptr = pool_ -> allocate(totalSize);
        if (!ptr) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }

    //释放内存
    void deallocate(pointer p, size_type n) {
        if (p && pool_) {
            pool_ -> deallocate(p, n*sizeof(T));
        }
    }

    //获取最大可以分配的大小
    size_type max_size() const noexcept {
        //返回理论最大值，实际受限于内存池
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    //构造对象
    template<typename U,typename... Args>
    void construct(U* p,Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    //销毁对象
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    //获取底层内存池
    SizeClassMemoryPool* get_pool() const noexcept {
        return pool_;
    }

    // 重绑定模板（允许在容器内部使用不同的类型）
    template<typename U>
    struct rebind {
        using other = MemoryPoolAllocator<U>;
    };

    private:
    SizeClassMemoryPool* pool_;

    template<typename U>
    friend class MemoryPoolAllocator;

    template<typename U,typename V>
    friend constexpr bool std::operator==(const MemoryPoolAllocator<U>&, const MemoryPoolAllocator<V>&) noexcept;

    template<typename U, typename V>
    friend constexpr bool std::operator!=(const MemoryPoolAllocator<U>&, const MemoryPoolAllocator<V>&) noexcept;

};

// 比较操作符的实现
namespace std {
    template<typename T,typename U>
    constexpr bool operator==(const MemoryPoolAllocator<T>& lhs, const MemoryPoolAllocator<U>& rhs) noexcept {
        return lhs.pool_ == rhs.pool_;
    }

    template<typename T,typename U>
    constexpr bool operator!=(const MemoryPoolAllocator<T>& lhs, const MemoryPoolAllocator<U>& rhs) noexcept {
        return lhs.pool_ != rhs.pool_;
    }
}
#endif //MEMORYPOOLALLOCATOR_H
