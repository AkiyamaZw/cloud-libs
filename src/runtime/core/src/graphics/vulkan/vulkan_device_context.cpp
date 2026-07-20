#include "graphics/vulkan/vulkan_device_context.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "core/runtime_log.h"
#include <algorithm>

namespace cloud::vulkan
{
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
	vdc.resource_data.default_sampler = CreateVkSampler(vdc.device_data, vdc.resource_data, sc);

	BufferCreation bc{};
	bc.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bc.usage_type = ResourceUsageType::Immutable, bc.size = 0;
	bc.initial_data = nullptr;
	bc.name = "fullscreen_vb";
	vdc.resource_data.fullscreen_vertex_buffer =
		CreateVkBuffer(bc, vdc.device_data, vdc.resource_data);

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
		CreateVkTexture(vdc.device_data, vdc.runtime_data, tc, vdc.resource_data);

	vdc.window_data.swapchain_output.SetDepthFormat(VK_FORMAT_D32_SFLOAT);
	RenderPassCreation rpc = {};
	rpc.type = RenderPassType::SwapChain;
	rpc.name = "swapchain_pass";
	rpc.color_op = RenderPassOperation::Clear;
	rpc.depth_op = RenderPassOperation::Clear;
	rpc.stencil_op = RenderPassOperation::Clear;
	vdc.resource_data.swapchain_pass = CreateVkRenderPass(
		rpc, vdc.device_data, vdc.runtime_data, vdc.window_data, vdc.resource_data);

	BufferCreation dynamic_bc = {};
	dynamic_bc.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
							 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	dynamic_bc.usage_type = ResourceUsageType::Immutable;
	dynamic_bc.size = 1024 * 1024 * 10 * MaxSwapchainImages;
	dynamic_bc.name = "dynamic_buffer";
	DynamicBuffer &dynamic_buffer = vdc.resource_data.dynamic_buffer;
	dynamic_buffer.buffer = CreateVkBuffer(dynamic_bc, vdc.device_data, vdc.resource_data);
	dynamic_buffer.mapped_memory = (uint8_t *)infra::MapBuffer(
		{dynamic_buffer.buffer, 0, 0}, dynamic_buffer, vdc.resource_data);
}

void DestoryDefaultResource(VulkanDeviceContext &vdc)
{
	DynamicBuffer &db = vdc.resource_data.dynamic_buffer;
	infra::UnMapBuffer({db.buffer, 0, 0}, db, vdc.resource_data);
	infra::PendingToQueue(vdc.runtime_data, vdc.resource_data.dynamic_buffer.buffer);
	infra::PendingToQueue(vdc.runtime_data, vdc.resource_data.swapchain_pass);
	infra::PendingToQueue(vdc.runtime_data, vdc.resource_data.texture_depth_handle);
	infra::PendingToQueue(vdc.runtime_data, vdc.resource_data.fullscreen_vertex_buffer);
	infra::PendingToQueue(vdc.runtime_data, vdc.resource_data.default_sampler);
}

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

	DestroyResourceInQueue(vdc.runtime_data, vdc.device_data, vdc.resource_data);

	/* resource should be clear upper */
	DestroyVmaAllocator(vdc.resource_data);
	DestroyVkDeviceAndQueue(vdc.device_data);
	DestroyWindowSurface(vdc.instance_data, vdc.window_data);
	DestroyVkInstance(vdc.instance_data);
}

