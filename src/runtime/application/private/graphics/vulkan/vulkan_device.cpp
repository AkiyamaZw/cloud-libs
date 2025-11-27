#include "graphics/vulkan/vulkan_device.h"
#include "runtime_log.h"
#include "data_structure/resource_pool.h"
#include <array>
#include <map>
#include <ranges>
#include <algorithm>
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/vk_mem_alloc.h"
#include "graphics/vulkan/command_buffer.h"
#include "graphics/vulkan/vulkan_interface.h"

#include <corecrt_io.h>

#define ArraySize(array) (sizeof(array) / sizeof(array)[0])


namespace cloud::vulkan
{
struct _GpuDevice
{
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


    std::array<CommandBuffer*, 128> queued_command_buffers;
    uint32_t num_allocated_command_buffers{0};
    uint32_t num_queued_command_buffers{0};

    uint32_t vulkan_image_index{0};
    uint32_t current_frame{0};
    uint32_t previous_frame{0};
    uint64_t absolute_frame{0};
    bool timestamps_enabled{false};

    std::vector<ResourceUpdate> resource_deletion_queue{16};
    std::vector<DescriptorSetUpdate> descriptor_set_updates{16};


    // resource
    BufferHandle fullscreen_vertex_buffer;
    SamplerHandle default_sampler;

    void SetResourceName(VkObjectType type, uint64_t handle, const char* name);
};


struct CommandBufferRing
{
    static const uint16_t GMaxThreads = 1;
    static const uint16_t GMaxPools = MaxSwapchainImages*GMaxThreads;
    static const uint16_t GBuffersPerPool = 4;
    static const uint16_t GMaxBuffers = GBuffersPerPool * GMaxPools;

    VkCommandPool vk_command_pool[GMaxPools];
    CommandBuffer command_buffer[GMaxBuffers];
    uint8_t next_free_per_thread_frame[GMaxPools];
    void Init(_GpuDevice* gpu);
    void Destroy(_GpuDevice* gpu);
    void Reset(_GpuDevice* gpu, uint32_t frame_index);
    static uint32_t IndexInPool(uint32_t index){return (uint16_t)index / GBuffersPerPool;}
    CommandBuffer* GetCommandBuffer(uint32_t frame_index, bool begin);
    CommandBuffer* GetCommandBufferInstant(uint32_t frame_index, bool begin);
} g_vulkan_cmd_buffer_ring;

void _GpuDevice::SetResourceName(VkObjectType type, uint64_t handle, const char *name)
{
    if (debug_utils_extension_present)
    {
        infra::SetResourceName(device, type, handle, name);
    }
}

void CommandBufferRing::Init(_GpuDevice* gpu)
{
    for (uint32_t i = 0; i < GMaxPools; i++)
    {
        VkCommandPoolCreateInfo cmd_pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr,};
        cmd_pool_info.queueFamilyIndex = gpu->queue_family;
        cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        check_vk(vkCreateCommandPool(gpu->device, &cmd_pool_info, nullptr, &vk_command_pool[i]));
    }

    for (uint32_t i=0; i< GMaxBuffers; i++)
    {
        VkCommandBufferAllocateInfo cmd_alloc_cmd = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr};
        const uint32_t pool_index = IndexInPool(i);
        cmd_alloc_cmd.commandPool = vk_command_pool[pool_index];
        cmd_alloc_cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_alloc_cmd.commandBufferCount = 1;
        check_vk(vkAllocateCommandBuffers(gpu->device, &cmd_alloc_cmd, &command_buffer[i].vk_command_buffer));
        command_buffer[i].handle = i;
        command_buffer[i].Reset();
    }
}

void CommandBufferRing::Destroy(_GpuDevice* gpu)
{
    for (uint32_t i=0; i<GMaxPools;++i)
    {
        vkDestroyCommandPool(gpu->device, vk_command_pool[i], nullptr);
    }
}

void CommandBufferRing::Reset(_GpuDevice* gpu, uint32_t frame_index)
{
    for (uint32_t i=0; i<GMaxThreads;++i)
    {
        vkResetCommandPool(gpu->device, vk_command_pool[frame_index*GMaxThreads + i], 0);
    }
}

CommandBuffer * CommandBufferRing::GetCommandBuffer(uint32_t frame_index, bool begin)
{
    CommandBuffer* cmd_buffer = &command_buffer[frame_index * GBuffersPerPool];
    if (begin)
    {
        cmd_buffer->Reset();
        VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd_buffer->vk_command_buffer, &begin_info);
    }
    return cmd_buffer;
}

