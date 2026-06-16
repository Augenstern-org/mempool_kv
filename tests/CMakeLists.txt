add_executable(test_smoke test_smoke.cpp)
target_link_libraries(test_smoke PRIVATE mempool_flags)
add_test(NAME smoke COMMAND test_smoke)
