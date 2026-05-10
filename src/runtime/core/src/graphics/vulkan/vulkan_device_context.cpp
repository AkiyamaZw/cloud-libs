#include "graphics/vulkan/vulkan_device_context.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "core/runtime_log.h"
#include <algorithm>
#include <fstream>
#include <string>

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

	// CreateQueryPool(param);
	// assert(timestamp_query_pool);

	// CreateRenderPass();
	// assert(render_pass);

	// CreateFramebuffers();
	// assert(swapchain_framebuffers[0]);

	// CreatePipelineLayout();
	// assert(pipeline_layout);

	// CreateGraphicsPipeline();
	// assert(graphics_pipeline);

	// gpu_resource_manager = std::make_unique<GPUResourceManager>(device,
	// 															vma_allocator,
	// 															descriptor_pool,
	// 															current_frame,
	// 															device_resource,
	// 															default_resource,
	// 															resource_deletion_queue,
	// 															descriptor_set_updates,
	// 															debug_utils_extension_present);

	// // 初始化时间
	// start_time = std::chrono::high_resolution_clock::now();
	// CreateSyncMarkers();
	// assert(render_complete_semaphore[0]);
	// assert(image_acquired_semaphore[0]);
	// assert(command_buffer_fence[0]);
	// g_vulkan_cmd_buffer_ring.Init(device, queue_family);

	// SamplerCreation sc{};
	// sc.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	// sc.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	// sc.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	// sc.min_filter = VK_FILTER_LINEAR;
	// sc.mag_filter = VK_FILTER_LINEAR;
	// sc.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	// sc.name = "Sampler Default";
	// default_sampler = gpu_resource_manager->CreateSampler(sc);
}

void Shutdown(VulkanDeviceContext &vdc)
{
	using namespace infra;
	DestroyVmaAllocator(vdc.resource_data);
	DestroyVkSwapchain(vdc.device_data, vdc.window_data);
	DestroyVkDeviceAndQueue(vdc.device_data);
	DestroyWindowSurface(vdc.instance_data, vdc.window_data);
	DestroyVkInstance(vdc.instance_data);
	// 	// 等待所有命令缓冲区完成执行
	// 	for (uint32_t i = 0; i < MaxSwapchainImages; ++i)
	// 	{
	// 		vkWaitForFences(device, 1, &command_buffer_fence[i], VK_TRUE, UINT64_MAX);
	// 	}

	// 	// 等待所有队列操作完成
	// 	vkDeviceWaitIdle(device);

	// 	g_vulkan_cmd_buffer_ring.Destroy(device);
	// 	render::MapBufferParameter map_buffer_param{device_resource.dynamic_buffer, 0, 0};
	// 	gpu_resource_manager->UnMapBuffer(map_buffer_param);

	// 	// 销毁uniform buffer
	// 	gpu_resource_manager->DestroyBuffer(uniform_buffer, current_frame);

	// 	// 销毁dynamic buffer和sampler
	// 	gpu_resource_manager->DestroyBuffer(device_resource.dynamic_buffer, current_frame);
	// 	gpu_resource_manager->DestroySampler(default_sampler, current_frame);

	// 	// 释放deletion queue中的资源
	// 	gpu_resource_manager->ReleaseResourcesInDeletionQueue();

	// 	device_resource.samplers.Shutdown();
	// 	device_resource.pipelines.Shutdown();
	// 	device_resource.shaders.Shutdown();
	// 	device_resource.descriptor_sets.Shutdown();

	// 	// 销毁渲染相关资源
	// 	vkDestroyPipeline(device, graphics_pipeline, nullptr);

	// 	// 销毁descriptor set
	// 	vkFreeDescriptorSets(device, descriptor_pool, 1, &descriptor_set);

	// 	// 销毁descriptor set layout
	// 	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);

	// 	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
	// 	vkDestroyRenderPass(device, render_pass, nullptr);

	// 	DestroySyncMarkers();
	// 	vkDestroyQueryPool(device, timestamp_query_pool, nullptr);
	// 	vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	// 	vmaDestroyAllocator(vma_allocator);
	// 	DestroySwapChain();
	// 	vkDestroyDevice(device, nullptr);
	// 	vkDestroySurfaceKHR(instance, window_surface, nullptr);

	// #if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	// 	auto vkDestroyDebugUtilsMessengerEXT =
	// 		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
	// 			instance, "vkDestroyDebugUtilsMessengerEXT");
	// 	vkDestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger, nullptr);
	// #endif
	// vkDestroyInstance(instance, nullptr);
}

} // namespace cloud::vulkan
