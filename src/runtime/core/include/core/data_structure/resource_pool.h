#pragma once
#include <cassert>
#include <cstdint>

namespace cloud
{
class ResourcePool
{
  public:
	static constexpr uint32_t INVALID_NUM = 0xffffffff;
	ResourcePool(const uint32_t &pool_size, uint32_t resource_size);
	virtual ~ResourcePool();
	void Shutdown();
	uint32_t FetchResource();
	void ReleaseResource(uint32_t index);
	void ReleaseAllResources();
	void *Access(uint32_t index);
	const void *Access(uint32_t index) const;
	uint32_t GetCapacity() const { return pool_size_; };

  protected:
	uint8_t *memory_{nullptr};
	uint32_t *free_indices_{nullptr};
	uint32_t free_indices_head_{0};
	uint32_t pool_size_{16};
	uint32_t resource_size_{4};
	uint32_t used_indices_{16};
};
} // namespace cloud