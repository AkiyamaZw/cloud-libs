#pragma once
#include "gpu_resource_manager.h"
#include <memory>
#include <vector>
#include <array>
#include "data_structure/resource_pool.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/command_buffer.h"
#include "graphics/vulkan/vk_mem_alloc.h"
#include "graphics/device.h"

namespace cloud::vulkan
{
struct DeviceBase;
class GPUResourceManager;

struct ComponentResource
{
	static constexpr uint32_t buffer_pool_size = 4096;
	cloud::ResourcePool buffers{buffer_pool_size, sizeof(Buffer)};
	static constexpr uint32_t texture_pool_size = 512;
	cloud::ResourcePool textures{texture_pool_size, sizeof(Texture)};
	static constexpr uint32_t render_pass_pool_size = 256;
	cloud::ResourcePool render_passes{render_pass_pool_size, sizeof(RenderPass)};
	static constexpr uint32_t descriptor_layout_pool_size = 128;
	cloud::ResourcePool descriptor_set_layout{descriptor_layout_pool_size,
											  sizeof(DescriptorSetLayout)};
	static constexpr uint32_t pipeline_pool_size = 128;
	cloud::ResourcePool pipelines{pipeline_pool_size, sizeof(Pipeline)};
	static constexpr uint32_t shader_pool_size = 128;
	cloud::ResourcePool shaders{shader_pool_size, sizeof(ShaderState)};
	static constexpr uint32_t descriptor_set_pool_size = 256;
	cloud::ResourcePool descriptor_sets{descriptor_set_pool_size, sizeof(DescriptorSet)};
	static constexpr uint32_t sampler_pool_size = 32;
	cloud::ResourcePool samplers{sampler_pool_size, sizeof(Sampler)};

	uint32_t dynamic_max_per_frame_size{0};
	BufferHandle dynamic_buffer;
	uint8_t *dynamic_mapped_memory{nullptr};
	uint32_t dynamic_allocated_size{0};
	uint32_t dynamic_per_frame_size{1024 * 1024 * 10};
};

struct CommandBufferRing
{
	static const uint16_t GMaxThreads = 1;
	static const uint16_t GMaxPools = MaxSwapchainImages * GMaxThreads;
	static const uint16_t GBuffersPerPool = 4;
	static const uint16_t GMaxBuffers = GBuffersPerPool * GMaxPools;

	VkCommandPool vk_command_pool[GMaxPools];
	CommandBuffer command_buffer[GMaxBuffers];
	uint8_t next_free_per_thread_frame[GMaxPools];
	void Init(DeviceBase *gpu);
	void Destroy(DeviceBase *gpu);
	void Reset(DeviceBase *gpu, uint32_t frame_index);
	static uint32_t IndexInPool(uint32_t index) { return (uint16_t)index / GBuffersPerPool; }
	CommandBuffer *GetCommandBuffer(uint32_t frame_index, bool begin);
	CommandBuffer *GetCommandBufferInstant(uint32_t frame_index, bool begin);
};

/* this class define:
 * 1. data used in  gpudevice
 * 2. manager class interest in operating part of data
 */
struct VulaknDeviceContext
{
	struct InstanceData
	{
		VkInstance instance;
		VkDebugReportCallbackEXT debug_callback{VK_NULL_HANDLE};
		VkDebugUtilsMessengerEXT debug_utils_messenger{VK_NULL_HANDLE};
		std::vector<const char *> enabled_extensions;
		std::vector<const char *> enabled_layers;
		bool debug_utils_extension_present{false};
	} instance_data;
};

struct DeviceBase
{
	virtual ~DeviceBase();
	/* basic api object */
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	float gpu_timestamp_frequency;
	uint64_t ubo_alignment{256};
	uint64_t ssbo_alignment{256};
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkDescriptorPool descriptor_pool;
	VkQueryPool timestamp_query_pool;

	/* extension debug */
	bool debug_utils_extension_present{false};
	VkDebugReportCallbackEXT debug_callback;
	VkDebugUtilsMessengerEXT debug_utils_messenger;

	/* window */
	VkSurfaceKHR window_surface;
	VkSurfaceFormatKHR window_surface_format;
	PresentMode present_mode{PresentMode::VSync};
	VkPresentModeKHR vk_present_mode;

	/* swapchain */
	RenderPassOutput swapchain_output;
	VkSwapchainKHR swapchain;
	std::array<VkImage, MaxSwapchainImages> swapchain_images;
	std::array<VkImageView, MaxSwapchainImages> swapchain_image_views;
	std::array<VkFramebuffer, MaxSwapchainImages> swapchain_framebuffers;
	uint32_t swapchain_width;
	uint32_t swapchain_height;
	uint32_t swapchain_image_count;

	VmaAllocator vma_allocator;

	/* sync */
	std::array<VkSemaphore, MaxSwapchainImages> render_complete_semaphore;
	std::array<VkSemaphore, MaxSwapchainImages> image_acquired_semaphore;
	std::array<VkFence, MaxSwapchainImages> command_buffer_fence;

	/* resource */
	std::unique_ptr<GPUResourceManager> gpu_resource_manager{nullptr};
	ComponentResource device_resource;

	std::array<CommandBuffer *, 128> queued_command_buffers;
	uint32_t num_allocated_command_buffers{0};
	uint32_t num_queued_command_buffers{0};

	uint32_t vulkan_image_index{0};
	uint32_t current_frame{0};
	uint32_t previous_frame{0};
	uint64_t absolute_frame{0};
	bool timestamps_enabled{false};

	std::vector<ResourceUpdate> resource_deletion_queue;
	std::vector<DescriptorSetUpdate> descriptor_set_updates;

	// resource
	BufferHandle fullscreen_vertex_buffer;
	SamplerHandle default_sampler;
	
	VulaknDeviceContext vulakn_device_context;

  public:
	void Init(GpuCreateParam &param);
	void Shutdown();

  private:
	void CreateInstance(GpuCreateParam &param);
	void CreateDebugExt();
	void CreatePhysicalDevice();
	void CreateDeviceAndQueue();
	void CreateSwapChain();
	void DestroySwapChain();
	void CreateVmaAllocator();
	void CreateDescriptorPool();
	void CreateQueryPool(const GpuCreateParam &param);
	void CreateSyncMarkers();
	void DestroySyncMarkers();
};

bool InitializeContextInstance(VulaknDeviceContext::InstanceData &context, const GpuCreateParam &param);
bool DestroyContextInstance(VulaknDeviceContext::InstanceData &context);

} // namespace cloud::vulkan