#include "graphics/vulkan/gpu_resource_manager.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "runtime_log.h"

namespace cloud::vulkan
{
GPUResourceManager::GPUResourceManager(VkDevice device,
									   ResourcePool &samplers,
									   ResourcePool &shaders,
									   ResourcePool &pipelines,
									   ResourcePool &descriptor_sets,
									   std::vector<ResourceUpdate> &resource_deletion_queue,
									   std::vector<DescriptorSetUpdate> &descriptor_set_updates,
									   bool debug_utils_extension_present)
	: device_(device)
	, samplers_(&samplers)
	, shaders_(&shaders)
	, pipelines_(&pipelines)
	, descriptor_sets_(&descriptor_sets)
	, resource_deletion_queue_(resource_deletion_queue)
	, descriptor_set_updates_(descriptor_set_updates)
	, debug_utils_extension_present_(debug_utils_extension_present)
{
}

GPUResourceManager::~GPUResourceManager() {}

void GPUResourceManager::SetResourceName(VkObjectType type, uint64_t handle, const char *name)
{
	if (debug_utils_extension_present_)
	{
		infra::SetResourceName(device_, type, handle, name);
	}
}

SamplerHandle GPUResourceManager::CreateSampler(const SamplerCreation &creation)
{
	SamplerHandle handle = {samplers_->FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	Sampler *sampler = Access<Sampler>(handle.index, *samplers_);
	infra::CreateSampler(device_, creation, sampler->sampler);
	SetResourceName(
		VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(sampler->sampler), creation.name);
	return handle;
}
void GPUResourceManager::DestroySampler(const SamplerHandle &handle, const uint32_t &frame_index)
{
	if (handle.index < samplers_->GetCapacity())
	{
		resource_deletion_queue_.push_back(
			{ResourceUpdateType::Sampler, handle.index, frame_index});
	}
	else
	{
		WARN("release sampler handle with error handle index {}", handle.index);
	}
}

void GPUResourceManager::ReleaseResourcesInDeletionQueue() const
{
	for (uint32_t i = 0; i < resource_deletion_queue_.size(); i++)
	{
		ResourceUpdate &r = resource_deletion_queue_[i];
		if (r.current_frame == -1)
		{
			continue;
		}
		switch (r.type)
		{
		case ResourceUpdateType::Buffer:
			break;
		case ResourceUpdateType::Texture:
			break;
		case ResourceUpdateType::Pipeline:
			break;
		case ResourceUpdateType::Sampler:
			DestroySamplerInstance(r.handle);
			break;
		case ResourceUpdateType::DescriptorSetLayout:
			break;
		case ResourceUpdateType::DescriptorSet:
			break;
		case ResourceUpdateType::RenderPass:
			break;
		case ResourceUpdateType::Framebuffer:
			break;
		case ResourceUpdateType::ShaderState:
			break;
		case ResourceUpdateType::TextureView:
			break;
		case ResourceUpdateType::PagePool:
			break;
		case ResourceUpdateType::Count:
			break;
		}
	}
}

void GPUResourceManager::DestroySamplerInstance(ResourceHandle handle) const
{
	if (auto sampler = static_cast<Sampler *>(samplers_->Access(handle)))
	{
		vkDestroySampler(device_, sampler->sampler, nullptr);
	}
	samplers_->ReleaseResource(handle);
}

} // namespace cloud::vulkan
