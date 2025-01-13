#pragma once
#include "define.h"
#include <cassert>

namespace cloud::world::ecs::internal
{
struct Block;

struct alignas(8) BlockHeader
{
    Block *prev{nullptr};
    Block *next{nullptr};
};

struct alignas(32) Block
{
    byte storage[BLOCK_MEMORY_16K - sizeof(BlockHeader)];
    BlockHeader *header;
};

static_assert(sizeof(Block) == BLOCK_MEMORY_16K, "Block size if not 16kb!");

template <typename T>
struct MemoryAcessor
{
    MemoryAcessor() = default;
    MemoryAcessor(void *ptr, size_t elem_count)
    {
        assert(ptr != nullptr && elem_count >= 0);
        data = (T *)ptr;
        count = elem_count;
    };

    T &operator[](size_t index)
    {
        assert(index < count);
        return data[index];
    }

    const T &operator[](size_t index) const
    {
        assert(index < count);
        return data[index];
    }

    T *begin() { return data; }
    T *end() { return data + count; }

    T *data{nullptr};
    size_t count{0};
};
} // namespace cloud::world::ecs::internal