CommandBuffer * CommandBufferRing::GetCommandBufferInstant(uint32_t frame_index, bool begin)
{
    CommandBuffer* cmd_buffer = &command_buffer[frame_index * GBuffersPerPool+1];
    return cmd_buffer;
}


static const char *s_instance_layer[] = {
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	"VK_LAYER_KHRONOS_validation",
#else
	"",
#endif
};

static const char *s_requested_extensions[] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
// Platform specific extension
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_MIR_KHR) || defined(VK_USE_PLATFORM_DISPLAY_KHR)
	VK_KHR_DISPLAY_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_WIN32_KHR

#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // VULKAN_DEBUG_REPORT
};

static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
									 VkDebugUtilsMessageTypeFlagsEXT types,
									 const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
									 void *user_data)
{
	INFO(" MessageID: {} {}\nMessage: {}\n\n",
		 callback_data->pMessageIdName,
		 callback_data->messageIdNumber,
		 callback_data->pMessage);

	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		// __debugbreak();
	}

	return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT create_debug_utils_messenger_info()
{
	VkDebugUtilsMessengerCreateInfoEXT creation_info = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
	creation_info.pfnUserCallback = debug_utils_callback;
	creation_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	creation_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

	return creation_info;
}

void CreateDebugExt(_GpuDevice* gpu)
{
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	assert(gpu->instance != VK_NULL_HANDLE);
	uint32_t num_instance_extensions;
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
	std::vector<VkExtensionProperties> extensions(num_instance_extensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, extensions.data());
	const auto &result = std::find_if(extensions.begin(), extensions.end(), [](const auto &rhs) {
		return !strcmp(rhs.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	});
	if (result != extensions.end())
	{
		gpu->debug_utils_extension_present = true;
	}
	if (!gpu->debug_utils_extension_present)
	{
		INFO("[Vulkan Device] Extension {} for debugging non presenting",
			 VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				gpu->instance, "vkCreateDebugUtilsMessengerEXT");
		VkDebugUtilsMessengerCreateInfoEXT debug_msger_create_info =
			create_debug_utils_messenger_info();
		vkCreateDebugUtilsMessengerEXT(gpu->instance,
									   &debug_msger_create_info,
									   nullptr,
									   &gpu->debug_utils_messenger);
	}
	check_true(gpu->debug_utils_messenger != VK_NULL_HANDLE);
	INFO("[Vulkan GPU Device] DebugUtilsMessenger Created..");
#endif
}

bool get_family_queue(VkPhysicalDevice physical_device,
					  VkSurfaceKHR window_surface,
					  uint32_t &queue_family_index)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		physical_device, &queue_family_count, queue_families.data());

	VkBool32 surface_supported;
	for (uint32_t family_index = 0; family_index < queue_family_count; ++family_index)
	{
		VkQueueFamilyProperties queue_family = queue_families[family_index];
		if (queue_family.queueCount > 0 &&
			queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(
				physical_device, family_index, window_surface, &surface_supported);

			if (surface_supported)
			{
				queue_family_index = family_index;
				break;
			}
		}
	}
	return surface_supported;
}

VkResult UseLatestApiVersion(uint32_t &api_version)
{
	if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))
		return vkEnumerateInstanceVersion(&api_version);
	return VK_SUCCESS;
}

void CreateInstance(_GpuDevice* gpu, GpuCreateParam &param)
{
	std::vector<const char *> window_extension;
	uint32_t extension_count = 0;
#ifdef WIN32
	extension_count = 2;
	window_extension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	window_extension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __APPLE__

	const char **extension_names = nullptr;
	extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
	if (extension_count == 0)
	{
		FATAL("[GLFW] cannot get Vulkan Extentions Info!");
		return;
	}
	for (int i = 0; i < extension_count; i++)
	{
		window_extension.push_back(extension_names[i]);
	}
#endif

	std::vector<const char *> extensions;
	for (int i = 0; i < std::size(s_requested_extensions); i++)
	{
		extensions.push_back(s_requested_extensions[i]);
	}
	VkResult succ;
	extensions.insert(extensions.end(), window_extension.begin(), window_extension.end());
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	uint32_t api_version = 0;
	succ = UseLatestApiVersion(api_version);
	check_vk(succ);
	INFO("api version: {}.{}.{}",
		 VK_VERSION_MAJOR(api_version),
		 VK_VERSION_MINOR(api_version),
		 VK_VERSION_PATCH(api_version));

	VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
								  .apiVersion = api_version};
	VkInstanceCreateInfo ins_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
									 .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
									 .pApplicationInfo = &app_info,
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
									 .enabledLayerCount = std::size(s_instance_layer),
									 .ppEnabledLayerNames = s_instance_layer,
									 .enabledExtensionCount =
										 static_cast<uint32_t>(extensions.size()),
									 .ppEnabledExtensionNames = extensions.data()
