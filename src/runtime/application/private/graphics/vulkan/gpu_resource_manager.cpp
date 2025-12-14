#include "graphics/vulkan/gpu_resource_manager.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "graphics/vulkan/device_base.h"
#include "runtime_log.h"

namespace cloud::vulkan
{
GPUResourceManager::GPUResourceManager(VkDevice device,
									   VmaAllocator allocator,
									   ComponentResource &device_resource,
									   std::vector<ResourceUpdate> &resource_deletion_queue,
									   std::vector<DescriptorSetUpdate> &descriptor_set_updates,
									   bool debug_utils_extension_present)
	: device_(device)
	, allocator_(allocator)
	, device_resource_(device_resource)
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
	SamplerHandle handle = {device_resource_.samplers.FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	Sampler *sampler = Access<Sampler>(handle.index, device_resource_.samplers);
	infra::CreateSampler(device_, creation, sampler->sampler);
	SetResourceName(
		VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(sampler->sampler), creation.name);
	return handle;
}
void GPUResourceManager::DestroySampler(const SamplerHandle &handle, const uint32_t &frame_index)
{
	if (handle.index < device_resource_.samplers.GetCapacity())
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
			DestroyBufferInstance(r.handle);
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
	if (auto sampler = static_cast<Sampler *>(device_resource_.samplers.Access(handle)))
	{
		vkDestroySampler(device_, sampler->sampler, nullptr);
	}
	device_resource_.samplers.ReleaseResource(handle);
}

BufferHandle GPUResourceManager::CreateBuffer(const BufferCreation &creation)
{
	BufferHandle handle = {device_resource_.buffers.FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	Buffer *buffer = Access<Buffer>(handle.index, device_resource_.buffers);
	buffer->name = creation.name;
	buffer->size = creation.size;
	buffer->usage_type = creation.usage_type;
	buffer->usage_flags = creation.usage_flags;
	buffer->handle = handle;
	buffer->global_offset = 0;
	buffer->parent_handle = BufferHandle{ResourcePool::INVALID_NUM};
	static const VkBufferUsageFlags buffer_usage_mask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
														VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
														VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const bool use_global_buffer = (creation.usage_flags & buffer_usage_mask) != 0;
	if (creation.usage_type == ResourceUsageType::Dynamic && use_global_buffer)
	{
		buffer->parent_handle = device_resource_.dynamic_buffer;
		return handle;
	};
	VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | creation.usage_flags;
	buffer_create_info.size = creation.size > 0 ? creation.size : 1;

	VmaAllocationCreateInfo allocation_create_info{};
	allocation_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	VmaAllocationInfo allocation_info{};
	vmaCreateBuffer(allocator_,
					&buffer_create_info,
					&allocation_create_info,
					&buffer->buffer,
					&buffer->allocation,
					&allocation_info);
	SetResourceName(VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer->buffer, creation.name);
	buffer->memory = allocation_info.deviceMemory;
	if (creation.initial_data)
	{
		void *data;
		vmaMapMemory(allocator_, buffer->allocation, &data);
		memcpy(data, creation.initial_data, (size_t)creation.size);
		vmaUnmapMemory(allocator_, buffer->allocation);
	}
	return handle;
}

void GPUResourceManager::DestroyBuffer(const BufferHandle &handle, const uint32_t &frame_index)
{
	if (handle.index < device_resource_.buffers.GetCapacity())
	{
		resource_deletion_queue_.push_back({ResourceUpdateType::Buffer, handle.index, frame_index});
	}
	else
	{
		WARN("Graphics error: try to free invalid buffer {}", handle.index);
	}
}

void GPUResourceManager::DestroyBufferInstance(ResourceHandle handle) const
{
	auto *buffer = static_cast<Buffer *>(device_resource_.buffers.Access(handle));
	if (buffer && buffer->parent_handle.index == ResourcePool::INVALID_NUM)
	{
		vmaDestroyBuffer(allocator_, buffer->buffer, buffer->allocation);
	}
	device_resource_.buffers.ReleaseResource(handle);
}

} // namespace cloud::vulkan
