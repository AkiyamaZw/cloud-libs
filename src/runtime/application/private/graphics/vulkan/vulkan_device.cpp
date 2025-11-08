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

#define ArraySize(array) (sizeof(array) / sizeof(array)[0])
#define check_vk(succ)                                                                             \
	if ((succ) != VK_SUCCESS)                                                                      \
	{                                                                                              \
		printf("%d", succ);                                                                        \
		assert(false);                                                                             \
	}

#define check_true(succ) assert(succ)

namespace cloud::vulkan
{
typedef struct _GpuDevice
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

} GpuDevice;

GpuDevice g_vulkan_device;

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

void CreateDebugExt()
{
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	assert(g_vulkan_device.instance != VK_NULL_HANDLE);
	uint32_t num_instance_extensions;
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
	std::vector<VkExtensionProperties> extensions(num_instance_extensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, extensions.data());
	const auto &result = std::find_if(extensions.begin(), extensions.end(), [](const auto &rhs) {
		return !strcmp(rhs.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	});
	if (result != extensions.end())
	{
		g_vulkan_device.debug_utils_extension_present = true;
	}
	if (!g_vulkan_device.debug_utils_extension_present)
	{
		INFO("[Vulkan Device] Extension {} for debugging non presenting",
			 VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				g_vulkan_device.instance, "vkCreateDebugUtilsMessengerEXT");
		VkDebugUtilsMessengerCreateInfoEXT debug_msger_create_info =
			create_debug_utils_messenger_info();
		vkCreateDebugUtilsMessengerEXT(g_vulkan_device.instance,
									   &debug_msger_create_info,
									   nullptr,
									   &g_vulkan_device.debug_utils_messenger);
	}
	check_true(g_vulkan_device.debug_utils_messenger != VK_NULL_HANDLE);
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

void CreateInstance(GpuCreateParam &param)
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

	succ = vkCreateInstance(&ins_info, nullptr, &g_vulkan_device.instance);
	check_vk(succ);
	INFO("[Vulkan Gpu Device] Instance Created..");
}

void CreatePhysicalDevice()
{
	uint32_t num_physical_device = 0;
	VkResult succ =
		vkEnumeratePhysicalDevices(g_vulkan_device.instance, &num_physical_device, nullptr);
	check_vk(succ);

	std::vector<VkPhysicalDevice> gpus(num_physical_device);
	succ = vkEnumeratePhysicalDevices(g_vulkan_device.instance, &num_physical_device, gpus.data());
	check_vk(succ);
	VkPhysicalDeviceProperties device_property;
	VkPhysicalDevice discrate_device = VK_NULL_HANDLE;
	VkPhysicalDevice intergrate_device = VK_NULL_HANDLE;

	for (uint32_t index = 0; index < num_physical_device; ++index)
	{
		vkGetPhysicalDeviceProperties(gpus[index], &g_vulkan_device.physical_device_properties);
		const VkPhysicalDeviceType device_type =
			g_vulkan_device.physical_device_properties.deviceType;
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			if (get_family_queue(
					gpus[index], g_vulkan_device.window_surface, g_vulkan_device.queue_family))
			{
				discrate_device = gpus[index];
				break;
			}
			continue;
		}
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			if (get_family_queue(
					gpus[index], g_vulkan_device.window_surface, g_vulkan_device.queue_family))
			{
				intergrate_device = gpus[index];
				break;
			}
			continue;
		}
	}
	if (discrate_device != VK_NULL_HANDLE)
	{
		g_vulkan_device.physical_device = discrate_device;
	}
	else if (intergrate_device != VK_NULL_HANDLE)
	{
		g_vulkan_device.physical_device = intergrate_device;
	}
	check_true(g_vulkan_device.physical_device != VK_NULL_HANDLE);
	g_vulkan_device.gpu_timestamp_frequency =
		g_vulkan_device.physical_device_properties.limits.timestampPeriod / (1000 * 1000);
	g_vulkan_device.ubo_alignment =
		g_vulkan_device.physical_device_properties.limits.minUniformBufferOffsetAlignment;
	g_vulkan_device.ssbo_alignment =
		g_vulkan_device.physical_device_properties.limits.minStorageBufferOffsetAlignment;

	INFO("[vulkan device] select gpu {}, gpu_timestamp_frequency:{:.8f}",
		 g_vulkan_device.physical_device_properties.deviceName,
		 g_vulkan_device.gpu_timestamp_frequency);
}