#endif
	};

	const VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
		create_debug_utils_messenger_info();
	ins_info.pNext = &debug_create_info;

	succ = vkCreateInstance(&ins_info, nullptr, &gpu->instance);
	check_vk(succ);
	INFO("[Vulkan Gpu Device] Instance Created..");
}

void CreatePhysicalDevice(_GpuDevice* gpu)
{
	uint32_t num_physical_device = 0;
	VkResult succ =
		vkEnumeratePhysicalDevices(gpu->instance, &num_physical_device, nullptr);
	check_vk(succ);

	std::vector<VkPhysicalDevice> gpus(num_physical_device);
	succ = vkEnumeratePhysicalDevices(gpu->instance, &num_physical_device, gpus.data());
	check_vk(succ);
	VkPhysicalDeviceProperties device_property;
	VkPhysicalDevice discrate_device = VK_NULL_HANDLE;
	VkPhysicalDevice intergrate_device = VK_NULL_HANDLE;

	for (uint32_t index = 0; index < num_physical_device; ++index)
	{
		vkGetPhysicalDeviceProperties(gpus[index], &gpu->physical_device_properties);
		const VkPhysicalDeviceType device_type =
			gpu->physical_device_properties.deviceType;
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			if (get_family_queue(
					gpus[index], gpu->window_surface, gpu->queue_family))
			{
				discrate_device = gpus[index];
				break;
			}
			continue;
		}
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			if (get_family_queue(
					gpus[index], gpu->window_surface, gpu->queue_family))
			{
				intergrate_device = gpus[index];
				break;
			}
			continue;
		}
	}
	if (discrate_device != VK_NULL_HANDLE)
	{
		gpu->physical_device = discrate_device;
	}
	else if (intergrate_device != VK_NULL_HANDLE)
	{
		gpu->physical_device = intergrate_device;
	}
	check_true(gpu->physical_device != VK_NULL_HANDLE);
	gpu->gpu_timestamp_frequency =
		gpu->physical_device_properties.limits.timestampPeriod / (1000 * 1000);
	gpu->ubo_alignment =
		gpu->physical_device_properties.limits.minUniformBufferOffsetAlignment;
	gpu->ssbo_alignment =
		gpu->physical_device_properties.limits.minStorageBufferOffsetAlignment;

	INFO("[vulkan device] select gpu {}, gpu_timestamp_frequency:{:.8f}",
		 gpu->physical_device_properties.deviceName,
		 gpu->gpu_timestamp_frequency);
}

void CreateDeviceAndQueue(_GpuDevice* gpu)
{
#ifdef WIN32
    std::vector<const char *> device_extensions = {"VK_KHR_swapchain"};
#else
	std::vector<const char *> device_extensions = {"VK_KHR_swapchain", "VK_KHR_portability_subset"};
#endif
	const float queue_priority[] = {1.f};
	VkDeviceQueueCreateInfo queue_info[1] = {};
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].queueFamilyIndex = gpu->queue_family;
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = queue_priority;

	VkPhysicalDeviceFeatures2 physical_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	vkGetPhysicalDeviceFeatures2(gpu->physical_device, &physical_features2);
	physical_features2.features.robustBufferAccess = VK_FALSE;

	VkDeviceCreateInfo device_cinfo = {};
	device_cinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_cinfo.queueCreateInfoCount = std::size(queue_info);
	device_cinfo.pQueueCreateInfos = queue_info;
	device_cinfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_cinfo.ppEnabledExtensionNames = device_extensions.data();
	device_cinfo.pNext = &physical_features2;

	VkResult succ = vkCreateDevice(
		gpu->physical_device, &device_cinfo, nullptr, &gpu->device);
	check_vk(succ);
	assert(gpu->device != nullptr);

	vkGetDeviceQueue(
		gpu->device, gpu->queue_family, 0, &gpu->queue);
}

