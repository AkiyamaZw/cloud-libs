#include "bitbool.h"
#include <bitset>
#include <format>
#include <cassert>

namespace cloud::world::ecs::internal
{
BitBool64::BitBool64(void *raw_ptr, size_t _count)
    : count(_count)
{
    ptr = reinterpret_cast<uint64_t *>(raw_ptr);
    for (size_t i = 0; i < count; ++i)
    {
        bit_array()[i] = 0;
    }
};

bool BitBool64::get_bit(size_t index) const
{
    // get index of uint64_t array
    const uint16_t idx = (index >> 6);
    // get index in a the uint64_t as a array
    const uint16_t shift = (index & 0x3F);
    // set a mask using shift
    const uint64_t mask = (uint64_t(0x1) << shift);

    assert(index < count);
    return bit_array()[idx] & mask;
}

void BitBool64::set_bit(size_t index)
{
    bit_array()[(index >> 6)] |= (uint64_t(0x1) << uint16_t(index & 0x3F));
}

void BitBool64::clear_bit(size_t index)
{
    bit_array()[(index >> 6)] &=
        uint64_t(-1) ^ ((uint64_t(0x1) << uint16_t(index & 0x3F)));
}

std::string BitBool64::debug_format_to_string() const
{
    std::string ss = std::format("total count {}\n", count);
    for (size_t i = 0; i < (count >> 6); ++i)
    {
        std::bitset<64> mask(bit_array()[i]);
        ss += std::format("{}, {} \n", mask.to_string(), mask.count());
    }
    return ss;
}

} // namespace cloud::world::ecs::internal