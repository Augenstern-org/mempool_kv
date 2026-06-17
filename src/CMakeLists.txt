# 把 FixedPool 实现编译成静态库，供 main 和测试目标共用
add_library(mempool STATIC
                    mempool/fixed_pool.cpp
                    mempool/hash_kv_store.cpp)


target_link_libraries(mempool PUBLIC mempool_flags)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE mempool)