VkPresentModeKHR ConvertToVkPresentMode(PresentMode mode)
{
	switch (mode)
	{
	case PresentMode::VSyncFast:
		return VK_PRESENT_MODE_MAILBOX_KHR;
	case PresentMode::VSyncRelaxed:
		return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	case PresentMode::Immediate:
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	case PresentMode::VSync:
	default:
		return VK_PRESENT_MODE_MAILBOX_KHR;
	}
}

void SetPresentMode(_GpuDevice* gpu,
                    PresentMode in_present_mode,
					VkPresentModeKHR &out_vk_present_mode,
					uint32_t &out_swapchain_count,
					PresentMode &out_present_mode)
{
	uint32_t supported_cnt = 0;
	static VkPresentModeKHR present_mode[8];
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		gpu->physical_device, gpu->window_surface, &supported_cnt, nullptr);
	check_true(supported_cnt > 0);
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->physical_device,
											  gpu->window_surface,
											  &supported_cnt,
											  present_mode);
	bool mode_found = false;
	VkPresentModeKHR request_mode = ConvertToVkPresentMode(in_present_mode);
	for (uint32_t i = 0; i < supported_cnt; i++)
	{
		if (request_mode == present_mode[i])
		{
			mode_found = true;
			break;
		}
	}
	out_vk_present_mode = mode_found ? request_mode : VK_PRESENT_MODE_FIFO_KHR;
	out_swapchain_count = 3;
	out_present_mode = mode_found ? in_present_mode : PresentMode::VSync;
}

void CreateSwapChain(_GpuDevice* gpu)
{
	constexpr VkFormat surface_image_format[] = {VK_FORMAT_B8G8R8A8_UNORM,
												 VK_FORMAT_R8G8B8A8_UNORM,
												 VK_FORMAT_B8G8R8_UNORM,
												 VK_FORMAT_R8G8B8_UNORM};
	constexpr VkColorSpaceKHR surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	uint32_t supported_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		gpu->physical_device, gpu->window_surface, &supported_count, nullptr);
	std::vector<VkSurfaceFormatKHR> supported_format(supported_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->physical_device,
										 gpu->window_surface,
										 &supported_count,
										 supported_format.data());

	bool format_found = false;
	for (int i = 0; i < ArraySize(surface_image_format); ++i)
	{
		for (int j = 0; j < supported_count; ++j)
		{
			if (supported_format[j].format == surface_image_format[i] &&
				supported_format[j].colorSpace == surface_color_space)
			{
				gpu->window_surface_format = supported_format[j];
				format_found = true;
				break;
			}
		}
		if (format_found)
		{
			break;
		}
	}
	check_true(format_found);
	gpu->swapchain_output.Reset();
	gpu->swapchain_output.SetColorFormat(gpu->window_surface_format.format);

	SetPresentMode(gpu,
	gpu->present_mode,
				   gpu->vk_present_mode,
				   gpu->swapchain_image_count,
				   gpu->present_mode);

	VkBool32 surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpu->physical_device,
										 gpu->queue_family,
										 gpu->window_surface,
										 &surface_supported);
	if (surface_supported != VK_TRUE)
	{
		FATAL("Cannot find surface support device");
	}

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		gpu->physical_device, gpu->window_surface, &surface_capabilities);

	VkExtent2D swapchain_extent = surface_capabilities.currentExtent;
	if (swapchain_extent.width == UINT32_MAX)
	{
		swapchain_extent.width = std::clamp(swapchain_extent.width,
											surface_capabilities.minImageExtent.width,
											surface_capabilities.maxImageExtent.width);
		swapchain_extent.height = std::clamp(swapchain_extent.height,
											 surface_capabilities.minImageExtent.height,
											 surface_capabilities.maxImageExtent.height);
	}
	INFO("Create swapchain {}, {} - Saved {} {}, min image {}\n",
		 swapchain_extent.width,
		 swapchain_extent.height,
         gpu->swapchain_width,
         gpu->swapchain_height,
		 surface_capabilities.minImageCount);
    gpu->swapchain_width = swapchain_extent.width;
	gpu->swapchain_height = swapchain_extent.height;

	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.pNext = nullptr;
	swapchain_create_info.surface = gpu->window_surface;
	swapchain_create_info.minImageCount = gpu->swapchain_image_count;
	swapchain_create_info.imageFormat = gpu->window_surface_format.format;
	swapchain_create_info.imageExtent = swapchain_extent;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = gpu->vk_present_mode;

	VkResult succ = vkCreateSwapchainKHR(
		gpu->device, &swapchain_create_info, nullptr, &gpu->swapchain);
	check_vk(succ);

	succ = vkGetSwapchainImagesKHR(gpu->device,
								   gpu->swapchain,
								   &gpu->swapchain_image_count,
								   nullptr);
	check_vk(succ);

	vkGetSwapchainImagesKHR(gpu->device,
							gpu->swapchain,
							&gpu->swapchain_image_count,
							gpu->swapchain_images.data());

	for (size_t i = 0; i < gpu->swapchain_image_count; i++)
	{
		VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = gpu->window_surface_format.format;
		info.image = gpu->swapchain_images[i];
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		succ = vkCreateImageView(
			gpu->device, &info, nullptr, &gpu->swapchain_image_views[i]);
	}
}

