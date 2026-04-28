#include "graphics/vulkan/vulkan_interface.h"
#include "core/runtime_log.h"
#include "spdlog/fmt/bundled/chrono.h"

#define _ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define _GET_N_ARGS(...) _ARG_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define _COPY_1(d, s, p1) d.p1 = s.p1;
#define _COPY_2(d, s, p1, p2) _COPY_1(d, s, p1) _COPY_1(d, s, p2)
#define _COPY_3(d, s, p1, p2, p3) _COPY_2(d, s, p1, p2) _COPY_1(d, s, p3)
#define _COPY_4(d, s, p1, p2, p3, p4) _COPY_3(d, s, p1, p2, p3) _COPY_1(d, s, p4)
#define _COPY_5(d, s, p1, p2, p3, p4, p5) _COPY_4(d, s, p1, p2, p3, p4) _COPY_1(d, s, p5)
#define _COPY_6(d, s, p1, p2, p3, p4, p5, p6) _COPY_5(d, s, p1, p2, p3, p4, p5) _COPY_1(d, s, p6)
#define _COPY_7(d, s, p1, p2, p3, p4, p5, p6, p7)                                                  \
	_COPY_6(d, s, p1, p2, p3, p4, p5, p6) _COPY_1(d, s, p7)
#define _COPY_8(d, s, p1, p2, p3, p4, p5, p6, p7, p8)                                              \
	_COPY_7(d, s, p1, p2, p3, p4, p5, p6, p7) _COPY_1(d, s, p8)
#define _COPY_9(d, s, p1, p2, p3, p4, p5, p6, p7, p8, p9)                                          \
	_COPY_8(d, s, p1, p2, p3, p4, p5, p6, p7, p8) _COPY_1(d, s, p9)
#define _COPY_10(d, s, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)                                    \
	_COPY_9(d, s, p1, p2, p3, p4, p5, p6, p7, p8, p9) _COPY_1(d, s, p10)

#define _COPY_DISPATCH(n) _COPY_##n
#define _COPY_MEMBERS(dest, src, n, ...) _COPY_DISPATCH(n)(dest, src, __VA_ARGS__)
#define COPY_MEMBERS(dest, src, ...)                                                               \
	do                                                                                             \
	{                                                                                              \
		_COPY_MEMBERS(dest, src, _GET_N_ARGS(__VA_ARGS__), __VA_ARGS__);                           \
	} while (0)

namespace cloud::vulkan::infra
{
PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT;

void InitVulkanInterface(VkDevice device, bool debug_message)
{
	if (debug_message)
	{
		pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(
			device, "vkSetDebugUtilsObjectNameEXT");
		pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(
			device, "vkCmdBeginDebugUtilsLabelEXT");
		pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(
			device, "vkCmdEndDebugUtilsLabelEXT");
	}
}

void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char *name)
{
	VkDebugUtilsObjectNameInfoEXT name_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
	name_info.objectType = type;
	name_info.objectHandle = handle;
	name_info.pObjectName = name;
	pfnSetDebugUtilsObjectNameEXT(device, &name_info);
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

VkResult UseLatestApiVersion(uint32_t &api_version)
{
	if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))
		return vkEnumerateInstanceVersion(&api_version);
	return VK_SUCCESS;
}

