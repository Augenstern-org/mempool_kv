add_executable(bench_fixed_pool bench_fixed_pool.cpp)
target_link_libraries(bench_fixed_pool PRIVATE mempool mempool_flags)  # flags 必须显式链
target_compile_options(bench_fixed_pool PRIVATE -O2)

add_executable(bench_hash_kv bench_hash_kv.cpp)
target_link_libraries(bench_hash_kv PRIVATE mempool mempool_flags)  # flags 必须显式链
target_compile_options(bench_hash_kv PRIVATE -O2)