void StartFrame(VulkanDeviceContext &vdc)
{
	auto &sync_signal = vdc.runtime_data.sync_signal;
	FrameAdanceCounter &time_counter = vdc.runtime_data.frame_counter;
	auto &device = vdc.device_data.device;
	VkFence *render_complete_fence = &sync_signal.command_buffer_fence[time_counter.current_frame];
	if (vkGetFenceStatus(device, *render_complete_fence) != VK_SUCCESS)
	{
		vkWaitForFences(device, 1, render_complete_fence, VK_TRUE, UINT64_MAX);
	}
	vkResetFences(device, 1, render_complete_fence);

	VkSemaphore image_accuired_semaphore =
		sync_signal.image_acquired_semaphore[time_counter.current_frame];
	VkResult succ = vkAcquireNextImageKHR(device,
										  vdc.window_data.vk_swapchain,
										  UINT64_MAX,
										  image_accuired_semaphore,
										  VK_NULL_HANDLE,
										  &vdc.window_data.vulkan_image_index);
	if (succ == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// infra::resize_swapchain();
		assert(false);
	}

	vdc.runtime_data.command_buffer_ring.Reset(device, time_counter.current_frame);
	vdc.resource_data.dynamic_buffer.AdvanceSlot(time_counter.previous_frame,
												 time_counter.current_frame);

	auto &descriptor_set_container = vdc.runtime_data.descriptor_set_updates;
	if (!descriptor_set_container.empty())
	{
		for (uint32_t i = descriptor_set_container.size() - 1; i >= 0; --i)
		{
			DescriptorSetUpdate &update = descriptor_set_container[i];

			infra::update_descriptor_set_instance(
				vdc.device_data, vdc.runtime_data, vdc.resource_data, update);
			update.current_frame = UINT32_MAX;
			descriptor_set_container.pop_back();
		}
	}
}

void present(VulkanDeviceContext &vdc)
{
	FrameAdanceCounter &frame_counter = vdc.runtime_data.frame_counter;
	SyncSignal &sync_signal = vdc.runtime_data.sync_signal;
	uint32_t current_frame = frame_counter.current_frame;
	VkFence *render_complete_fence = &sync_signal.command_buffer_fence[current_frame];
	VkSemaphore *render_complete_semaphore = &sync_signal.render_complete_semaphore[current_frame];
	VkCommandBuffer enqueued_command_buffers[4];

	auto &queued_command_buffers = vdc.runtime_data.queued_command_buffers;
	for (uint32_t c = 0; c < vdc.runtime_data.num_queued_command_buffers; ++c)
	{
		CommandBuffer *command_buffer = queued_command_buffers[c];
		enqueued_command_buffers[c] = command_buffer->vk_command_buffer;

		if (command_buffer->is_recoding && command_buffer->current_render_pass &&
			(command_buffer->current_render_pass->type != RenderPassType::Compute))
		{
			vkCmdEndRenderPass(command_buffer->vk_command_buffer);
		}
		vkEndCommandBuffer(command_buffer->vk_command_buffer);
	}

	VkSemaphore wait_semaphore[] = {sync_signal.image_acquired_semaphore[current_frame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphore;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = vdc.runtime_data.num_queued_command_buffers;
	submit_info.pCommandBuffers = enqueued_command_buffers;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = render_complete_semaphore;

	vkQueueSubmit(vdc.device_data.queue, 1, &submit_info, *render_complete_fence);

	VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = render_complete_semaphore;

	VkSwapchainKHR swap_chains[] = {vdc.window_data.vk_swapchain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &vdc.window_data.vulkan_image_index;
	present_info.pResults = nullptr;
	VkResult succ = vkQueuePresentKHR(vdc.device_data.queue, &present_info);

	vdc.runtime_data.num_queued_command_buffers = 0;

	if (succ == VK_ERROR_OUT_OF_DATE_KHR || succ == VK_SUBOPTIMAL_KHR || vdc.window_data.resized)
	{
		vdc.window_data.resized = false;
		// todo resize swapchain
		AdvanceFrameCounter(frame_counter);
		return;
	}

	if (frame_counter.timestamps_enabled)
	{
		assert(false);
	}
	AdvanceFrameCounter(frame_counter);

	DestroyResource(vdc.runtime_data, vdc.device_data, vdc.resource_data);
}

} // namespace cloud::vulkan
