//
// Created by neuroil on 2026/6/17.
//

#include "mempool/fixed_pool.h"
#include "mempool/fixed_pool.h"

#include <cstdlib>
#include <new>

// helper
inline size_t align_up(size_t size, size_t alignment) {
    // 这里的 alignment 必须是 2 的幂次
    return (size + (alignment - 1)) & ~(alignment - 1);
}

mempool::FixedPool::FixedPool(std::size_t block_size, std::size_t block_count)
    : block_count_(block_count), free_count_(block_count) {

    if (block_size < sizeof(FreeNode*)) block_size = sizeof(FreeNode*);
    block_size_ = align_up(block_size, sizeof(FreeNode*));
    // 申请内存
    size_t pool_size = block_size_ * block_count;
    pool_ = static_cast<std::byte*>(std::malloc(pool_size));
    if (!pool_) throw std::bad_alloc();

    // 初始化内存池
    auto* byte = pool_;
    FreeNode* head = nullptr;
    for (size_t index = 0; index < block_count_; ++index) {
        auto* node = reinterpret_cast<FreeNode*>(byte + index * block_size_);
        node->next = head;
        head = node;
    }

    free_list_ = head;
}

mempool::FixedPool::~FixedPool() {
    std::free(pool_);
}

void* mempool::FixedPool::allocate() {
    if (free_count_ <= 0) return nullptr;
    --free_count_;

    FreeNode* res = free_list_;
    free_list_ = free_list_->next;
    return res;
}

void mempool::FixedPool::deallocate(void* ptr) {
    auto* new_head = reinterpret_cast<FreeNode*>(ptr);

    new_head->next = free_list_;
    free_list_ = new_head;

    ++free_count_;
}


