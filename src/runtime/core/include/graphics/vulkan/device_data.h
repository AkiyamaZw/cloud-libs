#pragma
#include <vector>
#include <array>
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
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
	VkSwapchainKHR swapchain;
	std::array<VkImage, MaxSwapchainImages> swapchain_images;
	std::array<VkImageView, MaxSwapchainImages> swapchain_image_views;
	std::array<VkFramebuffer, MaxSwapchainImages> swapchain_framebuffers;
	uint32_t swapchain_image_count;
};

struct RenderPipelineData
{
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
};

struct ResourceData
{
	VmaAllocator vma_allocator;
};


struct CommandBuffer
{
	std::array<CommandBuffer *, 128> queued_command_buffers;
	uint32_t num_allocated_command_buffers{0}; // not used now
	uint32_t num_queued_command_buffers{0};
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
};


bool InitializeContextInstance(InstanceData &context,  const GpuCreateParam &param);
bool DestroyContextInstance(InstanceData &context);

} // namespace cloud::vulkan