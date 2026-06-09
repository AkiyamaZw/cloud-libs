#pragma once

#include <string_view>

#include "graphics/core/gpu_enum.h"

namespace cloud::render
{

using ResourceHandleIndex = uint32_t;
static constexpr uint32_t INVALIDResourceHandleIndex = 0xffffffff;

enum class ResourceType : uint8_t
{
	Buffer,
	Texture,
	Pipeline,
	Sampler,
	DescriptorSetLayout,
	DescriptorSet,
	RenderPass,
	ShaderState,
	Count
};

struct ResourceHandle
{
	ResourceHandleIndex index{INVALIDResourceHandleIndex};
	ResourceType type{ResourceType::Count};

	ResourceHandle() = default;

	ResourceHandle(ResourceHandleIndex InIndex, ResourceType InType)
		: index(InIndex)
		, type(InType) {};

	ResourceHandle(const ResourceHandle &rhs)
	{
		index = rhs.index;
		type = rhs.type;
	}

	ResourceHandle(ResourceHandle &&rhs)
	{
		index = rhs.index;
		type = rhs.type;
		rhs.index = -1;
		type = ResourceType::Count;
	}

	ResourceHandle &operator=(const ResourceHandle &rhs)
	{
		if (this == &rhs)
			return *this;
		this->index = rhs.index;
		this->type = rhs.type;
		return *this;
	}

	// ResourceHandle &operator=(ResourceHandle &&rhs) = delete;
};

struct MapBufferParameter
{
	ResourceHandle handle;
	uint32_t offset{0};
	uint32_t size{0};
};

struct SamplerCreation
{
	std::string_view name;
	FilterMode min_filter;
	FilterMode mag_filter;
	FilterMode mip_filter;
	AddressMode address_mode_u;
	AddressMode address_mode_v;
	AddressMode address_mode_w;
	ReductionMode reduction_mode;
};
} // namespace cloud::render