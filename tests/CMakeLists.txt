# 冒烟测试
add_executable(test_smoke test_smoke.cpp)
target_link_libraries(test_smoke PRIVATE mempool)
add_test(NAME smoke COMMAND test_smoke)

# 哈希表
add_executable(test_hash_kv test_hash_kv_store.cpp)
target_link_libraries(test_hash_kv PRIVATE mempool)
add_test(NAME hash_kv COMMAND test_hash_kv)