static VkBool32 debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
									 VkDebugUtilsMessageTypeFlagsEXT types,
									 const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
									 void *user_data)
{
	INFO(" MessageID: %s %d\nMessage: %s\n\n",
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

bool CreateVkInstance(InstanceData &instance_data, const GpuCreateParam &param)
{
#ifdef WIN32
	instance_data.enabled_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instance_data.enabled_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __APPLE__
	uint32_t extension_count = 0;
	const char **extension_names = nullptr;
	extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
	if (extension_count == 0)
	{
		FATAL("[GLFW] cannot get Vulkan Extentions Info!");
		return false;
	}
	for (int i = 0; i < extension_count; i++)
	{
		instance_data.enabled_extensions.push_back(extension_names[i]);
	}
#endif
	VkResult succ;
	instance_data.enabled_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	uint32_t api_version = 0;
	succ = UseLatestApiVersion(api_version);
	check_vk(succ);
	INFO("api version: %d.%d.%d",
		 VK_VERSION_MAJOR(api_version),
		 VK_VERSION_MINOR(api_version),
		 VK_VERSION_PATCH(api_version));

	VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
								  .apiVersion = api_version};
	VkInstanceCreateInfo ins_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
		.pApplicationInfo = &app_info,
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
		.enabledLayerCount = std::size(s_instance_layer),
		.ppEnabledLayerNames = s_instance_layer,
		.enabledExtensionCount = static_cast<uint32_t>(instance_data.enabled_extensions.size()),
		.ppEnabledExtensionNames = instance_data.enabled_extensions.data()
#endif
	};

	const VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
		create_debug_utils_messenger_info();
	ins_info.pNext = &debug_create_info;

	succ = vkCreateInstance(&ins_info, nullptr, &instance_data.instance);
	check_vk(succ);
	INFO("[Vulkan Gpu Device] Instance Created..");

	// create debug info
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	assert(instance_data.instance != VK_NULL_HANDLE);
	uint32_t num_instance_extensions;
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
	std::vector<VkExtensionProperties> extensions(num_instance_extensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, extensions.data());
	const auto &result = std::find_if(extensions.begin(), extensions.end(), [](const auto &rhs) {
		return !strcmp(rhs.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	});
	if (result != extensions.end())
	{
		instance_data.debug_utils_extension_present = true;
	}
	if (!instance_data.debug_utils_extension_present)
	{
		INFO("[Vulkan Device] Extension %s for debugging non presenting",
			 VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				instance_data.instance, "vkCreateDebugUtilsMessengerEXT");
		VkDebugUtilsMessengerCreateInfoEXT debug_msger_create_info =
			create_debug_utils_messenger_info();
		vkCreateDebugUtilsMessengerEXT(instance_data.instance,
									   &debug_msger_create_info,
									   nullptr,
									   &instance_data.debug_utils_messenger);
	}
	check_true(instance_data.debug_utils_messenger != VK_NULL_HANDLE);
	INFO("[Vulkan GPU Device] DebugUtilsMessenger Created..");
#endif

	return true;
}

void DestroyVkInstance(InstanceData &instance_data)
{
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	auto vkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance_data.instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(
		instance_data.instance, instance_data.debug_utils_messenger, nullptr);
#endif
	vkDestroyInstance(instance_data.instance, nullptr);
}

bool DestroyWindowData(const InstanceData &instance_data, WindowData &window_data)
{
	vkDestroySurfaceKHR(instance_data.instance, window_data.window_surface, nullptr);
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

bool CreateVkPhysicalDevice(const InstanceData &in_instance_data,
							const WindowData &in_window_data,
							DeviceData &out_device_data)
{
	uint32_t num_physical_device = 0;
	VkResult succ =
		vkEnumeratePhysicalDevices(in_instance_data.instance, &num_physical_device, nullptr);
	check_vk(succ);

	std::vector<VkPhysicalDevice> gpus(num_physical_device);
	succ = vkEnumeratePhysicalDevices(in_instance_data.instance, &num_physical_device, gpus.data());
	check_vk(succ);
	VkPhysicalDeviceProperties device_property;
	VkPhysicalDevice discrate_device = VK_NULL_HANDLE;
	VkPhysicalDevice intergrate_device = VK_NULL_HANDLE;

	for (uint32_t index = 0; index < num_physical_device; ++index)
	{
		vkGetPhysicalDeviceProperties(gpus[index], &out_device_data.physical_device_properties);
		const VkPhysicalDeviceType device_type =
			out_device_data.physical_device_properties.deviceType;
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			if (get_family_queue(
					gpus[index], in_window_data.window_surface, out_device_data.queue_family))
			{
				discrate_device = gpus[index];
				break;
			}
			continue;
		}
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			if (get_family_queue(
					gpus[index], in_window_data.window_surface, out_device_data.queue_family))
			{
				intergrate_device = gpus[index];
				break;
			}
			continue;
		}
	}
	if (discrate_device != VK_NULL_HANDLE)
	{
		out_device_data.physical_device = discrate_device;
	}
	else if (intergrate_device != VK_NULL_HANDLE)
	{
		out_device_data.physical_device = intergrate_device;
	}
	check_true(out_device_data.physical_device != VK_NULL_HANDLE);
	out_device_data.gpu_timestamp_frequency =
		out_device_data.physical_device_properties.limits.timestampPeriod / (1000 * 1000);
	out_device_data.ubo_alignment =
		out_device_data.physical_device_properties.limits.minUniformBufferOffsetAlignment;
	out_device_data.ssbo_alignment =
		out_device_data.physical_device_properties.limits.minStorageBufferOffsetAlignment;

	INFO("[vulkan device] select gpu %s, gpu_timestamp_frequency:%f",
		 out_device_data.physical_device_properties.deviceName,
		 out_device_data.gpu_timestamp_frequency);
	return true;
}

