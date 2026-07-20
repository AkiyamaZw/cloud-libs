#include "graphics/vulkan/resource/buffer.h"
#include "core/data_structure/resource_pool.h"
#include "graphics/vulkan/device_data.h"
#include "graphics/vulkan/vulkan_interface.h"

namespace cloud::vulkan
{
ResourceHandle CreateVkBuffer(const BufferCreation &creation,
							  const DeviceData &device_data,
							  ResourceData &resource_data)
{
	Buffer *buffer =
		infra::AllocResource<Buffer>(resource_data, ResourceType::Buffer, creation.name);
	buffer->size = creation.size;
	buffer->usage_type = creation.usage_type;
	buffer->usage_flags = creation.usage_flags;
	buffer->global_offset = 0;
	buffer->parent_handle = {ResourcePool::INVALID_NUM, ResourceType::Buffer};
	static const VkBufferUsageFlags buffer_usage_mask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
														VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
														VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const bool use_global_buffer = (creation.usage_flags & buffer_usage_mask) != 0;
	if (creation.usage_type == ResourceUsageType::Dynamic && use_global_buffer)
	{
		buffer->parent_handle = resource_data.dynamic_buffer.buffer;
		return buffer->handle;
	};
	VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | creation.usage_flags;
	buffer_create_info.size = creation.size > 0 ? creation.size : 1;

	VmaAllocationCreateInfo allocation_create_info{};
	allocation_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	VmaAllocationInfo allocation_info{};
	auto succ = vmaCreateBuffer(resource_data.vma_allocator,
								&buffer_create_info,
								&allocation_create_info,
								&buffer->buffer,
								&buffer->allocation,
								&allocation_info);
	check_vk(succ);
	infra::SetResourceName(
		device_data.device, VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer->buffer, creation.name);
	buffer->memory = allocation_info.deviceMemory;
	if (creation.initial_data)
	{
		void *data;
		vmaMapMemory(resource_data.vma_allocator, buffer->allocation, &data);
		memcpy(data, creation.initial_data, (size_t)creation.size);
		vmaUnmapMemory(resource_data.vma_allocator, buffer->allocation);
	}
	return buffer->handle;
}

void DestroyVkBuffer(const ResourceHandle &handle,
					 const DeviceData &device_data,
					 ResourceData &resource_data)
{
	Buffer *buffer = infra::Access<Buffer>(resource_data, handle);
	if (buffer && buffer->parent_handle.index == ResourcePool::INVALID_NUM)
	{
		vmaDestroyBuffer(resource_data.vma_allocator, buffer->buffer, buffer->allocation);
	}
	infra::ReleaseResourceBase(resource_data, buffer);
}
} // namespace cloud::vulkan