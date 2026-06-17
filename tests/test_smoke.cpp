// 冒烟测试：验证编译、AddressSanitizer 和测试框架端到端都能正常工作。
// 真正的 FixedPool 测试将在下一阶段到来，在我们一起实现 allocate()/deallocate() 之后。
#include "test_framework.h"
#include "mempool/fixed_pool.h"

static void framework_sanity() {
    CHECK(1 + 1 == 2);
    CHECK(true);
}

static void fp_test() {
    // init
    mempool::FixedPool fp(1, 4);
    CHECK(fp.block_size() == 8);
    CHECK(fp.free_count() == 4);
    CHECK(fp.capacity() == 4);

    // alloc
    CHECK(fp.allocate());
    CHECK(fp.free_count() == 3);
    CHECK(fp.capacity() == 4);

    auto* ptr = fp.allocate();
    CHECK(fp.free_count() == 2);

    CHECK(fp.allocate());
    CHECK(fp.free_count() == 1);

    fp.allocate();
    CHECK(fp.free_count() == 0);

    CHECK(fp.allocate() == nullptr);
    CHECK(fp.free_count() == 0);

    // dealloc
    fp.deallocate(ptr);
    CHECK(fp.free_count() == 1);
}

static void fp_write_test() {
    // 获取指针
    mempool::FixedPool fp(31, 3);
    void* ptr = fp.allocate();

    // write
    char* test_chars = static_cast<char*>(ptr);
    test_chars[0]  =  'N';
    test_chars[1]  =  'a';
    test_chars[2]  =  'n';
    test_chars[3]  =  'a';
    test_chars[30] =  'k';
    test_chars[32] =  'o';
    CHECK(test_chars[0] == 'N');
    CHECK(test_chars[30] == 'k');

    fp.deallocate(ptr);
}

int main() {
    RUN_TEST(framework_sanity);
    RUN_TEST(fp_test);
    RUN_TEST(fp_write_test);
    return mptest::summary();
}