bool CreateVkWindowDataFromGlfw(const InstanceData &instance_data,
								const GpuCreateParam &param,
								WindowData &window_data)
{
	window_data.swapchain_width = param.width;
	window_data.swapchain_height = param.height;
	auto succ = glfwCreateWindowSurface(instance_data.instance,
										static_cast<GLFWwindow *>(param.window),
										nullptr,
										&window_data.window_surface);
	check_vk(succ);
	return succ == VK_SUCCESS;
}

bool CreateVkDeviceAndQueue(DeviceData &device_data)
{
#ifdef WIN32
	std::vector<const char *> device_extensions = {"VK_KHR_swapchain"};
#else
	std::vector<const char *> device_extensions = {"VK_KHR_swapchain", "VK_KHR_portability_subset"};
#endif
	const float queue_priority[] = {1.f};
	VkDeviceQueueCreateInfo queue_info[1] = {};
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].queueFamilyIndex = device_data.queue_family;
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = queue_priority;

	VkPhysicalDeviceFeatures2 physical_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	vkGetPhysicalDeviceFeatures2(device_data.physical_device, &physical_features2);
	physical_features2.features.robustBufferAccess = VK_FALSE;

	VkDeviceCreateInfo device_cinfo = {};
	device_cinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_cinfo.queueCreateInfoCount = std::size(queue_info);
	device_cinfo.pQueueCreateInfos = queue_info;
	device_cinfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_cinfo.ppEnabledExtensionNames = device_extensions.data();
	device_cinfo.pNext = &physical_features2;

	VkResult succ =
		vkCreateDevice(device_data.physical_device, &device_cinfo, nullptr, &device_data.device);
	check_vk(succ);
	assert(device_data.device != nullptr);

	vkGetDeviceQueue(device_data.device, device_data.queue_family, 0, &device_data.queue);
	return device_data.device != nullptr;
}

bool CreateVkQueryPool(const GpuCreateParam &param, DeviceData &device_data)
{
	VkQueryPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
											  nullptr,
											  0,
											  VK_QUERY_TYPE_TIMESTAMP,
											  param.gpu_time_queries_per_frame * 2u *
												  MaxSwapchainImages,
											  0};
	vkCreateQueryPool(
		device_data.device, &pool_create_info, nullptr, &device_data.timestamp_query_pool);
	return true;
}

void DestroyVkQueryPool(DeviceData &device_data)
{
	vkDestroyQueryPool(device_data.device, device_data.timestamp_query_pool, nullptr);
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

void SetPresentMode(const WindowData &window_data,
					const DeviceData &device_data,
					PresentMode in_present_mode,
					VkPresentModeKHR &out_vk_present_mode,
					uint32_t &out_swapchain_count,
					PresentMode &out_present_mode)
{
	uint32_t supported_cnt = 0;
	static VkPresentModeKHR present_mode[8];
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device_data.physical_device, window_data.window_surface, &supported_cnt, nullptr);
	check_true(supported_cnt > 0);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device_data.physical_device, window_data.window_surface, &supported_cnt, present_mode);
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

