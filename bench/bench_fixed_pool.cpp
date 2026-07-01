//
// Created by neuroil on 2026/6/26.
//

#include <cstdio>
#include <chrono>
#include "mempool/fixed_pool.h"
#include "bench_util.h"

void test() {
    constexpr const int N = 1'000'000;
    std::size_t a = 1;
    std::size_t b = 2;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        a += b;
        b += a;
    }
    auto t1 = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double, std::nano>(t1 - t0).count();
    std::printf("\n循环 %d 次，", N);
    std::printf("耗时 %.2f ns", dt / N);
    std::printf("\na = %lu, b = %lu", a, b);
}

void test_new_delete() {
    constexpr int N = 10'086'000;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        void* ptr = ::operator new(324);
        do_not_optimize(ptr);
        ::operator delete(ptr);
        clobber_memory();
    }
    auto t1 = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double, std::nano>(t1 - t0).count();
    std::printf("\n循环 %d 次，", N);
    std::printf("new/delete 耗时: %.2f ns", dt / N);
}

void bench_alloc_lifo() {
    // 次数与 test_new_delete 对齐，便于直接对比每次操作耗时；
    // 取一块立刻还一块（LIFO），池容量只会占用 1 块，与循环次数无关。
    constexpr int N = 10'086'000;
    constexpr int block_count = 10086;
    mempool::FixedPool fp(324, block_count);
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        auto ptr = fp.allocate();
        do_not_optimize(ptr);
        fp.deallocate(ptr);
        clobber_memory();
    }
    auto t1 = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double, std::nano>(t1 - t0).count();
    std::printf("\n循环 %d 次，", N);
    std::printf("bench_alloc_lifo 耗时: %.2f ns", dt / N);
}

int main() {
    // test();
    test_new_delete();
    bench_alloc_lifo();
}
