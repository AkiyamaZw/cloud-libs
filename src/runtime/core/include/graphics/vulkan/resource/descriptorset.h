#pragma once
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct DescriptorSet
{
	VkDescriptorSet vk_descriptor_set;
	ResourceHandle *resources{nullptr};
	ResourceHandle *samplers{nullptr};
	uint16_t *bindings{nullptr};
	const DescriptorSetLayout *layout{nullptr};
	uint32_t num_resources{0};
};

struct DeviceData;
struct ResourceData;

void DestroyVkDescriptorSet(const ResourceHandle &handle,
							const DeviceData &device_data,
							ResourceData &resource_data);
} // namespace cloud::vulkan