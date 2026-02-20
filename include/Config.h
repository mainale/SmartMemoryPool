//
// Created by 30665 on 26-2-18.
//

#ifndef CONFIG_H
#define CONFIG_H

// 控制详细输出（如分配失败警告、扩展信息等）
#ifndef ENABLE_VERBOSE
#define ENABLE_VERBOSE 0
#endif

namespace MemoryPoolConfig {
    //调试模式
#ifndef DEBUG_MODE
    #define DEBUG_MODE 0
    #endif

    //性能统计
#ifndef ENABLE_STATISTICS
    #define ENABLE_STATISTICS 1
    #endif

    //线程安全
#ifndef ENABLE_THREAD_SAFETY
    #define ENABLE_THREAD_SAFETY 1
    #endif

    //默认块数
    constexpr size_t DEFAULT_BLOCKS_PER_CLASS = 1024;

    //最大小对象大小
    constexpr size_t MAX_SMALL_OBJECT_SIZE = 1024;

    //内存对齐
    constexpr size_t DEFAULT_ALIGNMENT = 8;

    //是否启用大对象分配
    constexpr bool ENABLE_LARGE_OBJECT_ALLOCATION = true;

    //大对象阈值
    constexpr size_t LARGE_OBJECT_THRESHOLD = 1024;

}
#endif //CONFIG_H
