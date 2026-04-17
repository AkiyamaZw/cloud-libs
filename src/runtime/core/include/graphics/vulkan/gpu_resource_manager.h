#pragma once
#include <vector>
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_resource.h"
#include "core/data_structure/resource_pool.h"

namespace cloud::vulkan
{
struct ComponentResource;
struct DefaultResources;

class GPUResourceManager
{
  public:
	GPUResourceManager(VkDevice device,
					   VmaAllocator allocator,
					   VkDescriptorPool descriptor_pool,
					   uint32_t& current_frame,
					   ComponentResource &device_resource,
					   DefaultResources& default_resource,
					   std::vector<ResourceUpdate> &resource_deletion_queue,
					   std::vector<DescriptorSetUpdate> &descriptor_set_updates,
					   bool debug_utils_extension_present);
	~GPUResourceManager();

	void SetResourceName(VkObjectType type, uint64_t handle, const char *name);
	void ReleaseResourcesInDeletionQueue() const;

	void UpdateDynamicBuffer();
	void UpdateDescriptorSet();

	SamplerHandle CreateSampler(const SamplerCreation &creation);
	void DestroySampler(const SamplerHandle &handle, const uint32_t &frame_index);
	void DestroySamplerInstance(ResourceHandle handle) const;

	BufferHandle CreateBuffer(const BufferCreation &creation);
	void DestroyBuffer(const BufferHandle &handle, const uint32_t &frame_index);
	void DestroyBufferInstance(ResourceHandle handle) const;
	

	void DestroyDescriptorSet(const DescriptorSetHandle& handle, const uint32_t &frame_index);

	template <typename T>
	T *Access(const ResourceHandle &handle, ResourcePool &pool);

	void *DynamicAllocate(uint32_t size);
	void *MapBuffer(const render::MapBufferParameter &param);
	void UnMapBuffer(const render::MapBufferParameter &param);

protected:
	void UpdateDescriptorSetInternal(DescriptorSetUpdate &update);

  private:
	VkDevice device_;
	VmaAllocator allocator_;
	VkDescriptorPool descriptor_pool_;
	ComponentResource &device_resource_;
	DefaultResources &default_resource_;
	std::vector<ResourceUpdate> &resource_deletion_queue_;
	std::vector<DescriptorSetUpdate> &descriptor_set_updates_;
	uint32_t& current_frame_;
	bool debug_utils_extension_present_{false};
	

	
};

template <typename T>
T *GPUResourceManager::Access(const ResourceHandle &handle, ResourcePool &pool)
{
	return reinterpret_cast<T *>(pool.Access(handle));
}
} // namespace cloud::vulkan