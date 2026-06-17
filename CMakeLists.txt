cmake_minimum_required(VERSION 3.20)
project(mem_pool_kv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# 默认使用调试友好的编译方式；在学习过程中依赖 Sanitizer 来发现问题。
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

option(MEMPOOL_ASAN "Build with AddressSanitizer" ON)
# 公共包含路径、警告选项和 Sanitizer 开关的唯一真实来源。每个目标都链接这个库，
# 因此设置保持一致，编译器标志不会被重复指定。
add_library(mempool_flags INTERFACE)
target_include_directories(mempool_flags INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(mempool_flags INTERFACE -Wall -Wextra -Wpedantic)

if(MEMPOOL_ASAN)
  target_compile_options(mempool_flags INTERFACE
    -fsanitize=address -fno-omit-frame-pointer -g)
  target_link_options(mempool_flags INTERFACE -fsanitize=address)
endif()

enable_testing()
add_subdirectory(tests)
add_subdirectory(src)
