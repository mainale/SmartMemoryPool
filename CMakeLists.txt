cmake_minimum_required(VERSION 3.21)
project(SmartMemoryPool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 选项（用于控制构建测试和示例）
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example programs" ON)

# 包含目录
include_directories(include)

# 核心源文件（不包含测试）
set(SOURCES
        src/FixedMemoryPool.cpp
        src/Statistics.cpp
        src/SizeClassMemoryPool.cpp
)

# 创建静态库
add_library(MemoryPool STATIC ${SOURCES})

# 线程支持（跨平台）
find_package(Threads REQUIRED)
target_link_libraries(MemoryPool PRIVATE Threads::Threads)

# 编译定义
target_compile_definitions(MemoryPool PRIVATE
        ENABLE_VERBOSE=$<CONFIG:Debug>
        THROW_ON_ALLOC_FAIL=0
)

# AddressSanitizer for Debug builds（按编译器区分）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(MemoryPool PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(MemoryPool PRIVATE -fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(MemoryPool PRIVATE /fsanitize=address)
        target_link_options(MemoryPool PRIVATE /fsanitize=address)
    endif()
endif()

# 测试程序
if(BUILD_TESTING)
    add_executable(run_tests tests/TestAllocator.cpp)
    target_link_libraries(run_tests MemoryPool Threads::Threads)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(run_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(run_tests PRIVATE -fsanitize=address)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options(run_tests PRIVATE /fsanitize=address)
            target_link_options(run_tests PRIVATE /fsanitize=address)
        endif()
    endif()
endif()

# 示例程序
if(BUILD_EXAMPLES)
    add_executable(example1_basic examples/example1_basic.cpp)
    target_link_libraries(example1_basic MemoryPool Threads::Threads)

    add_executable(example2_stl_vector examples/example2_stl_vector.cpp)
    target_link_libraries(example2_stl_vector MemoryPool Threads::Threads)

    add_executable(example3_multithread examples/example3_multithread.cpp)
    target_link_libraries(example3_multithread MemoryPool Threads::Threads)

    add_executable(example4_auto_expand examples/example4_auto_expand.cpp)
    target_link_libraries(example4_auto_expand MemoryPool Threads::Threads)

    # 为示例添加 ASan 支持
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            foreach(target example1_basic example2_stl_vector example3_multithread example4_auto_expand)
                target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
                target_link_options(${target} PRIVATE -fsanitize=address)
            endforeach()
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            foreach(target example1_basic example2_stl_vector example3_multithread example4_auto_expand)
                target_compile_options(${target} PRIVATE /fsanitize=address)
                target_link_options(${target} PRIVATE /fsanitize=address)
            endforeach()
        endif()
    endif()
endif()

# 测试注册
enable_testing()
if(BUILD_TESTING)
    add_test(NAME MemoryPoolTest COMMAND run_tests)
endif()

# 安装规则
install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

install(TARGETS MemoryPool
        EXPORT MemoryPoolTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(EXPORT MemoryPoolTargets
        FILE MemoryPoolTargets.cmake
        NAMESPACE MemoryPool::
        DESTINATION lib/cmake/MemoryPool
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/MemoryPoolConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
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