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

constexpr size_t BLOCK_MEMORY_16K = 16384;

constexpr size_t BLOCK_MEMORY_8K = 8192;
using byte = unsigned char;

struct MetaType;
namespace internal
{
struct Chunk;
} // namespace internal

using MetaTypeList = std::vector<const MetaType *>;

} // namespace cloud::world::ecs
