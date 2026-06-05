#include <algorithm>
#include <functional>
#include "graphics/vulkan/vulkan_interface.h"
#include "core/runtime_log.h"
#include "graphics/vulkan/device_data.h"
#include "core/data_structure/memory.h"



#define COPY_MEMBER(obj_left, obj_right, member) obj_left.member = obj_right.member

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

void SetResourceName(VkDevice device, VkObjectType type, uint64_t ptr, const char *name)
{
	VkDebugUtilsObjectNameInfoEXT name_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
	name_info.objectType = type;
	name_info.objectHandle = ptr;
	name_info.pObjectName = name;
	pfnSetDebugUtilsObjectNameEXT(device, &name_info);
	INFO("Resource \"%s\" created with handle %d", name, ptr);
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
	for (int i = 0; i < ArraySize(s_requested_extensions); ++i)
	{
		instance_data.enabled_extensions.push_back(s_requested_extensions[i]);
	}
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
//#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
		.enabledLayerCount = std::size(s_instance_layer),
		.ppEnabledLayerNames = s_instance_layer,
		.enabledExtensionCount = static_cast<uint32_t>(instance_data.enabled_extensions.size()),
		.ppEnabledExtensionNames = instance_data.enabled_extensions.data()
//#endif
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

void DestroyWindowSurface(const InstanceData &instance_data, WindowData &window_data)
{ vkDestroySurfaceKHR(instance_data.instance, window_data.window_surface, nullptr); }

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

bool CreateVkWindowSurfaceFromGlfw(const InstanceData &instance_data,
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

	InitVulkanInterface(device_data.device);
	return device_data.device != nullptr;
}

void DestroyVkDeviceAndQueue(DeviceData &device_data)
{
	vkDestroyDevice(device_data.device, device_data.allocation_callback);
	device_data.queue = VK_NULL_HANDLE;
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
{ vkDestroyQueryPool(device_data.device, device_data.timestamp_query_pool, nullptr); }

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

void DestroyVkSwapchain(const DeviceData &device_data, WindowData &window_data)
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
{ vmaDestroyAllocator(resource_data.vma_allocator); }

bool CreateVkDescriptorPool(const DeviceData &device_data, ResourceData &resource_data)
{
	VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	auto &pool_sizes = resource_data.pool_data.pool_sizes;
	pool_info.maxSets = resource_data.pool_data.k_global_pool_elements * ArraySize(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)ArraySize(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VkResult succ = vkCreateDescriptorPool(device_data.device,
										   &pool_info,
										   device_data.allocation_callback,
										   &resource_data.pool_data.vk_descriptor_pool);
	return succ == VK_SUCCESS;
}

void DestroyVkDescriptorPool(const DeviceData &device_data, ResourceData &resource_data)
{
	vkDestroyDescriptorPool(device_data.device,
							resource_data.pool_data.vk_descriptor_pool,
							device_data.allocation_callback);
}

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

bool InitRuntimeLoopData(const DeviceData &device_data, RuntimeLoopData &rl_data)
{
	rl_data.command_buffer_ring.Init(device_data.device, device_data.queue_family);
	rl_data.frame_counter.current_frame = 1;
	rl_data.frame_counter.previous_frame = 0;
	rl_data.frame_counter.absolute_frame = 0;
	rl_data.frame_counter.timestamps_enabled = false;
	rl_data.resource_deletion_queue.clear();
	rl_data.descriptor_set_updates.clear();
	return true;
}

void DestoryRuntimeLoopData(const DeviceData &device_data, RuntimeLoopData &rl_data)
{ rl_data.command_buffer_ring.Destroy(device_data.device); }

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
{ return AccessTexture(resource_data, handle.index); }

Buffer *Access(ResourceData &resource_data, const BufferHandle &handle)
{ return AccessBuffer(resource_data, handle.index); }

Sampler *Access(ResourceData &resource_data, const SamplerHandle &handle)
{ return AccessSampler(resource_data, handle.index); }

RenderPass *Access(ResourceData &resource_data, const RenderPassHandle &handle)
{ return AccessRenderPass(resource_data, handle.index); }

Texture *AccessTexture(ResourceData &resource_data, const ResourceHandle &handle)
{ return static_cast<Texture *>(resource_data.pool_data.textures.Access(handle)); }

Buffer *AccessBuffer(ResourceData &resource_data, const ResourceHandle &handle)
{ return static_cast<Buffer *>(resource_data.pool_data.buffers.Access(handle)); }

Sampler *AccessSampler(ResourceData &resource_data, const ResourceHandle &handle)
{ return static_cast<Sampler *>(resource_data.pool_data.textures.Access(handle)); }

RenderPass *AccessRenderPass(ResourceData &resource_data, const ResourceHandle &handle)
{ return static_cast<RenderPass *>(resource_data.pool_data.render_passes.Access(handle)); }

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
	VkSamplerCreateInfo create_info{};
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
		INFO("resource %s delete, handle %d", sampler->name, handle);
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
		buffer->parent_handle = resource_data.dynamic_buffer.buffer;
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
	#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
		INFO("%s crearted", buffer->name);
	#endif
	return handle;
}

void DestroyVkBuffer(const BufferHandle &handle, RuntimeLoopData &rl_data)
{
	rl_data.resource_deletion_queue.push_back(
		{ResourceUpdateType::Buffer, handle.index, rl_data.frame_counter.current_frame});
}

void DestroyVkBufferInstance(const ResourceHandle &handle,
							 const DeviceData &device_data,
							 ResourceData &resource_data)
{
	Buffer *buffer = AccessBuffer(resource_data, handle);
	INFO("resource %s delete, handle %d", buffer->name, handle);
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
	COPY_MEMBER(texture, creation, width);
	COPY_MEMBER(texture, creation, height);
	COPY_MEMBER(texture, creation, depth);
	COPY_MEMBER(texture, creation, name);
	COPY_MEMBER(texture, creation, mipmaps);
	COPY_MEMBER(texture, creation, flags);
	COPY_MEMBER(texture, creation, type);
	COPY_MEMBER(texture, creation, format);

	texture.sampler = nullptr;
	texture.handle = handle;
	VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_create_info.format = texture.format;
	image_create_info.flags = 0;
	ToVKEnum(texture.type, image_create_info.imageType);
	COPY_MEMBER(image_create_info.extent, texture, width);
	COPY_MEMBER(image_create_info.extent, texture, height);
	COPY_MEMBER(image_create_info.extent, texture, depth);
	image_create_info.mipLevels = texture.mipmaps;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	const bool is_rt = HasAny(creation.flags, TextureFlags::Mask::RenderTarget);
	const bool is_compute = HasAny(creation.flags, TextureFlags::Mask::Compute);
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
	VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
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
	return handle;
}

void DestroyVkTexture(TextureHandle &handle, RuntimeLoopData &rl_data)
{
	rl_data.resource_deletion_queue.push_back(
		{ResourceUpdateType::Texture, handle.index, rl_data.frame_counter.current_frame});
}

void DestroyVkTextureInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data)
{
	Texture *tex = AccessTexture(resource_data, handle);
	INFO("resource %s delete, handle %d", tex->name, handle);
	if (tex)
	{
		vkDestroyImageView(device_data.device, tex->view, device_data.allocation_callback);
		vmaDestroyImage(resource_data.vma_allocator, tex->image, tex->allocation);
	}
	resource_data.pool_data.textures.ReleaseResource(handle);
}

void CreateVkSwapchainRenderPass(const DeviceData &device_data,
								 RuntimeLoopData &rl_data,
								 WindowData &window_data,
								 ResourceData &resource_data,
								 RenderPass &render_pass)
{
	VkAttachmentDescription color_attach{};
	color_attach.format = window_data.window_surface_format.format;
	color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attach_ref{};
	color_attach_ref.attachment = 0;
	color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	Texture *depth_tex = Access(resource_data, resource_data.texture_depth_handle);
	VkAttachmentDescription depth_attach{};
	depth_attach.format = depth_tex->format;
	depth_attach.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attach_ref{};
	depth_attach_ref.attachment = 1;
	depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attach_ref;
	subpass.pDepthStencilAttachment = &depth_attach_ref;

	VkAttachmentDescription attaches[] = {color_attach, depth_attach};
	VkRenderPassCreateInfo rpinfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	rpinfo.attachmentCount = 2;
	rpinfo.pAttachments = attaches;
	rpinfo.subpassCount = 1;
	rpinfo.pSubpasses = &subpass;
	check_vk(vkCreateRenderPass(device_data.device, &rpinfo, nullptr, &render_pass.vk_render_pass));
	SetResourceName(device_data.device,
					VK_OBJECT_TYPE_RENDER_PASS,
					(uint64_t)render_pass.vk_render_pass,
					render_pass.name);
	VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	fb_info.renderPass = render_pass.vk_render_pass;
	fb_info.attachmentCount = 2;
	fb_info.width = window_data.swapchain_width;
	fb_info.height = window_data.swapchain_height;
	fb_info.layers = 1;

	VkImageView fb_attaches[2];
	fb_attaches[1] = depth_tex->view;
	// todo 这个循环说不通
	for (size_t i = 0; i < window_data.swapchain_image_count; i++)
	{
		fb_attaches[0] = window_data.swapchain_image_views[i];
		fb_info.pAttachments = fb_attaches;
		vkCreateFramebuffer(
			device_data.device, &fb_info, nullptr, &window_data.swapchain_framebuffers[i]);
		SetResourceName(device_data.device,
						VK_OBJECT_TYPE_FRAMEBUFFER,
						(uint64_t)window_data.swapchain_framebuffers[i],
						render_pass.name);
	}

	render_pass.width = window_data.swapchain_width;
	render_pass.height = window_data.swapchain_height;

	VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CommandBuffer *cb = GetInstantCommandBuffer(rl_data);
	vkBeginCommandBuffer(cb->vk_command_buffer, &info);
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {render_pass.width, render_pass.height, 1};
	for (size_t i = 0; i < window_data.swapchain_image_count; ++i)
	{
		TransitionImageLayout(cb->vk_command_buffer,
							  window_data.swapchain_images[i],
							  window_data.window_surface_format.format,
							  VK_IMAGE_LAYOUT_UNDEFINED,
							  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
							  false);
	}
	vkEndCommandBuffer(cb->vk_command_buffer);
	VkSubmitInfo sub_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	sub_info.commandBufferCount = 1;
	sub_info.pCommandBuffers = &cb->vk_command_buffer;
	vkQueueSubmit(device_data.queue, 1, &sub_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(device_data.queue);
}

void CreateVkFrameBuffer(const DeviceData &device_data,
						 ResourceData &resource_data,
						 RenderPass &rp,
						 const TextureHandle *out_textures,
						 const uint32_t num_rt,
						 const TextureHandle &depth_stencil_tex)
{
	VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	fb_info.renderPass = rp.vk_render_pass;
	fb_info.width = rp.width;
	fb_info.height = rp.height;
	fb_info.layers = 1;

	VkImageView fb_attchs[MaxSwapchainImages + 1]{};
	uint32_t active_attachs = 0;
	for (; active_attachs < num_rt; ++active_attachs)
	{
		Texture *tex = Access(resource_data, out_textures[active_attachs]);
		fb_attchs[active_attachs] = tex->view;
	}
	if (depth_stencil_tex.index != ResourcePool::INVALID_NUM)
	{
		Texture *tex = Access(resource_data, depth_stencil_tex);
		fb_attchs[active_attachs++] = tex->view;
	}
	fb_info.pAttachments = fb_attchs;
	fb_info.attachmentCount = active_attachs;
	check_vk(vkCreateFramebuffer(device_data.device, &fb_info, nullptr, &rp.vk_frame_buffer));
	SetResourceName(
		device_data.device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)rp.vk_frame_buffer, rp.name);
}

RenderPassOutput FillRenderPassOutput(const RenderPassCreation &creation,
									  ResourceData &resource_data)
{
	RenderPassOutput out;
	out.Reset();
	for (uint32_t i = 0; i < creation.num_render_targets; ++i)
	{
		Texture *tex = Access(resource_data, creation.output_textures[i]);
		out.SetColorFormat(tex->format);
	}
	if (creation.depth_stencil_texture.index != ResourcePool::INVALID_NUM)
	{
		Texture *tex = Access(resource_data, creation.depth_stencil_texture);
		out.SetDepthFormat(tex->format);
	}
	out.color_operation = creation.color_op;
	out.depth_operation = creation.depth_op;
	out.stencil_operation = creation.stencil_op;
	return out;
}

VkRenderPass CreateVkRenderPassInner(const DeviceData &device_data,
									 ResourceData &resource_data,
									 const RenderPassOutput &output,
									 const char *name)
{
	VkAttachmentDescription color_attachs[8] = {};
	VkAttachmentReference color_attachs_ref[8] = {};
	VkAttachmentLoadOp color_op, depth_op, stencil_op;
	VkImageLayout color_initial, depth_initial;
	switch (output.color_operation)
	{
	case RenderPassOperation::Load:
		color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
		color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		break;
	case RenderPassOperation::Clear:
		color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		break;
	default:
		color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
		break;
	}
	switch (output.depth_operation)
	{
	case RenderPassOperation::Load:
		depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
		depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		break;
	case RenderPassOperation::Clear:
		depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		break;
	default:
		depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
		break;
	}
	switch (output.stencil_operation)
	{
	case RenderPassOperation::Load:
		stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
		break;
	case RenderPassOperation::Clear:
		stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		break;
	default:
		stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		break;
	}
	uint32_t c_index = 0;
	for (; c_index < output.num_color_formats; ++c_index)
	{
		VkAttachmentDescription &color_attach = color_attachs[c_index];
		color_attach.format = output.color_formats[c_index];
		color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attach.loadOp = color_op;
		color_attach.stencilLoadOp = stencil_op;
		color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attach.initialLayout = color_initial;
		color_attach.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		auto &color_ref = color_attachs_ref[c_index];
		color_ref.attachment = c_index;
		color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	VkAttachmentDescription depth_attach{};
	VkAttachmentReference depth_ref{};
	if (output.depth_stencil_format != VK_FORMAT_UNDEFINED)
	{
		depth_attach.format = output.depth_stencil_format;
		depth_attach.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attach.loadOp = depth_op;
		depth_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attach.stencilLoadOp = stencil_op;
		depth_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attach.initialLayout = depth_initial;
		depth_attach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_ref.attachment = c_index;
		depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkAttachmentDescription attachs[MaxSwapchainImages + 1]{};
	uint32_t active_attach = 0;
	for (; active_attach < output.num_color_formats; ++active_attach)
	{
		attachs[active_attach] = color_attachs[active_attach];
		++active_attach;
	}
	subpass.pDepthStencilAttachment = nullptr;
	uint32_t depth_stencil_count = 0;
	if (output.depth_stencil_format != VK_FORMAT_UNDEFINED)
	{
		attachs[subpass.colorAttachmentCount] = depth_attach;
		subpass.pDepthStencilAttachment = &depth_ref;
		depth_stencil_count = 1;
	}
	VkRenderPassCreateInfo rp_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	rp_info.attachmentCount = (active_attach ? active_attach - 1 : 0) + depth_stencil_count;
	rp_info.pAttachments = attachs;
	rp_info.subpassCount = 1;
	VkRenderPass vk_rp;
	check_vk(vkCreateRenderPass(device_data.device, &rp_info, nullptr, &vk_rp));
	SetResourceName(device_data.device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)vk_rp, name);
	return vk_rp;
}

VkRenderPass GetVkRenderPass(const DeviceData &device_data,
							 ResourceData &resource_data,
							 const RenderPassOutput &output,
							 const char *name)
{
	size_t rp_hash = 0;
	render::hash_combine(rp_hash, (void *)&output);
	auto iter = resource_data.render_pass_cache.find(rp_hash);
	if (iter != resource_data.render_pass_cache.end())
	{
		return iter->second;
	}
	VkRenderPass vk_rp = CreateVkRenderPassInner(device_data, resource_data, output, name);
	resource_data.render_pass_cache.emplace(rp_hash, vk_rp);
	return vk_rp;
}

RenderPassHandle CreateVkRenderPass(const RenderPassCreation &creation,
									const DeviceData &device_data,
									RuntimeLoopData &rl_data,
									WindowData &window_data,
									ResourceData &resource_data)
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
	rp->vk_frame_buffer = VK_NULL_HANDLE;
	rp->vk_frame_buffer = VK_NULL_HANDLE;
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
		CreateVkSwapchainRenderPass(device_data, rl_data, window_data, resource_data, *rp);
	}
	else if (creation.type == RenderPassType::Compute)
	{
		// todo not implement now!
	}
	else if (creation.type == RenderPassType::Geometry)
	{
		rp->output = FillRenderPassOutput(creation, resource_data);
		rp->vk_render_pass =
			CreateVkRenderPassInner(device_data, resource_data, rp->output, rp->name);
		CreateVkFrameBuffer(device_data,
							resource_data,
							*rp,
							creation.output_textures,
							creation.num_render_targets,
							creation.depth_stencil_texture);
	}

	return handle;
}

