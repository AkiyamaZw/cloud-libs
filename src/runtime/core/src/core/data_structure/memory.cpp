#include "core/data_structure/memory.h"

namespace cloud
{
size_t MemoryAlign(const size_t size, const size_t alignment)
{
	const size_t align_mask = alignment - 1;
	return (size + align_mask) & ~align_mask;
}
} // namespace cloud