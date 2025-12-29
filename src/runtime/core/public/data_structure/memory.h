#pragma once
#include <cstddef>
#include <cstdint>

namespace cloud
{
class Allocator
{
  public:
	virtual ~Allocator() = default;
	virtual void *allocate(size_t size, size_t alignment) = 0;
	virtual void *allocate(size_t size, size_t alignment, const char *file, int32_t line) = 0;
	virtual void deallocate(void *ptr) = 0;
};

size_t MemoryAlign(const size_t size, const size_t alignment);
} // namespace cloud