void DestroyVkRenderPass(RenderPassHandle &handle, RuntimeLoopData &rl_data)
{

	rl_data.resource_deletion_queue.emplace_back(
		ResourceUpdateType::RenderPass, handle.index, rl_data.frame_counter.current_frame);
}

void DestroyVkRenderPassInstance(const ResourceHandle &handle,
								 const DeviceData &device_data,
								 ResourceData &resource_data)
{
	RenderPass *rp = AccessRenderPass(resource_data, handle);
	INFO("resource %s delete, handle %d", rp->name, handle);
	if (rp)
	{
		if (rp->num_render_targets)
		{
			vkDestroyFramebuffer(
				device_data.device, rp->vk_frame_buffer, resource_data.allocation_callback);
		}
		resource_data.pool_data.render_passes.ReleaseResource(handle);
	}
}

void *DynamicAllocate(DynamicBuffer &dynamic_buffer, uint32_t size)
{
	void *memory = dynamic_buffer.mapped_memory + dynamic_buffer.allocated_size;
	dynamic_buffer.allocated_size += (uint32_t)cloud::MemoryAlign(size, GUboAlignment);
	return memory;
}

void *MapBuffer(const DynamicBuffer::MapBufferParameters &param,
				DynamicBuffer &dynamic_buffer,
				ResourceData &resource_data)
{
	if (param.handle.index == ResourcePool::INVALID_NUM)
		return nullptr;
	Buffer *buffer = Access(resource_data, param.handle);
	if (buffer->parent_handle.index == dynamic_buffer.buffer.index)
	{
		buffer->global_offset = dynamic_buffer.allocated_size;
		return DynamicAllocate(dynamic_buffer, param.size == 0 ? buffer->size : param.size);
	}
	void *data;
	vmaMapMemory(resource_data.vma_allocator, buffer->allocation, &data);
	return data;
}

