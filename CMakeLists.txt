cmake_minimum_required(VERSION 3.21)  # 使用更低的版本
project(SmartMemoryPool)

set(CMAKE_CXX_STANDARD 17)

# 设置变量
set(INCLUDE_DIR include)
set(SOURCE_DIR src)

# 包含头文件
include_directories(${INCLUDE_DIR})

# 手动列出所有源文件（不要用通配符）
add_executable(SmartMemoryPool
        main.cpp
        ${SOURCE_DIR}/FixedMemoryPool.cpp
        src/Statistics.cpp
        include/SizeClassMemoryPool.h
        src/SizeClassMemoryPool.cpp
)

# 如果只想编译库，可以添加：
# add_library(MemoryPool STATIC
#     src/FixedMemoryPool.cpp
#     src/MemoryPool.cpp
#     src/Statistics.cpp
# )

# 设置可执行文件的输出目录（可选）
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)