void CreateDeviceAndQueue()
{
#ifdef WIN32
    std::vector<const char *> device_extensions = {"VK_KHR_swapchain"};
#else
	std::vector<const char *> device_extensions = {"VK_KHR_swapchain", "VK_KHR_portability_subset"};
#endif
	const float queue_priority[] = {1.f};
	VkDeviceQueueCreateInfo queue_info[1] = {};
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].queueFamilyIndex = g_vulkan_device.queue_family;
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = queue_priority;

	VkPhysicalDeviceFeatures2 physical_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	vkGetPhysicalDeviceFeatures2(g_vulkan_device.physical_device, &physical_features2);
	physical_features2.features.robustBufferAccess = VK_FALSE;

	VkDeviceCreateInfo device_cinfo = {};
	device_cinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_cinfo.queueCreateInfoCount = std::size(queue_info);
	device_cinfo.pQueueCreateInfos = queue_info;
	device_cinfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_cinfo.ppEnabledExtensionNames = device_extensions.data();
	device_cinfo.pNext = &physical_features2;

	VkResult succ = vkCreateDevice(
		g_vulkan_device.physical_device, &device_cinfo, nullptr, &g_vulkan_device.device);
	check_vk(succ);
	assert(g_vulkan_device.device != nullptr);

	vkGetDeviceQueue(
		g_vulkan_device.device, g_vulkan_device.queue_family, 0, &g_vulkan_device.queue);
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

void SetPresentMode(PresentMode in_present_mode,
					VkPresentModeKHR &out_vk_present_mode,
					uint32_t &out_swapchain_count,
					PresentMode &out_present_mode)
{
	uint32_t supported_cnt = 0;
	static VkPresentModeKHR present_mode[8];
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		g_vulkan_device.physical_device, g_vulkan_device.window_surface, &supported_cnt, nullptr);
	check_true(supported_cnt > 0);
	vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkan_device.physical_device,
											  g_vulkan_device.window_surface,
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

void CreateSwapChain()
{
	constexpr VkFormat surface_image_format[] = {VK_FORMAT_B8G8R8A8_UNORM,
												 VK_FORMAT_R8G8B8A8_UNORM,
												 VK_FORMAT_B8G8R8_UNORM,
												 VK_FORMAT_R8G8B8_UNORM};
	constexpr VkColorSpaceKHR surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	uint32_t supported_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		g_vulkan_device.physical_device, g_vulkan_device.window_surface, &supported_count, nullptr);
	std::vector<VkSurfaceFormatKHR> supported_format(supported_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan_device.physical_device,
										 g_vulkan_device.window_surface,
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
				g_vulkan_device.window_surface_format = supported_format[j];
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
	g_vulkan_device.swapchain_output.Reset();
	g_vulkan_device.swapchain_output.SetColorFormat(g_vulkan_device.window_surface_format.format);

	SetPresentMode(g_vulkan_device.present_mode,
				   g_vulkan_device.vk_present_mode,
				   g_vulkan_device.swapchain_image_count,
				   g_vulkan_device.present_mode);

	VkBool32 surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(g_vulkan_device.physical_device,
										 g_vulkan_device.queue_family,
										 g_vulkan_device.window_surface,
										 &surface_supported);
	if (surface_supported != VK_TRUE)
	{
		FATAL("Cannot find surface support device");
	}

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		g_vulkan_device.physical_device, g_vulkan_device.window_surface, &surface_capabilities);

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
		 g_vulkan_device.swapchain_width,
		 g_vulkan_device.swapchain_height,
		 surface_capabilities.minImageCount);
	g_vulkan_device.swapchain_width = swapchain_extent.width;
	g_vulkan_device.swapchain_height = swapchain_extent.height;

	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.pNext = nullptr;
	swapchain_create_info.surface = g_vulkan_device.window_surface;
	swapchain_create_info.minImageCount = g_vulkan_device.swapchain_image_count;
	swapchain_create_info.imageFormat = g_vulkan_device.window_surface_format.format;
	swapchain_create_info.imageExtent = swapchain_extent;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = g_vulkan_device.vk_present_mode;

	VkResult succ = vkCreateSwapchainKHR(
		g_vulkan_device.device, &swapchain_create_info, nullptr, &g_vulkan_device.swapchain);
	check_vk(succ);

	succ = vkGetSwapchainImagesKHR(g_vulkan_device.device,
								   g_vulkan_device.swapchain,
								   &g_vulkan_device.swapchain_image_count,
								   nullptr);
	check_vk(succ);

	vkGetSwapchainImagesKHR(g_vulkan_device.device,
							g_vulkan_device.swapchain,
							&g_vulkan_device.swapchain_image_count,
							g_vulkan_device.swapchain_images.data());

	for (size_t i = 0; i < g_vulkan_device.swapchain_image_count; i++)
	{
		VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = g_vulkan_device.window_surface_format.format;
		info.image = g_vulkan_device.swapchain_images[i];
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		succ = vkCreateImageView(
			g_vulkan_device.device, &info, nullptr, &g_vulkan_device.swapchain_image_views[i]);
	}
}