void DestroySwapchain(_GpuDevice* gpu)
{
	for (size_t i = 0; i < gpu->swapchain_image_count; i++)
	{
		vkDestroyImageView(
			gpu->device, gpu->swapchain_image_views[i], nullptr);
		// vkDestroyFramebuffer(g_vulkan_device.device, g_vulkan_device.swapchain_freamebuffers[i],
		// nullptr);
	}
	vkDestroySwapchainKHR(gpu->device, gpu->swapchain, nullptr);
}

void CreateVmaAllocator(_GpuDevice* gpu)
{
	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.physicalDevice = gpu->physical_device;
	allocator_create_info.device = gpu->device;
	allocator_create_info.instance = gpu->instance;

	VkResult succ = vmaCreateAllocator(&allocator_create_info, &gpu->vma_allocator);
	check_vk(succ);
}

void CreateDescriptorPool(_GpuDevice* gpu)
{
	VkDescriptorPoolSize descriptor_pool_size[] = {
		{VK_DESCRIPTOR_TYPE_SAMPLER, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, GlobalPoolElements},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, GlobalPoolElements}};
	VkDescriptorPoolCreateInfo pool_create_info = {};
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_create_info.maxSets = GlobalPoolElements * ArraySize(descriptor_pool_size);
	pool_create_info.poolSizeCount = (uint32_t)ArraySize(descriptor_pool_size);
	pool_create_info.pPoolSizes = descriptor_pool_size;
	VkResult succ = vkCreateDescriptorPool(
		gpu->device, &pool_create_info, nullptr, &gpu->descriptor_pool);
	check_vk(succ);
}

void CreateQueryPool(_GpuDevice* gpu, const GpuCreateParam &param)
{
    VkQueryPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                              nullptr,
                                              0,
                                              VK_QUERY_TYPE_TIMESTAMP,
                                              param.gpu_time_queries_per_frame * 2u *
                                                  MaxSwapchainImages,
                                              0};
    vkCreateQueryPool(
        gpu->device, &pool_create_info, nullptr, &gpu->timestamp_query_pool);
}


void CreateSyncMarkers(_GpuDevice* gpu)
{
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (size_t i=0; i< MaxSwapchainImages; ++i)
    {
        vkCreateSemaphore(gpu->device, &semaphore_create_info, nullptr, &gpu->render_complete_semaphore[i]);
        vkCreateSemaphore(gpu->device, &semaphore_create_info, nullptr, &gpu->image_acquired_semaphore[i]);
        VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(gpu->device, &fence_create_info, nullptr, &gpu->command_buffer_fence[i]);
    }
}

void DestroySyncMarkers(_GpuDevice* gpu)
{
    for (size_t i = 0; i < gpu->swapchain_image_count; ++i)
    {
        vkDestroySemaphore(gpu->device, gpu->render_complete_semaphore[i], nullptr);
        vkDestroySemaphore(gpu->device, gpu->image_acquired_semaphore[i], nullptr);
        vkDestroyFence(gpu->device, gpu->command_buffer_fence[i], nullptr);
    }
}


GpuDevice * GpuDevice::Inst()
{
    static GpuDevice instance;
    return &instance;
}

GpuDevice::~GpuDevice()
{
    impl_ = nullptr;
}

