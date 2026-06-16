#pragma once
// 最小化的零依赖测试框架。当项目增长到足以支撑时，通过 CMake FetchContent
// 引入 GoogleTest。
#include <cstdio>

namespace mptest {
inline int checks_run = 0;
inline int checks_failed = 0;

inline int summary() {
    std::printf("\n%d checks run, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
} // namespace mptest

#define CHECK(cond)                                                       \
    do {                                                                  \
        ++::mptest::checks_run;                                           \
        if (!(cond)) {                                                    \
            ++::mptest::checks_failed;                                    \
            std::printf("  [FAIL] %s:%d: CHECK(%s)\n",                    \
                        __FILE__, __LINE__, #cond);                       \
        }                                                                 \
    } while (0)

#define RUN_TEST(fn)                                                      \
    do {                                                                  \
        std::printf("[RUN ] %s\n", #fn);                                  \
        fn();                                                             \
    } while (0)
