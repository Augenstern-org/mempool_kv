//
// Created by neuroil on 2026/6/26.
//
#pragma once

/*
    - 每次 allocate() 的返回指针 → 喂 do_not_optimize(p)
    - 每次 deallocate(p) 之后 → 调 clobber_memory()。

    deallocate 的副作用是把 next 指针写进刚释放那块自己的内存里，没有写屏障，
    -O2 会认为这块写"无人观察"直接删掉。
 */
template <typename T>
inline void do_not_optimize(const T& v) {
    asm volatile("" : : "r,m"(v) : "memory");
}
inline void clobber_memory() {
    asm volatile("" : : : "memory");
}