void DestroySwapchain()
{
	for (size_t i = 0; i < g_vulkan_device.swapchain_image_count; i++)
	{
		vkDestroyImageView(
			g_vulkan_device.device, g_vulkan_device.swapchain_image_views[i], nullptr);
		// vkDestroyFramebuffer(g_vulkan_device.device, g_vulkan_device.swapchain_freamebuffers[i],
		// nullptr);
	}
	vkDestroySwapchainKHR(g_vulkan_device.device, g_vulkan_device.swapchain, nullptr);
}

void CreateVmaAllocator()
{
	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.physicalDevice = g_vulkan_device.physical_device;
	allocator_create_info.device = g_vulkan_device.device;
	allocator_create_info.instance = g_vulkan_device.instance;

	VkResult succ = vmaCreateAllocator(&allocator_create_info, &g_vulkan_device.vma_allocator);
	check_vk(succ);
}

void CreateDescriptorPool()
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
		g_vulkan_device.device, &pool_create_info, nullptr, &g_vulkan_device.descriptor_pool);
	check_vk(succ);
}

void CreateQueryPool(const GpuCreateParam &param)
{
    VkQueryPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                              nullptr,
                                              0,
                                              VK_QUERY_TYPE_TIMESTAMP,
                                              param.gpu_time_queries_per_frame * 2u *
                                                  MaxSwapchainImages,
                                              0};
    vkCreateQueryPool(
        g_vulkan_device.device, &pool_create_info, nullptr, &g_vulkan_device.timestamp_query_pool);
}


void CreateSyncMarkers()
{
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (size_t i=0; i< MaxSwapchainImages; ++i)
    {
        vkCreateSemaphore(g_vulkan_device.device, &semaphore_create_info, nullptr, &g_vulkan_device.render_complete_semaphore[i]);
        vkCreateSemaphore(g_vulkan_device.device, &semaphore_create_info, nullptr, &g_vulkan_device.image_acquired_semaphore[i]);
        VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(g_vulkan_device.device, &fence_create_info, nullptr, &g_vulkan_device.command_buffer_fence[i]);
    }
}

void DestroySyncMarkers()
{
    for (size_t i = 0; i < g_vulkan_device.swapchain_image_count; ++i)
    {
        vkDestroySemaphore(g_vulkan_device.device, g_vulkan_device.render_complete_semaphore[i], nullptr);
        vkDestroySemaphore(g_vulkan_device.device, g_vulkan_device.image_acquired_semaphore[i], nullptr);
        vkDestroyFence(g_vulkan_device.device, g_vulkan_device.command_buffer_fence[i], nullptr);
    }
}

void InitGpuDevice(GpuCreateParam &param)
{
	INFO("[Vulkan Gpu Device] Start init...");
	VkResult succ;

	// instance
	CreateInstance(param);
	assert(g_vulkan_device.instance);

	// messenger
	CreateDebugExt();

	// surface creation
	g_vulkan_device.swapchain_width = param.width;
	g_vulkan_device.swapchain_height = param.height;
	succ = glfwCreateWindowSurface(g_vulkan_device.instance,
								   static_cast<GLFWwindow *>(param.window),
								   nullptr,
								   &g_vulkan_device.window_surface);
	check_vk(succ);

	CreatePhysicalDevice();
	assert(g_vulkan_device.physical_device);

	CreateDeviceAndQueue();
	assert(g_vulkan_device.device);
	assert(g_vulkan_device.queue);

	CreateSwapChain();
	assert(g_vulkan_device.swapchain);

	CreateVmaAllocator();
	assert(g_vulkan_device.vma_allocator);

	CreateDescriptorPool();
	assert(g_vulkan_device.descriptor_pool);

    CreateQueryPool(param);
	assert(g_vulkan_device.timestamp_query_pool);

    CreateSyncMarkers();
    assert(g_vulkan_device.render_complete_semaphore[0]);
    assert(g_vulkan_device.image_acquired_semaphore[0]);
    assert(g_vulkan_device.command_buffer_fence[0]);
}

void ShutdownGpuDevice()
{

    DestroySyncMarkers();
	vkDestroyQueryPool(g_vulkan_device.device, g_vulkan_device.timestamp_query_pool, nullptr);
	vkDestroyDescriptorPool(g_vulkan_device.device, g_vulkan_device.descriptor_pool, nullptr);
	vmaDestroyAllocator(g_vulkan_device.vma_allocator);
	DestroySwapchain();
	vkDestroyDevice(g_vulkan_device.device, nullptr);
	vkDestroySurfaceKHR(g_vulkan_device.instance, g_vulkan_device.window_surface, nullptr);

#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	auto vkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			g_vulkan_device.instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(
		g_vulkan_device.instance, g_vulkan_device.debug_utils_messenger, nullptr);
#endif
	vkDestroyInstance(g_vulkan_device.instance, nullptr);
}
} // namespace cloud::vulkan