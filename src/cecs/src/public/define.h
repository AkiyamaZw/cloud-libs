#pragma once
#include <cstddef>
#include <bitset>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <queue>
#include <limits>
#include <vector>

namespace cloud::world::ecs
{
using MaskType = std::bitset<64>;
using EntityIndex = uint64_t;
using EntityVersion = uint64_t;
using Index = size_t;
static constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

constexpr size_t BLOCK_MEMORY_32K = 32 * 1024;
constexpr size_t BLOCK_MEMORY_16K = 16384;
constexpr size_t BLOCK_MEMORY_8K = 8192;

constexpr bool is_16K = true;
constexpr size_t BLOCK_MEMORY =
    BLOCK_MEMORY_16K; // is_16K ? BLOCK_MEMORY_16K : BLOCK_MEMORY_8K;

using byte = unsigned char;

struct MetaType;

using MetaTypeList = std::vector<const MetaType *>;

} // namespace cloud::world::ecs
