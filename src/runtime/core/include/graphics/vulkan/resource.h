#pragma once
#include <array>
#include <utility>
#include <array>
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/gpu_resource.h"
#include "core/data_structure/resource_pool.h"
#include "core/runtime_log.h"
#include "graphics/vulkan/resource/buffer.h"
#include "graphics/vulkan/resource/texture.h"
#include "graphics/vulkan/resource/sampler.h"
#include "graphics/vulkan/resource/renderpass.h"
#include "graphics/vulkan/resource/descriptorset.h"

namespace cloud::vulkan
{

struct DeviceResourcePoolData
{
	using ResourcePool = cloud::ResourcePool;
	static constexpr uint32_t ResourceTypeCount = std::to_underlying(ResourceType::Count);
	// resource obejct pool
	static constexpr uint32_t buffer_pool_size = 4096;
	static constexpr uint32_t texture_pool_size = 512;
	static constexpr uint32_t render_pass_pool_size = 256;
	static constexpr uint32_t descriptor_layout_pool_size = 128;
	static constexpr uint32_t pipeline_pool_size = 128;
	static constexpr uint32_t shader_pool_size = 128;
	static constexpr uint32_t descriptor_set_pool_size = 256;
	static constexpr uint32_t sampler_pool_size = 32;

	std::array<ResourcePool, ResourceTypeCount> resource_pool_array{
		ResourcePool{buffer_pool_size, sizeof(Buffer)},
		ResourcePool{texture_pool_size, sizeof(Texture)},
		ResourcePool{pipeline_pool_size, sizeof(Pipeline)},
		ResourcePool{sampler_pool_size, sizeof(Sampler)},
		ResourcePool{descriptor_layout_pool_size, sizeof(DescriptorSetLayout)},
		ResourcePool{descriptor_set_pool_size, sizeof(DescriptorSet)},
		ResourcePool{render_pass_pool_size, sizeof(RenderPass)},
		ResourcePool{shader_pool_size, sizeof(ShaderState)}};

	// descriptor pool
	static constexpr uint32_t k_global_pool_elements = 128;
	static constexpr VkDescriptorPoolSize pool_sizes[] = {
		{VK_DESCRIPTOR_TYPE_SAMPLER, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, k_global_pool_elements},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, k_global_pool_elements}};
	VkDescriptorPool vk_descriptor_pool;
};

template <typename T>
ResourcePool &GetResourcePool(DeviceResourcePoolData &pools)
{ return pools.resource_pool_array[std::to_underlying(ResourceTraits<T>::type)]; }

template <typename T>
ResourceHandle FetchResource(DeviceResourcePoolData &pools)
{ return ResourceHandle{GetResourcePool<T>(pools).FetchResource(), ResourceTraits<T>::type}; }

template <typename T>
T *Access(DeviceResourcePoolData &pools, const ResourceHandle &handle)
{ return static_cast<T *>(GetResourcePool<T>(pools).Access(handle.index)); }

template <typename T>
T *AllocResource(DeviceResourcePoolData &pools, const char *name)
{
	ResourceHandle handle = FetchResource<T>(pools);
	if (handle.index == ResourcePool::INVALID_NUM)
		return nullptr;
	T *res = Access<T>(pools, handle);
	res->handle = handle;
	res->name = name;
	INFO("Resource \"%s\" created with handle %d", name, handle.index);
	return res;
}

struct RuntimeLoopData;
struct DeviceData;
void DestroyResource(RuntimeLoopData &rl_data,
					 const DeviceData &device_data,
					 ResourceData &resource_data);

} // namespace cloud::vulkan
