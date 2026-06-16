#pragma once

#include <vector>
#include <array>
#include <map>
#include <chrono>
#include <unordered_map>
#include "graphics/vulkan/command_buffer.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/resource.h"

namespace cloud::vulkan
{
static constexpr uint32_t s_swapchain_image_count = 3;

struct InstanceData
{
	VkInstance instance;
	VkDebugReportCallbackEXT debug_callback{VK_NULL_HANDLE};
	VkDebugUtilsMessengerEXT debug_utils_messenger{VK_NULL_HANDLE};
	std::vector<const char *> enabled_extensions;
	std::vector<const char *> enabled_layers;
	bool debug_utils_extension_present{false};
};

struct DeviceData
{
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	float gpu_timestamp_frequency;
	uint64_t ubo_alignment{256};
	uint64_t ssbo_alignment{256};
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkQueryPool timestamp_query_pool;
	VkAllocationCallbacks *allocation_callback{nullptr};
};

struct WindowData
{
	VkSurfaceKHR window_surface;
	VkSurfaceFormatKHR window_surface_format;
	uint32_t swapchain_width;
	uint32_t swapchain_height;

	PresentMode present_mode{PresentMode::VSync};
	VkPresentModeKHR vk_present_mode;

	RenderPassOutput swapchain_output;
	VkSwapchainKHR vk_swapchain;
	std::array<VkImage, MaxSwapchainImages> swapchain_images;
	std::array<VkImageView, MaxSwapchainImages> swapchain_image_views;
	std::array<VkFramebuffer, MaxSwapchainImages> swapchain_framebuffers;
	uint32_t swapchain_image_count{s_swapchain_image_count};
	uint32_t vulkan_image_index{0};

	bool resized{false};
};

struct RenderPipelineData
{
	// VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
};

struct DynamicBuffer
{
	uint32_t max_per_frame_size{0};
	ResourceHandle buffer;
	uint8_t *mapped_memory{nullptr};
	uint32_t allocated_size{0};
	uint32_t per_frame_size{1024 * 1024 * 10};

	struct MapBufferParameters
	{
		ResourceHandle handle;
		uint32_t offset{0};
		uint32_t size{0};
	};
};

struct ResourceData
{
	VkAllocationCallbacks *allocation_callback{nullptr};
	VmaAllocator vma_allocator{nullptr};
	DeviceResourcePoolData pool_data;
	ResourceHandle fullscreen_vertex_buffer;
	ResourceHandle default_sampler;
	ResourceHandle texture_depth_handle;
	ResourceHandle swapchain_pass;
	DynamicBuffer dynamic_buffer;
	std::map<size_t, VkRenderPass> render_pass_cache;
};

// sync marker
struct SyncSignal
{
	std::array<VkSemaphore, MaxSwapchainImages> render_complete_semaphore;
	std::array<VkSemaphore, MaxSwapchainImages> image_acquired_semaphore;
	std::array<VkFence, MaxSwapchainImages> command_buffer_fence;
};

// ticker counter in renderer
struct FrameAdanceCounter
{
	uint32_t current_frame{0};
	uint32_t previous_frame{0};
	uint64_t absolute_frame{0};
	bool timestamps_enabled{false};
	std::chrono::high_resolution_clock::time_point start_time;
};

void AdvanceFrameCounter(FrameAdanceCounter &counter);

struct RuntimeLoopData
{
	// sync mark
	SyncSignal sync_signal;
	CommandBufferRing command_buffer_ring;
	FrameAdanceCounter frame_counter;

	std::vector<ResourceUpdate> resource_deletion_queue;
	std::vector<DescriptorSetUpdate> descriptor_set_updates;

	std::array<CommandBuffer *, 128> queued_command_buffers;
	uint32_t num_allocated_command_buffers{0};
	uint32_t num_queued_command_buffers{0};
};

bool InitializeContextInstance(InstanceData &context, const GpuCreateParam &param);
bool DestroyContextInstance(InstanceData &context);

} // namespace cloud::vulkan