#pragma once
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct TextureCreation
{
	void *initial_data{nullptr};
	uint16_t width = 1;
	uint16_t height = 1;
	uint16_t depth = 1;
	uint8_t mipmaps = 1;
	uint8_t flags = 1; // bitmask
	VkFormat format{VK_FORMAT_UNDEFINED};
	TextureType type{TextureType::Texture2D};
	const char *name{nullptr};
};

struct Sampler;
struct Texture : public ResourceBase
{
	ResourceHandle parent_handle;
	VkImage image;
	VkImageView view;
	VkFormat format;
	VkImageLayout layout;
	VkImageUsageFlags usage_flags;
	VmaAllocation allocation;
	ResourceState state{ResourceState::RESOURCE_STATE_UNDEFINED};
	uint16_t width{1};
	uint16_t height{1};
	uint16_t depth{1};
	uint8_t mipmaps{1};
	uint8_t flags{0};
	uint16_t mip_base_levels{0};
	uint16_t array_base_layer{0};
	bool sparse{false};
	TextureType type{TextureType::Texture2D};
	Sampler *sampler;
};

template <>
struct ResourceTraits<Texture>
{
	static constexpr ResourceType type = ResourceType::Texture;
};

struct DeviceData;
struct ResourceData;
struct RuntimeLoopData;

ResourceHandle CreateVkTexture(const DeviceData &device_data,
							   RuntimeLoopData &rl_data,
							   const TextureCreation &creation,
							   ResourceData &resource_data);

void DestroyVkTexture(const ResourceHandle &handle,
					  const DeviceData &device_data,
					  ResourceData &resource_data);
} // namespace cloud::vulkan