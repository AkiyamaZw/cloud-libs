#include "graphics/vulkan/gpu_enums.h"
#include "core/runtime_log.h"

namespace cloud::vulkan
{
void ToVKEnum(const render::FilterMode mode, VkFilter &out_filter)
{
	switch (mode)
	{
	case render::FilterMode::Point:
		out_filter = VK_FILTER_NEAREST;
		break;
	case render::FilterMode::Linear:
		out_filter = VK_FILTER_LINEAR;
		break;
	}
}
void ToVKEnum(const render::FilterMode mode, VkSamplerMipmapMode &out_mipmap_mode)
{
	switch (mode)
	{
	case render::FilterMode::Point:
		out_mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case render::FilterMode::Linear:
		out_mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	}
}

void ToVKEnum(const render::AddressMode mode, VkSamplerAddressMode &out_address_mode)
{
	switch (mode)
	{
	case render::AddressMode::Wrap:
		out_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case render::AddressMode::Mirror:
		out_address_mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		break;
	case render::AddressMode::Clamp:
		out_address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case render::AddressMode::Border:
		out_address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		break;
	case render::AddressMode::MirrorOnce:
		out_address_mode = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		break;
	}
}

void ToVKEnum(const render::ReductionMode mode, VkSamplerReductionMode &out_reduction_mode)
{
	switch (mode)
	{

	case render::ReductionMode::Filter:
		out_reduction_mode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
		break;
	case render::ReductionMode::Comparison:
		WARN("ReductionMode not found equal enum from ReductionMode::Comparison use "
			 "ReductionMode::Filter");
		out_reduction_mode = VK_SAMPLER_REDUCTION_MODE_MAX;
		break;
	case render::ReductionMode::Minimum:
		out_reduction_mode = VK_SAMPLER_REDUCTION_MODE_MIN;
		break;
	case render::ReductionMode::Maximum:
		out_reduction_mode = VK_SAMPLER_REDUCTION_MODE_MAX;
		break;
	}
}

void ToVKEnum(const TextureType::Enum type, VkImageType &out_type)
{
	static VkImageType s_vk_target[render::TextureType::Count] = {VK_IMAGE_TYPE_1D,
																  VK_IMAGE_TYPE_2D,
																  VK_IMAGE_TYPE_3D,
																  VK_IMAGE_TYPE_2D,
																  VK_IMAGE_TYPE_1D,
																  VK_IMAGE_TYPE_2D,
																  VK_IMAGE_TYPE_3D};

	out_type = s_vk_target[type];
}

} // namespace cloud::vulkan