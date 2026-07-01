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

# LTO
# 必须在任何 add_subdirectory / add_executable 之前设置：
# CMAKE_INTERPROCEDURAL_OPTIMIZATION 的值在目标创建时被读取并固化到目标属性上，
# 之后再改动不会回填给已经创建的目标。若放在 add_subdirectory(bench) 之后，
# bench 目标会漏掉 LTO，导致 allocate/deallocate 无法内联进基准循环。
option(ENABLE_LTO "Enable Link Time Optimization" OFF)
if(ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error) # 接住检查结果，不支持时不硬报错
  if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(WARNING "未启用 LTO：编译器不支持 IPO：${ipo_error}")
  endif()
endif()

# 性能测试
option(MEMPOOL_BENCH "Build benchmarks (requires -DMEMPOOL_ASAN=OFF)" OFF)
if (MEMPOOL_BENCH)
  add_subdirectory(bench)
endif ()

if(MEMPOOL_ASAN)
  target_compile_options(mempool_flags INTERFACE
    -fsanitize=address -fno-omit-frame-pointer -g)
  target_link_options(mempool_flags INTERFACE -fsanitize=address)
endif()

enable_testing()
add_subdirectory(tests)
add_subdirectory(src)