void GpuDevice::InitGpuDevice(GpuCreateParam &param)
{
    impl_ = new _GpuDevice();

	INFO("[Vulkan Gpu Device] Start init...");
	VkResult succ;

	// instance
	CreateInstance(impl_, param);
	assert(impl_->instance);

	// messenger
	CreateDebugExt(impl_);

	// surface creation
	impl_->swapchain_width = param.width;
	impl_->swapchain_height = param.height;
	succ = glfwCreateWindowSurface(impl_->instance,
								   static_cast<GLFWwindow *>(param.window),
								   nullptr,
								   &impl_->window_surface);
	check_vk(succ);

	CreatePhysicalDevice(impl_);
	assert(impl_->physical_device);

	CreateDeviceAndQueue(impl_);
	assert(impl_->device);
	assert(impl_->queue);

    infra::InitVulkanInterface(impl_->device);

	CreateSwapChain(impl_);
	assert(impl_->swapchain);

	CreateVmaAllocator(impl_);
	assert(impl_->vma_allocator);

	CreateDescriptorPool(impl_);
	assert(impl_->descriptor_pool);

    CreateQueryPool(impl_, param);
	assert(impl_->timestamp_query_pool);

    CreateSyncMarkers(impl_);
    assert(impl_->render_complete_semaphore[0]);
    assert(impl_->image_acquired_semaphore[0]);
    assert(impl_->command_buffer_fence[0]);

    g_vulkan_cmd_buffer_ring.Init(impl_);


    SamplerCreation sc{};
    sc.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sc.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sc.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sc.min_filter = VK_FILTER_LINEAR;
    sc.mag_filter = VK_FILTER_LINEAR;
    sc.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sc.name = "Sampler Default";
    impl_->default_sampler = CreateSampler(sc);
}

void GpuDevice::ShutdownGpuDevice()
{

    g_vulkan_cmd_buffer_ring.Destroy(impl_);
    DestroySampler(impl_->default_sampler);

    DestroySyncMarkers(impl_);
	vkDestroyQueryPool(impl_->device, impl_->timestamp_query_pool, nullptr);
	vkDestroyDescriptorPool(impl_->device, impl_->descriptor_pool, nullptr);
	vmaDestroyAllocator(impl_->vma_allocator);
	DestroySwapchain(impl_);
	vkDestroyDevice(impl_->device, nullptr);
	vkDestroySurfaceKHR(impl_->instance, impl_->window_surface, nullptr);

#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	auto vkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			impl_->instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(
		impl_->instance, impl_->debug_utils_messenger, nullptr);
#endif
	vkDestroyInstance(impl_->instance, nullptr);
}

SamplerHandle GpuDevice::CreateSampler(const render::SamplerCreation &creation)
{
    SamplerCreation sampler_creation{};
    sampler_creation.name = creation.name.data();
    ToVKEnum(creation.min_filter, sampler_creation.min_filter);
    ToVKEnum(creation.mag_filter, sampler_creation.mag_filter);
    ToVKEnum(creation.mip_filter, sampler_creation.mip_filter);
    ToVKEnum(creation.address_mode_u, sampler_creation.address_mode_u);
    ToVKEnum(creation.address_mode_v, sampler_creation.address_mode_v);
    ToVKEnum(creation.address_mode_w, sampler_creation.address_mode_w);
    ToVKEnum(creation.reduction_mode, sampler_creation.reduction_mode);
    return CreateSampler(sampler_creation);
}

void GpuDevice::DestroySampler(const SamplerHandle &handle)
{
    if (handle.index < impl_->samplers.GetCapacity())
    {
        impl_->resource_deletion_queue.push_back({ResourceUpdateType::Sampler, handle.index, impl_->current_frame});
    }
    else
    {
        WARN("release sampler handle with error handle index {}", handle.index);
    }
}
SamplerHandle GpuDevice::CreateSampler(const SamplerCreation &creation)
{
    SamplerHandle handle = {impl_->samplers.FetchResource()};
    if (handle.index == ResourcePool::INVALID_NUM)
    {
        return handle;
    }
    Sampler* sampler = AccessSampler(handle);
    infra::CreateSampler(impl_->device, creation, sampler->sampler);
    impl_->SetResourceName(VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(sampler->sampler), creation.name);
    return handle;
}

Sampler *GpuDevice::AccessSampler(const SamplerHandle &handle)
{
    return static_cast<Sampler *>(impl_->samplers.Access(handle.index));
}

const Sampler *GpuDevice::AccessSampler(const SamplerHandle &handle) const
{
    return static_cast<const Sampler *>(impl_->samplers.Access(handle.index));
}

} // namespace cloud::vulkan