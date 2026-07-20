#include "graphics/vulkan/resource/sampler.h"
#include "core/data_structure/resource_pool.h"
#include "graphics/vulkan/device_data.h"
#include "graphics/vulkan/vulkan_interface.h"

namespace cloud::vulkan
{
void TranslateSamplerCreation(const SamplerCreation &creation,
							  VkSamplerCreateInfo &sampler_create_info)
{
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.addressModeU = creation.address_mode_u;
	sampler_create_info.addressModeV = creation.address_mode_v;
	sampler_create_info.addressModeW = creation.address_mode_w;
	sampler_create_info.minFilter = creation.min_filter;
	sampler_create_info.magFilter = creation.mag_filter;
	sampler_create_info.mipmapMode = creation.mip_filter;
	sampler_create_info.anisotropyEnable = 0;
	sampler_create_info.compareEnable = 0;
	sampler_create_info.unnormalizedCoordinates = 0;
	sampler_create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
}

void CreateSampler(VkDevice device, const SamplerCreation &creation, VkSampler &sampler)
{
	VkSamplerCreateInfo create_info;
	TranslateSamplerCreation(creation, create_info);
	auto succ = vkCreateSampler(device, &create_info, nullptr, &sampler);
	check_vk(succ);
}

ResourceHandle CreateVkSampler(const DeviceData &device_data,
							   ResourceData &resource_data,
							   const SamplerCreation &creation)
{
	Sampler *sampler =
		infra::AllocResource<Sampler>(resource_data, ResourceType::Sampler, creation.name);
	VkSamplerCreateInfo create_info{};
	TranslateSamplerCreation(creation, create_info);
	auto succ = vkCreateSampler(device_data.device, &create_info, nullptr, &sampler->sampler);
	check_vk(succ);
	infra::SetResourceName(device_data.device,
						   VK_OBJECT_TYPE_SAMPLER,
						   reinterpret_cast<uint64_t>(sampler->sampler),
						   creation.name);
	return sampler->handle;
}

void DestroyVkSampler(const ResourceHandle &handle,
					  const DeviceData &device_data,
					  ResourceData &resource_data)
{
	auto sampler = infra::Access<Sampler>(resource_data, handle);
	if (sampler)
	{
		vkDestroySampler(device_data.device, sampler->sampler, nullptr);
	}
	infra::ReleaseResourceBase(resource_data, sampler);
}
} // namespace cloud::vulkan