void UnMapBuffer(const DynamicBuffer::MapBufferParameters &param,
				 DynamicBuffer &dynamic_buffer,
				 ResourceData &resource_data)
{
	if (param.handle.index == ResourcePool::INVALID_NUM)
		return;
	Buffer *buffer = Access(resource_data, param.handle);
	if (buffer->parent_handle.index == dynamic_buffer.buffer.index)
		return;
	vmaUnmapMemory(resource_data.vma_allocator, buffer->allocation);
}

using instance_delete_handler =
	std::function<void(const ResourceHandle &, const DeviceData &device_data, ResourceData &)>;
std::unordered_map<ResourceUpdateType, instance_delete_handler> s_delete_map = {
	{ResourceUpdateType::Buffer, DestroyVkBufferInstance},
	{ResourceUpdateType::Texture, DestroyVkTextureInstance},
	{ResourceUpdateType::Sampler, DestroyVkSamplerInstance},
	{ResourceUpdateType::RenderPass, DestroyVkRenderPassInstance},
};

void DestroyResourceInstance(RuntimeLoopData &rl_data,
							 const DeviceData &device_data,
							 ResourceData &resource_data)
{
	for (uint32_t i = 0; i < rl_data.resource_deletion_queue.size(); ++i)
	{
		ResourceUpdate &res_to_delete = rl_data.resource_deletion_queue[i];

		if (res_to_delete.current_frame == -1)
			continue;
		auto iter = s_delete_map.find(res_to_delete.type);
		if (iter != s_delete_map.end())
		{
			const auto &handle = res_to_delete.handle;
			iter->second(res_to_delete.handle, device_data, resource_data);
		}
		else
		{
			FATAL("resource type %d has not delete instance handler!",
				  (uint32_t)res_to_delete.type);
		}
	}

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
} // namespace cloud::vulkan::infra