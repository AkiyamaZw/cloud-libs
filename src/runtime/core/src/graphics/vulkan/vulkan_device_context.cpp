#include "graphics/vulkan/vulkan_device_context.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "core/runtime_log.h"

namespace cloud::vulkan
{

void Init(VulkanDeviceContext &vdc, GpuCreateParam &param)
{
	INFO("[Vulkan Gpu Device] Start init...");
	using namespace infra;
	CreateVkInstance(vdc.instance_data, param);
	CreateVkWindowSurfaceFromGlfw(vdc.instance_data, param, vdc.window_data);
	CreateVkPhysicalDevice(vdc.instance_data, vdc.window_data, vdc.device_data);
	CreateVkDeviceAndQueue(vdc.device_data);
	CreateVkSwapChain(vdc.device_data, vdc.window_data);
	CreateVmaAllocator(vdc.instance_data, vdc.device_data, vdc.resource_data);
	CreateVkDescriptorPool(vdc.device_data, vdc.resource_data);
	CreateVkQueryPool(param, vdc.device_data);
	CreateVkSyncMarkers(vdc.device_data, vdc.runtime_data);
	InitRuntimeLoopData(vdc.device_data, vdc.runtime_data);
	InitDefaultResource(vdc);
}

void Shutdown(VulkanDeviceContext &vdc)
{
	using namespace infra;

	vkDeviceWaitIdle(vdc.device_data.device);

	DestoryDefaultResource(vdc);

	DestoryRuntimeLoopData(vdc.device_data, vdc.runtime_data);
	DestroyVkSyncMarkers(vdc.device_data, vdc.runtime_data);
	DestroyVkQueryPool(vdc.device_data);
	DestroyVkDescriptorPool(vdc.device_data, vdc.resource_data);
	DestroyVkSwapchain(vdc.device_data, vdc.window_data);

	DestroyResourceInstance(vdc.runtime_data, vdc.device_data, vdc.resource_data);

	/* resource should be clear upper */
	DestroyVmaAllocator(vdc.resource_data);
	DestroyVkDeviceAndQueue(vdc.device_data);
	DestroyWindowSurface(vdc.instance_data, vdc.window_data);
	DestroyVkInstance(vdc.instance_data);
}

void InitDefaultResource(VulkanDeviceContext &vdc)
{
	SamplerCreation sc{};
	sc.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.min_filter = VK_FILTER_LINEAR;
	sc.mag_filter = VK_FILTER_LINEAR;
	sc.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sc.name = "default_sampler";
	vdc.resource_data.default_sampler =
		infra::CreateVkSampler(vdc.device_data, vdc.resource_data, sc);

	BufferCreation bc{};
	bc.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bc.usage_type = ResourceUsageType::Immutable, bc.size = 0;
	bc.initial_data = nullptr;
	bc.name = "fullscreen_vb";
	vdc.resource_data.fullscreen_vertex_buffer =
		infra::CreateVkBuffer(bc, vdc.device_data, vdc.resource_data);

	TextureCreation tc{};
	tc.initial_data = nullptr;
	tc.height = vdc.window_data.swapchain_height;
	tc.width = vdc.window_data.swapchain_width;
	tc.depth = 1;
	tc.mipmaps = 1;
	tc.flags = 0;
	tc.format = VK_FORMAT_D32_SFLOAT;
	tc.type = TextureType::Texture2D;
	tc.name = "depth_texture";
	vdc.resource_data.texture_depth_handle =
		infra::CreateVkTexture(vdc.device_data, vdc.runtime_data, tc, vdc.resource_data);

	vdc.window_data.swapchain_output.SetDepthFormat(VK_FORMAT_D32_SFLOAT);
	RenderPassCreation rpc = {};
	rpc.type = RenderPassType::SwapChain;
	rpc.name = "swapchain";
	rpc.color_op = RenderPassOperation::Clear;
	rpc.depth_op = RenderPassOperation::Clear;
	rpc.stencil_op = RenderPassOperation::Clear;
	vdc.resource_data.swapchain_pass = infra::CreateVkRenderPass(
		rpc, vdc.device_data, vdc.runtime_data, vdc.window_data, vdc.resource_data);

	BufferCreation dynamic_bc = {};
	dynamic_bc.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
							 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	dynamic_bc.usage_type = ResourceUsageType::Immutable;
	dynamic_bc.size = 1024 * 1024 * 10 * MaxSwapchainImages;
	dynamic_bc.name = "dynamic_buffer";
	DynamicBuffer &dynamic_buffer = vdc.resource_data.dynamic_buffer;
	dynamic_buffer.buffer = infra::CreateVkBuffer(dynamic_bc, vdc.device_data, vdc.resource_data);
	dynamic_buffer.mapped_memory = (uint8_t *)infra::MapBuffer(
		{dynamic_buffer.buffer, 0, 0}, dynamic_buffer, vdc.resource_data);
}

void DestoryDefaultResource(VulkanDeviceContext &vdc)
{
	DynamicBuffer &db = vdc.resource_data.dynamic_buffer;
	infra::UnMapBuffer({db.buffer, 0, 0}, db, vdc.resource_data);
	infra::DestroyVkRenderPass(vdc.resource_data.swapchain_pass, vdc.runtime_data);
	infra::DestroyVkTexture(vdc.resource_data.texture_depth_handle, vdc.runtime_data);
	infra::DestroyVkBuffer(vdc.resource_data.fullscreen_vertex_buffer, vdc.runtime_data);
	infra::DestroyVkSampler(vdc.resource_data.default_sampler, vdc.runtime_data);
}

} // namespace cloud::vulkan