bool CreateVkSwapChain(const DeviceData &device_data, WindowData &window_data)
{
	constexpr VkFormat surface_image_format[] = {VK_FORMAT_B8G8R8A8_UNORM,
												 VK_FORMAT_R8G8B8A8_UNORM,
												 VK_FORMAT_B8G8R8_UNORM,
												 VK_FORMAT_R8G8B8_UNORM};
	constexpr VkColorSpaceKHR surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	uint32_t supported_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device_data.physical_device, window_data.window_surface, &supported_count, nullptr);
	std::vector<VkSurfaceFormatKHR> supported_format(supported_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device_data.physical_device,
										 window_data.window_surface,
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
				window_data.window_surface_format = supported_format[j];
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
	window_data.swapchain_output.Reset();
	window_data.swapchain_output.SetColorFormat(window_data.window_surface_format.format);

	SetPresentMode(window_data,
				   device_data,
				   window_data.present_mode,
				   window_data.vk_present_mode,
				   window_data.swapchain_image_count,
				   window_data.present_mode);

	VkBool32 surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(device_data.physical_device,
										 device_data.queue_family,
										 window_data.window_surface,
										 &surface_supported);
	if (surface_supported != VK_TRUE)
	{
		FATAL("Cannot find surface support device");
	}

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device_data.physical_device, window_data.window_surface, &surface_capabilities);

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
	INFO("Create swapchain %d, %d - Saved %d %d, min image %d\n",
		 swapchain_extent.width,
		 swapchain_extent.height,
		 window_data.swapchain_width,
		 window_data.swapchain_height,
		 surface_capabilities.minImageCount);
	window_data.swapchain_width = swapchain_extent.width;
	window_data.swapchain_height = swapchain_extent.height;

	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.pNext = nullptr;
	swapchain_create_info.surface = window_data.window_surface;
	swapchain_create_info.minImageCount = window_data.swapchain_image_count;
	swapchain_create_info.imageFormat = window_data.window_surface_format.format;
	swapchain_create_info.imageExtent = swapchain_extent;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = window_data.vk_present_mode;

	VkResult succ = vkCreateSwapchainKHR(
		device_data.device, &swapchain_create_info, nullptr, &window_data.swapchain);
	check_vk(succ);

	succ = vkGetSwapchainImagesKHR(
		device_data.device, window_data.swapchain, &window_data.swapchain_image_count, nullptr);
	check_vk(succ);

	vkGetSwapchainImagesKHR(device_data.device,
							window_data.swapchain,
							&window_data.swapchain_image_count,
							window_data.swapchain_images.data());

	for (size_t i = 0; i < window_data.swapchain_image_count; i++)
	{
		VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = window_data.window_surface_format.format;
		info.image = window_data.swapchain_images[i];
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		succ = vkCreateImageView(
			device_data.device, &info, nullptr, &window_data.swapchain_image_views[i]);
	}
	return true;
}

void DestroySwapchain(const DeviceData &device_data, WindowData &window_data)
{

	for (size_t i = 0; i < window_data.swapchain_image_count; i++)
	{
		vkDestroyImageView(device_data.device, window_data.swapchain_image_views[i], nullptr);
		vkDestroyFramebuffer(device_data.device, window_data.swapchain_framebuffers[i], nullptr);
	}
	vkDestroySwapchainKHR(device_data.device, window_data.swapchain, nullptr);
}

bool CreateVmaAllocator(const InstanceData &instance_data,
						const DeviceData &device_data,
						ResourceData &resource_data)
{
	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.physicalDevice = device_data.physical_device;
	allocator_create_info.device = device_data.device;
	allocator_create_info.instance = instance_data.instance;

	VkResult succ = vmaCreateAllocator(&allocator_create_info, &resource_data.vma_allocator);
	check_vk(succ);
	return succ == VK_SUCCESS;
}

void DestroyVmaAllocator(ResourceData &resource_data)
{
	vmaDestroyAllocator(resource_data.vma_allocator);
}

