#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace cloud::world::ecs::internal
{
// use uint_t64 as a bool[64]
struct BitBool64
{
    uint64_t *ptr{nullptr};
    // count is for the total bit count
    size_t count{0};

    BitBool64(void *raw_ptr, size_t _count);

    uint64_t *bit_array() const { return ptr; }

    bool get_bit(size_t index) const;

    void set_bit(size_t index);

    void clear_bit(size_t index);

    std::string debug_format_to_string() const;
};
} // namespace cloud::world::ecs::internal