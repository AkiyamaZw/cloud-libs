#pragma once
#include <vector>
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_resource.h"
#include "data_structure/resource_pool.h"

namespace cloud::vulkan
{

class GPUResourceManager
{
  public:
	GPUResourceManager(VkDevice device,
					   ResourcePool &buffers,
					   ResourcePool &samplers,
					   ResourcePool &shaders,
					   ResourcePool &pipelines,
					   ResourcePool &descriptor_sets,
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

	template <typename T>
	T *Access(const ResourceHandle &handle, ResourcePool &pool);

  private:
	VkDevice device_;
	ResourcePool *buffers_;
	ResourcePool *samplers_;
	ResourcePool *shaders_;
	ResourcePool *pipelines_;
	ResourcePool *descriptor_sets_;
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