// bool CreateVkRenderPass(const WindowData &window_data,
//						const DeviceData &device_data,
//						RenderPipelineData &rp_data)
//{
//	VkAttachmentDescription color_attachment{};
//	color_attachment.format = window_data.window_surface_format.format;
//	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//	VkAttachmentReference color_attachment_ref{};
//	color_attachment_ref.attachment = 0;
//	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//	VkSubpassDescription subpass{};
//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpass.colorAttachmentCount = 1;
//	subpass.pColorAttachments = &color_attachment_ref;
//
//	VkRenderPassCreateInfo render_pass_info{};
//	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	render_pass_info.attachmentCount = 1;
//	render_pass_info.pAttachments = &color_attachment;
//	render_pass_info.subpassCount = 1;
//	render_pass_info.pSubpasses = &subpass;
//
//	VkResult result =
//		vkCreateRenderPass(device_data.device, &render_pass_info, nullptr, &rp_data.render_pass);
//	check_vk(result);
//	INFO("[Vulkan Gpu Device] Render Pass Created..");
//	return result == VK_SUCCESS;
// }

// void DestroyVkRenderPass(const DeviceData &device_data, RenderPipelineData &rp_data)
//{
//	vkDestroyRenderPass(device_data.device, rp_data.render_pass, nullptr);
// }

// void CreateVkFramebuffers(const DeviceData &device_data,
//						  const RenderPipelineData &rp_data,
//						  WindowData &window_data)
//{
//	for (size_t i = 0; i < window_data.swapchain_image_count; i++)
//	{
//		VkImageView attachments[] = {window_data.swapchain_image_views[i]};
//
//		VkFramebufferCreateInfo framebuffer_info{};
//		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		framebuffer_info.renderPass = rp_data.render_pass;
//		framebuffer_info.attachmentCount = 1;
//		framebuffer_info.pAttachments = attachments;
//		framebuffer_info.width = window_data.swapchain_width;
//		framebuffer_info.height = window_data.swapchain_height;
//		framebuffer_info.layers = 1;
//
//		VkResult result = vkCreateFramebuffer(
//			device_data.device, &framebuffer_info, nullptr, &window_data.swapchain_framebuffers[i]);
//		check_vk(result);
//	}
//	INFO("[Vulkan Gpu Device] Framebuffers Created..");
// }

bool CreateVkSyncMarkers(const DeviceData &device_data, RuntimeLoopData &rl_data)
{
	VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	for (size_t i = 0; i < MaxSwapchainImages; ++i)
	{
		vkCreateSemaphore(device_data.device,
						  &semaphore_create_info,
						  nullptr,
						  &rl_data.sync_signal.render_complete_semaphore[i]);
		vkCreateSemaphore(device_data.device,
						  &semaphore_create_info,
						  nullptr,
						  &rl_data.sync_signal.image_acquired_semaphore[i]);
		VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device_data.device,
					  &fence_create_info,
					  nullptr,
					  &rl_data.sync_signal.command_buffer_fence[i]);
	}
	return true;
}

void DestroyVkSyncMarkers(const DeviceData &device_data, const RuntimeLoopData &rl_data)
{
	for (size_t i = 0; i < MaxSwapchainImages; ++i)
	{
		vkDestroySemaphore(
			device_data.device, rl_data.sync_signal.render_complete_semaphore[i], nullptr);
		vkDestroySemaphore(
			device_data.device, rl_data.sync_signal.image_acquired_semaphore[i], nullptr);
		vkDestroyFence(device_data.device, rl_data.sync_signal.command_buffer_fence[i], nullptr);
	}
}

CommandBuffer *GetInstantCommandBuffer(RuntimeLoopData &rl_data)
{
	return rl_data.command_buffer_ring.GetCommandBufferInstant(rl_data.frame_counter.current_frame,
															   false);
}

