#pragma once
#include <vector>
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_resource.h"
#include "core/data_structure/resource_pool.h"

namespace cloud::vulkan
{
struct ComponentResource;

class GPUResourceManager
{
  public:
	GPUResourceManager(VkDevice device,
					   VmaAllocator allocator,
					   ComponentResource &device_resource,
					   std::vector<ResourceUpdate> &resource_deletion_queue,
					   std::vector<DescriptorSetUpdate> &descriptor_set_updates,
					   bool debug_utils_extension_present);
	~GPUResourceManager();

	void SetResourceName(VkObjectType type, uint64_t handle, const char *name);
	void ReleaseResourcesInDeletionQueue() const;

	SamplerHandle CreateSampler(const SamplerCreation &creation);
	void DestroySampler(const SamplerHandle &handle, const uint32_t &frame_index);
	void DestroySamplerInstance(ResourceHandle handle) const;

	BufferHandle CreateBuffer(const BufferCreation &creation);
	void DestroyBuffer(const BufferHandle &handle, const uint32_t &frame_index);
	void DestroyBufferInstance(ResourceHandle handle) const;

	template <typename T>
	T *Access(const ResourceHandle &handle, ResourcePool &pool);

	void *DynamicAllocate(uint32_t size);
	void *MapBuffer(const render::MapBufferParameter &param);
	void UnMapBuffer(const render::MapBufferParameter &param);

  private:
	VkDevice device_;
	VmaAllocator allocator_;
	ComponentResource &device_resource_;
	std::vector<ResourceUpdate> &resource_deletion_queue_;
	std::vector<DescriptorSetUpdate> &descriptor_set_updates_;
	bool debug_utils_extension_present_{false};
};

template <typename T>
T *GPUResourceManager::Access(const ResourceHandle &handle, ResourcePool &pool)
{
	return reinterpret_cast<T *>(pool.Access(handle));
}
} // namespace cloud::vulkan