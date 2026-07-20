#pragma once
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct Sampler : public ResourceBase
{
	VkSampler sampler;
	VkFilter min_filter{VK_FILTER_NEAREST};
	VkFilter mag_filter{VK_FILTER_NEAREST};
	VkSamplerMipmapMode mipmap_mode{VK_SAMPLER_MIPMAP_MODE_NEAREST};
	VkSamplerAddressMode address_mode_u{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_v{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_w{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerReductionMode reduction_mode{VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE};
};

struct SamplerCreation
{
	const char *name;
	VkFilter min_filter{VK_FILTER_NEAREST};
	VkFilter mag_filter{VK_FILTER_NEAREST};
	VkSamplerMipmapMode mip_filter{VK_SAMPLER_MIPMAP_MODE_NEAREST};
	VkSamplerAddressMode address_mode_u{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_v{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_w{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerReductionMode reduction_mode{VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE};
};

template <>
struct ResourceTraits<Sampler>
{
	static constexpr ResourceType type = ResourceType::Sampler;
};

struct DeviceData;
struct ResourceData;

ResourceHandle CreateVkSampler(const DeviceData &device_data,
							   ResourceData &resource_data,
							   const SamplerCreation &creation);

void DestroyVkSampler(const ResourceHandle &handle,
					  const DeviceData &device_data,
					  ResourceData &resource_data);

} // namespace cloud::vulkan