void TransitionImageLayout(VkCommandBuffer command_buffer,
						   VkImage &image,
						   VkFormat format,
						   VkImageLayout oldLayout,
						   VkImageLayout newLayout,
						   bool is_depth)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask =
		is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		// hy_assertm( false, "Unsupported layout transition!\n" );
	}

	vkCmdPipelineBarrier(
		command_buffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

Texture *Access(ResourceData &resource_data, const TextureHandle &handle)
{
	return static_cast<Texture *>(resource_data.pool_data.textures.Access(handle.index));
}

Buffer *Access(ResourceData &resource_data, const BufferHandle &handle)
{
	return static_cast<Buffer *>(resource_data.pool_data.buffers.Access(handle.index));
}

Sampler *Access(ResourceData &resource_data, const SamplerHandle &handle)
{
	return static_cast<Sampler *>(resource_data.pool_data.textures.Access(handle.index));
}

RenderPass *Access(ResourceData &resource_data, const RenderPassHandle &handle)
{ 
	return static_cast<RenderPass *>(resource_data.pool_data.render_passes.Access(handle.index));
}

void TranslateSamplerCreation(const SamplerCreation &creation,
							  VkSamplerCreateInfo &sampler_create_info)
{
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.addressModeU = creation.address_mode_u;
	sampler_create_info.addressModeV = creation.address_mode_v;
	sampler_create_info.addressModeW = creation.address_mode_w;
	sampler_create_info.minFilter = creation.min_filter;
	sampler_create_info.magFilter = creation.mag_filter;
	sampler_create_info.mipmapMode = creation.mip_filter;
	sampler_create_info.anisotropyEnable = 0;
	sampler_create_info.compareEnable = 0;
	sampler_create_info.unnormalizedCoordinates = 0;
	sampler_create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
}

void CreateSampler(VkDevice device, const SamplerCreation &creation, VkSampler &sampler)
{
	VkSamplerCreateInfo create_info;
	TranslateSamplerCreation(creation, create_info);
	auto succ = vkCreateSampler(device, &create_info, nullptr, &sampler);
	check_vk(succ);
}

SamplerHandle CreateVkSampler(const DeviceData &device_data,
							  ResourceData &resource_data,
							  const SamplerCreation &creation)
{
	SamplerHandle handle = {resource_data.pool_data.samplers.FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	Sampler *sampler =
		static_cast<Sampler *>(resource_data.pool_data.samplers.Access(handle.index));
	VkSamplerCreateInfo create_info;
	TranslateSamplerCreation(creation, create_info);
	auto succ = vkCreateSampler(device_data.device, &create_info, nullptr, &sampler->sampler);
	check_vk(succ);
	SetResourceName(device_data.device,
					VK_OBJECT_TYPE_SAMPLER,
					reinterpret_cast<uint64_t>(sampler->sampler),
					creation.name);
	return handle;
}

void DestroyVkSampler(const SamplerHandle &handle, RuntimeLoopData &rl_data)
{
	rl_data.resource_deletion_queue.push_back(
		{ResourceUpdateType::Sampler, handle.index, rl_data.frame_counter.current_frame});
}

void DestroyVkSamplerInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data)
{
	if (auto sampler = static_cast<Sampler *>(resource_data.pool_data.samplers.Access(handle)))
	{
		vkDestroySampler(device_data.device, sampler->sampler, nullptr);
	}
	resource_data.pool_data.samplers.ReleaseResource(handle);
}

BufferHandle CreateVkBuffer(const BufferCreation &creation,
							const DeviceData &device_data,
							ResourceData &resource_data)
{
	BufferHandle handle = {resource_data.pool_data.buffers.FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	Buffer *buffer = static_cast<Buffer *>(resource_data.pool_data.buffers.Access(handle.index));
	buffer->name = creation.name;
	buffer->size = creation.size;
	buffer->usage_type = creation.usage_type;
	buffer->usage_flags = creation.usage_flags;
	buffer->handle = handle;
	buffer->global_offset = 0;
	buffer->parent_handle = BufferHandle{ResourcePool::INVALID_NUM};
	static const VkBufferUsageFlags buffer_usage_mask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
														VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
														VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const bool use_global_buffer = (creation.usage_flags & buffer_usage_mask) != 0;
	if (creation.usage_type == ResourceUsageType::Dynamic && use_global_buffer)
	{
		buffer->parent_handle = resource_data.pool_data.dynamic_buffer;
		return handle;
	};
	VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | creation.usage_flags;
	buffer_create_info.size = creation.size > 0 ? creation.size : 1;

	VmaAllocationCreateInfo allocation_create_info{};
	allocation_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	VmaAllocationInfo allocation_info{};
	auto succ = vmaCreateBuffer(resource_data.vma_allocator,
								&buffer_create_info,
								&allocation_create_info,
								&buffer->buffer,
								&buffer->allocation,
								&allocation_info);
	check_vk(succ);
	SetResourceName(
		device_data.device, VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer->buffer, creation.name);
	buffer->memory = allocation_info.deviceMemory;
	if (creation.initial_data)
	{
		void *data;
		vmaMapMemory(resource_data.vma_allocator, buffer->allocation, &data);
		memcpy(data, creation.initial_data, (size_t)creation.size);
		vmaUnmapMemory(resource_data.vma_allocator, buffer->allocation);
	}
	return handle;
}

void DestroyVkBuffer(const BufferHandle &handle, RuntimeLoopData &rl_data)
{
	rl_data.resource_deletion_queue.push_back(
		{ResourceUpdateType::Buffer, handle.index, rl_data.frame_counter.current_frame});
}

void DestroyVkBufferInstance(const ResourceHandle &handle, ResourceData &resource_data)
{
	auto *buffer = static_cast<Buffer *>(resource_data.pool_data.buffers.Access(handle));
	if (buffer && buffer->parent_handle.index == ResourcePool::INVALID_NUM)
	{
		vmaDestroyBuffer(resource_data.vma_allocator, buffer->buffer, buffer->allocation);
	}
	resource_data.pool_data.buffers.ReleaseResource(handle);
}

void CreateVkTextureInner(const DeviceData &device_data,
						  const TextureCreation &creation,
						  const ResourceData &resource_data,
						  const TextureHandle &handle,
						  Texture &texture)
{
	COPY_MEMBERS(texture, creation, width, height, depth, name, mipmaps, flags, type, format);
	texture.sampler = nullptr;
	texture.handle = handle;
	VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_create_info.format = texture.format;
	image_create_info.flags = 0;
	ToVKEnum(texture.type, image_create_info.imageType);
	COPY_MEMBERS(image_create_info.extent, texture, width, height, depth);
	image_create_info.mipLevels = texture.mipmaps;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	const bool is_rt =
		(creation.flags & TextureFlags::Mask::RenderTarget) == TextureFlags::Mask::RenderTarget;
	const bool is_compute =
		(creation.flags & TextureFlags::Mask::Compute) == TextureFlags::Mask::Compute;
	image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	image_create_info.usage |= is_compute ? VK_IMAGE_USAGE_STORAGE_BIT : 0;
	if (utility::HasDepthOrStencil(creation.format))
	{
		image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_create_info.usage |= is_rt ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
	}
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo mem_info{};
	mem_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	auto succ = vmaCreateImage(resource_data.vma_allocator,
							   &image_create_info,
							   &mem_info,
							   &texture.image,
							   &texture.allocation,
							   nullptr);
	check_vk(succ);
	SetResourceName(
		device_data.device, VK_OBJECT_TYPE_IMAGE, (uint64_t)texture.image, creation.name);
	VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	info.image = texture.image;
	ToVKEnum(creation.type, info.viewType);
	info.format = creation.format;
	if (utility::HasDepthOrStencil(creation.format))
	{
		info.subresourceRange.aspectMask =
			utility::HasDepth(creation.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
	}
	else
	{
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.layerCount = 1;
	succ = vkCreateImageView(
		device_data.device, &info, resource_data.allocation_callback, &texture.view);
	check_vk(succ);
	SetResourceName(
		device_data.device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)texture.view, creation.name);
	texture.layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

TextureHandle CreateVkTexture(const DeviceData &device_data,
							  RuntimeLoopData &rl_data,
							  const TextureCreation &creation,
							  ResourceData &resource_data)
{
	TextureHandle handle = {resource_data.pool_data.textures.FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	Texture *texture =
		static_cast<Texture *>(resource_data.pool_data.textures.Access(handle.index));
	CreateVkTextureInner(device_data, creation, resource_data, handle, *texture);
	if (creation.initial_data)
	{
		VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		uint32_t image_size = creation.width * creation.height * 4;
		buffer_info.size = image_size;

		VmaAllocationCreateInfo memory_info{};
		memory_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		memory_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocationInfo allocation_info{};
		VkBuffer staging_buffer;
		VmaAllocation staging_allocation;
		check_vk(vmaCreateBuffer(resource_data.vma_allocator,
								 &buffer_info,
								 &memory_info,
								 &staging_buffer,
								 &staging_allocation,
								 &allocation_info));

		// Copy buffer_data
		void *destination_data;
		vmaMapMemory(resource_data.vma_allocator, staging_allocation, &destination_data);
		memcpy(destination_data, creation.initial_data, static_cast<size_t>(image_size));
		vmaUnmapMemory(resource_data.vma_allocator, staging_allocation);

		// Execute command buffer
		VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CommandBuffer *command_buffer = GetInstantCommandBuffer(rl_data);
		vkBeginCommandBuffer(command_buffer->vk_command_buffer, &beginInfo);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {creation.width, creation.height, creation.depth};

		// Transition
		TransitionImageLayout(command_buffer->vk_command_buffer,
							  texture->image,
							  texture->format,
							  VK_IMAGE_LAYOUT_UNDEFINED,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  false);
		// Copy
		vkCmdCopyBufferToImage(command_buffer->vk_command_buffer,
							   staging_buffer,
							   texture->image,
							   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   1,
							   &region);
		// Transition
		TransitionImageLayout(command_buffer->vk_command_buffer,
							  texture->image,
							  texture->format,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
							  false);

		vkEndCommandBuffer(command_buffer->vk_command_buffer);

		// Submit command buffer
		VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command_buffer->vk_command_buffer;

		vkQueueSubmit(device_data.queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device_data.queue);

		vmaDestroyBuffer(resource_data.vma_allocator, staging_buffer, staging_allocation);

		// TODO: free command buffer
		vkResetCommandBuffer(command_buffer->vk_command_buffer,
							 VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		texture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	return;
}

void DestroyTexture(RuntimeLoopData &rl_data, TextureHandle &handle)
{
	rl_data.resource_deletion_queue.push_back(
		{ResourceUpdateType::Texture, handle.index, rl_data.frame_counter.current_frame});
}

void DestroyVkSamplerInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data)
{
	Texture *texture = static_cast<Texture *>(resource_data.pool_data.textures.Access(handle));
	if (texture)
	{
		vkDestroyImageView(device_data.device, texture->view, resource_data.allocation_callback);
		vmaDestroyImage(resource_data.vma_allocator, texture->image, texture->allocation);
	}
	resource_data.pool_data.textures.ReleaseResource(handle);
}


void CreateSwapchainRenderPass(RenderPass& render_pass)
{ VkAttachmentDescription color_attach{};
	// todo 
}

RenderPassHandle CreateVkRenderPass(const RenderPassCreation &creation, ResourceData &resource_data)
{
	RenderPassHandle handle = {resource_data.pool_data.render_passes.FetchResource()};
	if (handle.index == ResourcePool::INVALID_NUM)
	{
		return handle;
	}
	RenderPass *rp = Access(resource_data, handle);
	rp->name = creation.name;
	rp->type = creation.type;
	rp->num_render_targets = creation.num_render_targets;
	rp->dispatch_x = 0;
	rp->dispatch_y = 0;
	rp->dispatch_z = 0;
	rp->vk_frame_buffer = nullptr;
	rp->vk_frame_buffer = nullptr;
	rp->scale_x = creation.scale_x;
	rp->scale_y = creation.scale_y;
	rp->resize = creation.resize;

	uint32_t index = 0;
	for (; index < creation.num_render_targets; index++)
	{
		Texture *tex = Access(resource_data, creation.output_textures[index]);
		rp->width = tex->width;
		rp->height = tex->height;
		rp->out_textures[index] = creation.output_textures[index];
	}
	rp->out_depth = creation.depth_stencil_texture;
	if (creation.type == RenderPassType::SwapChain)
	{
		// todo 
		CreateSwapchainRenderPass();
	}
	else if (creation.type == RenderPassType::Compute)
	{
		//todo
	}
	else if (creation.type == RenderPassType::Geometry)
	{
		// todo
	}

	return handle;
}

} // namespace cloud::vulkan::infra