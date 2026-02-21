cmake_minimum_required(VERSION 3.21)
project(SmartMemoryPool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example programs" ON)

include_directories(include)

# 核心库源文件（不含测试）
set(SOURCES
        src/FixedMemoryPool.cpp
        src/Statistics.cpp
        src/SizeClassMemoryPool.cpp
)

add_library(MemoryPool STATIC ${SOURCES})

# ---------- 线程处理 ----------
if(MSVC)
    # MSVC 不需要任何线程库，但为了代码安全，定义一个空的 Threads::Threads 目标
    add_library(Threads::Threads INTERFACE IMPORTED)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(MemoryPool PUBLIC Threads::Threads)
endif()
# ----------------------------

target_compile_definitions(MemoryPool PRIVATE
        ENABLE_VERBOSE=$<CONFIG:Debug>
        THROW_ON_ALLOC_FAIL=0
)

# AddressSanitizer（按编译器区分）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(MemoryPool PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(MemoryPool PRIVATE -fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(MemoryPool PRIVATE /fsanitize=address)
        target_link_options(MemoryPool PRIVATE /fsanitize=address)
    endif()
endif()

# 辅助宏：添加可执行文件（确保只链接 MemoryPool，不含任何 pthread）
macro(add_my_executable target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE MemoryPool)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target_name} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(${target_name} PRIVATE -fsanitize=address)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options(${target_name} PRIVATE /fsanitize=address)
            target_link_options(${target_name} PRIVATE /fsanitize=address)
        endif()
    endif()
endmacro()

# 测试程序
if(BUILD_TESTING)
    add_my_executable(run_tests tests/TestAllocator.cpp)
endif()

# 示例程序
if(BUILD_EXAMPLES)
    add_my_executable(example1_basic examples/example1_basic.cpp)
    add_my_executable(example2_stl_vector examples/example2_stl_vector.cpp)
    add_my_executable(example3_multithread examples/example3_multithread.cpp)
    add_my_executable(example4_auto_expand examples/example4_auto_expand.cpp)
endif()

# 测试注册
enable_testing()
if(BUILD_TESTING)
    add_test(NAME MemoryPoolTest COMMAND run_tests)
endif()

# 安装规则（保持不变）
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