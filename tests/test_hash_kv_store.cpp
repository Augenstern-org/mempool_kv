//
// Created by neuroil on 2026/6/17.
//

#include <iostream>

#include "test_framework.h"
#include "mempool/hash_kv_store.h"

static void basic_op() {
    mempool::HashKVStore ht(5, 20);
    std::string out;
    // get 到空键
    CHECK(ht.get("cat", out) == false);
    // put
    ht.put("Nanako", "奶酪");
    CHECK(ht.get("Nanako", out));
    CHECK(out == "奶酪");
    ht.put("i/tv", "Evil");
    // put 更新
    ht.put("Nanako", "日奈大王");
    ht.get("Nanako", out);
    CHECK(out == "日奈大王");
    // del
    CHECK(ht.del("i/tv"));
    CHECK(ht.get("i/tv", out) == false);
}

static void hash_conflict() {
    mempool::HashKVStore ht(1, 5);
    std::string out;
    ht.put("N", "日奈大王");
    ht.put("a", "日奈大王");
    ht.put("n", "日奈大王");
    ht.put("k", "日奈大王");
    ht.put("o", "日奈大王");
    CHECK(ht.get("o", out));
    CHECK(out == "日奈大王");
}

static void oom() {
    mempool::HashKVStore ht(1,1);
    ht.put("neuroil", "evil");
    try {
        ht.put("i/tv", "no");
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    RUN_TEST(basic_op);
    RUN_TEST(hash_conflict);
    RUN_TEST(oom);
    return mptest::summary();
}