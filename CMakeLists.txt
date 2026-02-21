cmake_minimum_required(VERSION 3.21)
project(SmartMemoryPool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 选项
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

# 线程支持：仅在非 MSVC 平台上需要显式链接
if(NOT MSVC)
    find_package(Threads REQUIRED)
    target_link_libraries(MemoryPool PUBLIC Threads::Threads)
endif()

# 编译定义
target_compile_definitions(MemoryPool PRIVATE
        ENABLE_VERBOSE=$<CONFIG:Debug>
        THROW_ON_ALLOC_FAIL=0
)

# AddressSanitizer 支持（按编译器区分）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(MemoryPool PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(MemoryPool PRIVATE -fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(MemoryPool PRIVATE /fsanitize=address)
        target_link_options(MemoryPool PRIVATE /fsanitize=address)
    endif()
endif()

# ==================== 测试程序 ====================
if(BUILD_TESTING)
    add_executable(run_tests tests/TestAllocator.cpp)
    target_link_libraries(run_tests MemoryPool)
    if(NOT MSVC)
        target_link_libraries(run_tests Threads::Threads)
    endif()

    # 为测试添加 ASan 支持
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

# ==================== 示例程序 ====================
if(BUILD_EXAMPLES)
    macro(add_example name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} MemoryPool)
        if(NOT MSVC)
            target_link_libraries(${name} Threads::Threads)
        endif()
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
                target_compile_options(${name} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
                target_link_options(${name} PRIVATE -fsanitize=address)
            elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                target_compile_options(${name} PRIVATE /fsanitize=address)
                target_link_options(${name} PRIVATE /fsanitize=address)
            endif()
        endif()
    endmacro()

    add_example(example1_basic examples/example1_basic.cpp)
    add_example(example2_stl_vector examples/example2_stl_vector.cpp)
    add_example(example3_multithread examples/example3_multithread.cpp)
    add_example(example4_auto_expand examples/example4_auto_expand.cpp)
endif()

# ==================== 测试注册 ====================
enable_testing()
if(BUILD_TESTING)
    add_test(NAME MemoryPoolTest COMMAND run_tests)
endif()

# ==================== 安装规则 ====================
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