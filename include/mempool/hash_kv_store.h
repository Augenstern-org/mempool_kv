//
// Created by neuroil on 2026/6/17.
//

#pragma once
#include <string>

#include "fixed_pool.h"

namespace mempool {
    struct Entry {
        char key[64];
        char val[256];
        Entry* next;
    };

    struct HashKVStore {
    public:
        HashKVStore(std::size_t bucket_count, std::size_t capacity);
        HashKVStore(const HashKVStore&) = delete;
        HashKVStore& operator=(const HashKVStore&) = delete;

        ~HashKVStore();

        bool get(std::string_view key, std::string& out) const;
        void put(std::string_view key, std::string_view val);
        bool del(std::string_view key);

        std::size_t size() const;

    private:
        std::size_t bucket_index(std::string_view key) const;

        std::size_t size_ = 0;
        std::size_t bucket_count_;
        Entry** entries_;
        FixedPool fixed_pool_;
    };
}
