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
					   ResourcePool &samplers,
					   ResourcePool &shaders,
					   ResourcePool &pipelines,
					   ResourcePool &descriptor_sets,
					   std::vector<ResourceUpdate> &resource_deletion_queue,
					   std::vector<DescriptorSetUpdate> &descriptor_set_updates,
					   bool debug_utils_extension_present);
	~GPUResourceManager();

	void SetResourceName(VkObjectType type, uint64_t handle, const char *name);

	SamplerHandle CreateSampler(const SamplerCreation &creation);
	void DestroySampler(const SamplerHandle &handle, const uint32_t &frame_index);

	void ReleaseResourcesInDeletionQueue() const;
	void DestroySamplerInstance(ResourceHandle handle) const;

	template <typename T>
	T *Access(const ResourceHandle &handle, ResourcePool &pool);

  private:
	VkDevice device_;
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