cmake_minimum_required(VERSION 3.21)
project(SmartMemoryPool)

set(CMAKE_CXX_STANDARD 17)

# 包含目录
include_directories(include)

# 源文件列表
set(SOURCES
        src/FixedMemoryPool.cpp
        src/Statistics.cpp
        src/SizeClassMemoryPool.cpp
        tests/TestAllocator.cpp
)

# 创建静态库
add_library(MemoryPool STATIC ${SOURCES})

# 编译定义
target_compile_definitions(MemoryPool PRIVATE
        ENABLE_VERBOSE=$<CONFIG:Debug>
        THROW_ON_ALLOC_FAIL=0
)

# 主测试程序（可选，如果希望保留之前的测试）
add_executable(run_tests tests/TestAllocator.cpp)
target_link_libraries(run_tests MemoryPool pthread)

# 示例程序
add_executable(example1_basic examples/example1_basic.cpp)
target_link_libraries(example1_basic MemoryPool pthread)

add_executable(example2_stl_vector examples/example2_stl_vector.cpp)
target_link_libraries(example2_stl_vector MemoryPool pthread)

add_executable(example3_multithread examples/example3_multithread.cpp)
target_link_libraries(example3_multithread MemoryPool pthread)

add_executable(example4_auto_expand examples/example4_auto_expand.cpp)
target_link_libraries(example4_auto_expand MemoryPool pthread)





# 安装头文件
install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

# 安装库
install(TARGETS MemoryPool
        EXPORT MemoryPoolTargets
        LIBRARY DESTINATION lib          # 动态库（如果改为 SHARED）
        ARCHIVE DESTINATION lib          # 静态库
        RUNTIME DESTINATION bin          # Windows 上的 DLL
        INCLUDES DESTINATION include
)

# 导出目标，供 find_package 使用
install(EXPORT MemoryPoolTargets
        FILE MemoryPoolTargets.cmake
        NAMESPACE MemoryPool::
        DESTINATION lib/cmake/MemoryPool
)

# 生成并安装包配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/MemoryPoolConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MemoryPoolConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/MemoryPoolConfig.cmake"
        INSTALL_DESTINATION lib/cmake/MemoryPool
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/MemoryPoolConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/MemoryPoolConfigVersion.cmake"
        DESTINATION lib/cmake/MemoryPool
)