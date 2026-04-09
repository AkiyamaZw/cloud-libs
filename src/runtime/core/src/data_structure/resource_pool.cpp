#include "core/data_structure/resource_pool.h"
#include "core/runtime_log.h"

namespace cloud
{
ResourcePool::ResourcePool(uint32_t pool_size, uint32_t resource_size)
	: pool_size_(pool_size)
	, resource_size_(resource_size)
{
	const size_t allocation_size = pool_size_ * (resource_size_ + sizeof(uint32_t));
	memory_ = static_cast<uint8_t *>(malloc(allocation_size));
	memset(memory_, 0, allocation_size);
	free_indices_ = reinterpret_cast<uint32_t *>(memory_ + pool_size_ * resource_size_);
	free_indices_head_ = 0;
	for (uint32_t i = 0; i < pool_size_; i++)
	{
		free_indices_[i] = i;
	}
	used_indices_ = 0;
}

ResourcePool::~ResourcePool() { Shutdown(); }

void ResourcePool::Shutdown()
{
	if (free_indices_head_ != 0)
	{
		WARN("Resource pool destroyed with unreleased resources");
		for (uint32_t i = 0; i < free_indices_head_; i++)
		{
			WARN("\tResource {}", free_indices_[i]);
		}
	}
	assert(used_indices_ == 0);
	if (memory_ != nullptr)
		free(memory_);
	memory_ = nullptr;
}

uint32_t ResourcePool::FetchResource()
{
	if (free_indices_head_ < pool_size_)
	{
		const uint32_t free_index = free_indices_[free_indices_head_++];
		++used_indices_;
		return free_index;
	}
	assert(false);
	return INVALID_NUM;
}

void ResourcePool::ReleaseResource(uint32_t index)
{
	free_indices_[--free_indices_head_] = index;
	--used_indices_;
}

void ResourcePool::ReleaseAllResources()
{
	free_indices_head_ = 0;
	used_indices_ = 0;
	for (uint32_t i = 0; i < pool_size_; i++)
	{
		free_indices_[i] = i;
	}
}

void *ResourcePool::Access(uint32_t index)
{
	if (index == INVALID_NUM)
		return nullptr;

	return &memory_[index * resource_size_];
}

const void *ResourcePool::Access(uint32_t index) const
{
	if (index == INVALID_NUM)
		return nullptr;

	return &memory_[index * resource_size_];
}

} // namespace cloud