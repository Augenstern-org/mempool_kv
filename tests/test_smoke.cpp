// 冒烟测试：验证编译、AddressSanitizer 和测试框架端到端都能正常工作。
// 真正的 FixedPool 测试将在下一阶段到来，在我们一起实现 allocate()/deallocate() 之后。
#include "test_framework.h"

static void framework_sanity() {
    CHECK(1 + 1 == 2);
    CHECK(true);
}

int main() {
    RUN_TEST(framework_sanity);
    return mptest::summary();
}
