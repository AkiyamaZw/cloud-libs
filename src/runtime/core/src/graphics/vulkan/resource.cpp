#include "graphics/vulkan/resource.h"
#include <functional>
#include <unordered_map>
#include <cassert>
#include "graphics/vulkan/device_data.h"
#include "graphics/vulkan/resource/buffer.h"
#include "graphics/vulkan/resource/texture.h"

namespace cloud::vulkan
{
using instance_delete_handler =
	std::function<void(const ResourceHandle &, const DeviceData &device_data, ResourceData &)>;
std::unordered_map<ResourceType, instance_delete_handler> s_delete_map = {
	{ResourceType::Buffer, DestroyVkBuffer},
	{ResourceType::Texture, DestroyVkTexture},
	{ResourceType::Sampler, DestroyVkSampler},
	{ResourceType::RenderPass, DestroyVkRenderPass},
	{ResourceType::DescriptorSet, DestroyVkDescriptorSet}};

void DestroyResource(RuntimeLoopData &rl_data,
					 const DeviceData &device_data,
					 ResourceData &resource_data)
{
	auto &container = rl_data.resource_deletion_queue;

	for (int i = (int)container.size() - 1; i >= 0; --i)
	{
		ResourceUpdate &res_to_delete = container[i];

		if (res_to_delete.current_frame != InvalidFrameID)
		{
			auto iter = s_delete_map.find(res_to_delete.handle.type);
			if (iter != s_delete_map.end())
			{
				const auto &handle = res_to_delete.handle;
				iter->second(res_to_delete.handle, device_data, resource_data);
				container.pop_back();
			}
			else
			{
				FATAL("resource type %d has not delete instance handler!",
					  (uint32_t)res_to_delete.handle.type);
			}
		}
		else
		{
			assert(false);
		}
	}
	assert(container.empty());

	auto &rp_cache = resource_data.render_pass_cache;
	auto rp_cache_iter = rp_cache.begin();
	while (rp_cache_iter != rp_cache.end())
	{
		VkRenderPass vk_rp = rp_cache_iter->second;
		vkDestroyRenderPass(device_data.device, vk_rp, device_data.allocation_callback);
		rp_cache_iter++;
	}
	rp_cache.clear();
}
} // namespace cloud::vulkan