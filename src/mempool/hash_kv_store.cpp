//
// Created by neuroil on 2026/6/17.
//

#include "mempool/hash_kv_store.h"
#include <cstring>
#include <stdexcept>

mempool::HashKVStore::HashKVStore(std::size_t bucket_count, std::size_t capacity) : bucket_count_(bucket_count),
                                                                                    fixed_pool_(sizeof(Entry), capacity) {
    entries_ = new Entry*[bucket_count_]{};
}

mempool::HashKVStore::~HashKVStore() {
    delete[] entries_;
}

std::size_t mempool::HashKVStore::bucket_index(std::string_view key) const {
    std::size_t hash = 2166136261u;
    for (char c : key) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 16777619u;
    }
    return hash % bucket_count_;
}

bool mempool::HashKVStore::get(std::string_view key, std::string& out) const {
    std::size_t index = bucket_index(key);
    Entry* curr = entries_[index];
    while (curr) {
        if (std::string_view(curr->key) == key) break;
        curr = curr->next;
    }
    if (!curr) return false;
    out = curr->val;
    return true;
}

void mempool::HashKVStore::put(std::string_view key, std::string_view val) {
    std::size_t index = bucket_index(key);
    Entry* curr = entries_[index];
    while (curr) {
        if (std::string_view(curr->key) == key) break;
        curr = curr->next;
    }
    if (curr) {
        std::memcpy(curr->val, val.data(), val.size());
        curr->val[val.size()] = '\0';
    }
    else {
        void* ptr = fixed_pool_.allocate();
        if (!ptr) throw std::runtime_error("failed to alloc!");
        auto* new_entry = reinterpret_cast<Entry*>(ptr);

        std::memcpy(new_entry->key, key.data(), key.size());
        new_entry->key[key.size()] = '\0';

        std::memcpy(new_entry->val, val.data(), val.size());
        new_entry->val[val.size()] = '\0';

        new_entry->next = entries_[index];
        entries_[index] = new_entry;
    }
}

bool mempool::HashKVStore::del(std::string_view key) {
    std::size_t index = bucket_index(key);
    Entry* curr = entries_[index];
    Entry* prev = nullptr;
    while (curr) {
        if (std::string_view(curr->key) == key) break;
        prev = curr;
        curr = curr->next;
    }
    if (!curr) return false;
    fixed_pool_.deallocate(curr);
    if (prev) prev->next = curr->next;
    else entries_[index] = curr->next;
    return true;
}
