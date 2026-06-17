#pragma once
#include <cstddef>

namespace mempool {


// 预分配一个连续区域，将其分割成等大的块
// 并将每个空闲块链接到一个单链表
class FixedPool {
public:
    FixedPool(std::size_t block_size, std::size_t block_count);
    ~FixedPool();

    FixedPool(const FixedPool&) = delete;
    FixedPool& operator=(const FixedPool&) = delete;

    void* allocate();
    void deallocate(void* ptr);

    std::size_t block_size() const { return block_size_; }
    std::size_t free_count() const { return free_count_; }
    std::size_t capacity()   const { return block_count_; }

private:
    struct FreeNode {
        FreeNode* next;
    };

    std::byte*  pool_;        // 预分配区域的起始地址
    FreeNode*   free_list_;   // 空闲链表的头节点
    std::size_t block_size_;  // 单个块的对齐后大小
    std::size_t block_count_;
    std::size_t free_count_;
};

} // namespace mempool
