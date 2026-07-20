#pragma once
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct BufferCreation
{
	VkBufferUsageFlags usage_flags{0};
	ResourceUsageType usage_type{ResourceUsageType::Immutable};
	uint32_t size{0};
	void *initial_data{nullptr};
	const char *name;
};

struct Buffer : public ResourceBase
{
	ResourceHandle parent_handle;
	VkBuffer buffer;
	VmaAllocation allocation;
	VkDeviceMemory memory;
	VkDeviceSize device_size;
	VkBufferUsageFlags usage_flags{0};
	ResourceUsageType usage_type{ResourceUsageType::Immutable};
	uint32_t size = 0;
	uint32_t global_offset{0};
	bool ready{false};
	uint8_t *mapped_data{nullptr};
};

struct DeviceData;
struct ResourceData;

ResourceHandle CreateVkBuffer(const BufferCreation &creation,
							  const DeviceData &device_data,
							  ResourceData &resource_data);

void DestroyVkBuffer(const ResourceHandle &handle,
					 const DeviceData &device_data,
					 ResourceData &resource_data);
} // namespace cloud::vulkan