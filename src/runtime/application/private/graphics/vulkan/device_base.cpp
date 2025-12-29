#include "graphics/vulkan/device_base.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "runtime_log.h"

#include <algorithm>

namespace cloud::vulkan
{
CommandBufferRing g_vulkan_cmd_buffer_ring;

void CommandBufferRing::Init(DeviceBase *gpu)
{
	for (uint32_t i = 0; i < GMaxPools; i++)
	{
		VkCommandPoolCreateInfo cmd_pool_info{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
		};
		cmd_pool_info.queueFamilyIndex = gpu->queue_family;
		cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		check_vk(vkCreateCommandPool(gpu->device, &cmd_pool_info, nullptr, &vk_command_pool[i]));
	}

	for (uint32_t i = 0; i < GMaxBuffers; i++)
	{
		VkCommandBufferAllocateInfo cmd_alloc_cmd = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
													 nullptr};
		const uint32_t pool_index = IndexInPool(i);
		cmd_alloc_cmd.commandPool = vk_command_pool[pool_index];
		cmd_alloc_cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_alloc_cmd.commandBufferCount = 1;
		check_vk(vkAllocateCommandBuffers(
			gpu->device, &cmd_alloc_cmd, &command_buffer[i].vk_command_buffer));
		command_buffer[i].handle = i;
		command_buffer[i].Reset();
	}
}

void CommandBufferRing::Destroy(DeviceBase *gpu)
{
	for (uint32_t i = 0; i < GMaxPools; ++i)
	{
		vkDestroyCommandPool(gpu->device, vk_command_pool[i], nullptr);
	}
}

void CommandBufferRing::Reset(DeviceBase *gpu, uint32_t frame_index)
{
	for (uint32_t i = 0; i < GMaxThreads; ++i)
	{
		vkResetCommandPool(gpu->device, vk_command_pool[frame_index * GMaxThreads + i], 0);
	}
}

CommandBuffer *CommandBufferRing::GetCommandBuffer(uint32_t frame_index, bool begin)
{
	CommandBuffer *cmd_buffer = &command_buffer[frame_index * GBuffersPerPool];
	if (begin)
	{
		cmd_buffer->Reset();
		VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd_buffer->vk_command_buffer, &begin_info);
	}
	return cmd_buffer;
}

CommandBuffer *CommandBufferRing::GetCommandBufferInstant(uint32_t frame_index, bool begin)
{
	CommandBuffer *cmd_buffer = &command_buffer[frame_index * GBuffersPerPool + 1];
	return cmd_buffer;
}
DeviceBase::~DeviceBase() { INFO("vulkan device destroyed"); }

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

void DeviceBase::CreateDebugExt()
{
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	assert(instance != VK_NULL_HANDLE);
	uint32_t num_instance_extensions;
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
	std::vector<VkExtensionProperties> extensions(num_instance_extensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, extensions.data());
	const auto &result = std::find_if(extensions.begin(), extensions.end(), [](const auto &rhs) {
		return !strcmp(rhs.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	});
	if (result != extensions.end())
	{
		debug_utils_extension_present = true;
	}
	if (!debug_utils_extension_present)
	{
		INFO("[Vulkan Device] Extension {} for debugging non presenting",
			 VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				instance, "vkCreateDebugUtilsMessengerEXT");
		VkDebugUtilsMessengerCreateInfoEXT debug_msger_create_info =
			create_debug_utils_messenger_info();
		vkCreateDebugUtilsMessengerEXT(
			instance, &debug_msger_create_info, nullptr, &debug_utils_messenger);
	}
	check_true(debug_utils_messenger != VK_NULL_HANDLE);
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

void DeviceBase::CreateInstance(GpuCreateParam &param)
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

	succ = vkCreateInstance(&ins_info, nullptr, &instance);
	check_vk(succ);
	INFO("[Vulkan Gpu Device] Instance Created..");
}

void DeviceBase::CreatePhysicalDevice()
{
	uint32_t num_physical_device = 0;
	VkResult succ = vkEnumeratePhysicalDevices(instance, &num_physical_device, nullptr);
	check_vk(succ);

	std::vector<VkPhysicalDevice> gpus(num_physical_device);
	succ = vkEnumeratePhysicalDevices(instance, &num_physical_device, gpus.data());
	check_vk(succ);
	VkPhysicalDeviceProperties device_property;
	VkPhysicalDevice discrate_device = VK_NULL_HANDLE;
	VkPhysicalDevice intergrate_device = VK_NULL_HANDLE;

	for (uint32_t index = 0; index < num_physical_device; ++index)
	{
		vkGetPhysicalDeviceProperties(gpus[index], &physical_device_properties);
		const VkPhysicalDeviceType device_type = physical_device_properties.deviceType;
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			if (get_family_queue(gpus[index], window_surface, queue_family))
			{
				discrate_device = gpus[index];
				break;
			}
			continue;
		}
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			if (get_family_queue(gpus[index], window_surface, queue_family))
			{
				intergrate_device = gpus[index];
				break;
			}
			continue;
		}
	}
	if (discrate_device != VK_NULL_HANDLE)
	{
		physical_device = discrate_device;
	}
	else if (intergrate_device != VK_NULL_HANDLE)
	{
		physical_device = intergrate_device;
	}
	check_true(physical_device != VK_NULL_HANDLE);
	gpu_timestamp_frequency = physical_device_properties.limits.timestampPeriod / (1000 * 1000);
	ubo_alignment = physical_device_properties.limits.minUniformBufferOffsetAlignment;
	ssbo_alignment = physical_device_properties.limits.minStorageBufferOffsetAlignment;

	INFO("[vulkan device] select gpu {}, gpu_timestamp_frequency:{:.8f}",
		 physical_device_properties.deviceName,
		 gpu_timestamp_frequency);
}

void DeviceBase::CreateDeviceAndQueue()
{
#ifdef WIN32
	std::vector<const char *> device_extensions = {"VK_KHR_swapchain"};
#else
	std::vector<const char *> device_extensions = {"VK_KHR_swapchain", "VK_KHR_portability_subset"};
#endif
	const float queue_priority[] = {1.f};
	VkDeviceQueueCreateInfo queue_info[1] = {};
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].queueFamilyIndex = queue_family;
	queue_info[0].queueCount = 1;
	queue_info[0].pQueuePriorities = queue_priority;

	VkPhysicalDeviceFeatures2 physical_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	vkGetPhysicalDeviceFeatures2(physical_device, &physical_features2);
	physical_features2.features.robustBufferAccess = VK_FALSE;

	VkDeviceCreateInfo device_cinfo = {};
	device_cinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_cinfo.queueCreateInfoCount = std::size(queue_info);
	device_cinfo.pQueueCreateInfos = queue_info;
	device_cinfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_cinfo.ppEnabledExtensionNames = device_extensions.data();
	device_cinfo.pNext = &physical_features2;

	VkResult succ = vkCreateDevice(physical_device, &device_cinfo, nullptr, &device);
	check_vk(succ);
	assert(device != nullptr);

	vkGetDeviceQueue(device, queue_family, 0, &queue);
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

void SetPresentMode(DeviceBase *gpu,
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
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		gpu->physical_device, gpu->window_surface, &supported_cnt, present_mode);
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

void DeviceBase::CreateSwapChain()
{
	constexpr VkFormat surface_image_format[] = {VK_FORMAT_B8G8R8A8_UNORM,
												 VK_FORMAT_R8G8B8A8_UNORM,
												 VK_FORMAT_B8G8R8_UNORM,
												 VK_FORMAT_R8G8B8_UNORM};
	constexpr VkColorSpaceKHR surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	uint32_t supported_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		physical_device, window_surface, &supported_count, nullptr);
	std::vector<VkSurfaceFormatKHR> supported_format(supported_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		physical_device, window_surface, &supported_count, supported_format.data());

	bool format_found = false;
	for (int i = 0; i < ArraySize(surface_image_format); ++i)
	{
		for (int j = 0; j < supported_count; ++j)
		{
			if (supported_format[j].format == surface_image_format[i] &&
				supported_format[j].colorSpace == surface_color_space)
			{
				window_surface_format = supported_format[j];
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
	swapchain_output.Reset();
	swapchain_output.SetColorFormat(window_surface_format.format);

	SetPresentMode(this, present_mode, vk_present_mode, swapchain_image_count, present_mode);

	VkBool32 surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(
		physical_device, queue_family, window_surface, &surface_supported);
	if (surface_supported != VK_TRUE)
	{
		FATAL("Cannot find surface support device");
	}

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device, window_surface, &surface_capabilities);

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
		 swapchain_width,
		 swapchain_height,
		 surface_capabilities.minImageCount);
	swapchain_width = swapchain_extent.width;
	swapchain_height = swapchain_extent.height;

	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.pNext = nullptr;
	swapchain_create_info.surface = window_surface;
	swapchain_create_info.minImageCount = swapchain_image_count;
	swapchain_create_info.imageFormat = window_surface_format.format;
	swapchain_create_info.imageExtent = swapchain_extent;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = vk_present_mode;

	VkResult succ = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
	check_vk(succ);

	succ = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
	check_vk(succ);

	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data());

	for (size_t i = 0; i < swapchain_image_count; i++)
	{
		VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = window_surface_format.format;
		info.image = swapchain_images[i];
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		succ = vkCreateImageView(device, &info, nullptr, &swapchain_image_views[i]);
	}
}

void DeviceBase::DestroySwapChain()
{
	for (size_t i = 0; i < swapchain_image_count; i++)
	{
		vkDestroyImageView(device, swapchain_image_views[i], nullptr);
		// vkDestroyFramebuffer(g_vulkan_device.device, g_vulkan_device.swapchain_freamebuffers[i],
		// nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void DeviceBase::CreateVmaAllocator()
{
	VmaAllocatorCreateInfo allocator_create_info = {};
	allocator_create_info.physicalDevice = physical_device;
	allocator_create_info.device = device;
	allocator_create_info.instance = instance;

	VkResult succ = vmaCreateAllocator(&allocator_create_info, &vma_allocator);
	check_vk(succ);
}

void DeviceBase::CreateDescriptorPool()
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
	VkResult succ = vkCreateDescriptorPool(device, &pool_create_info, nullptr, &descriptor_pool);
	check_vk(succ);
}

void DeviceBase::CreateQueryPool(const GpuCreateParam &param)
{
	VkQueryPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
											  nullptr,
											  0,
											  VK_QUERY_TYPE_TIMESTAMP,
											  param.gpu_time_queries_per_frame * 2u *
												  MaxSwapchainImages,
											  0};
	vkCreateQueryPool(device, &pool_create_info, nullptr, &timestamp_query_pool);
}

void DeviceBase::CreateSyncMarkers()
{
	VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	for (size_t i = 0; i < MaxSwapchainImages; ++i)
	{
		vkCreateSemaphore(device, &semaphore_create_info, nullptr, &render_complete_semaphore[i]);
		vkCreateSemaphore(device, &semaphore_create_info, nullptr, &image_acquired_semaphore[i]);
		VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fence_create_info, nullptr, &command_buffer_fence[i]);
	}
}

void DeviceBase::DestroySyncMarkers()
{
	for (size_t i = 0; i < swapchain_image_count; ++i)
	{
		vkDestroySemaphore(device, render_complete_semaphore[i], nullptr);
		vkDestroySemaphore(device, image_acquired_semaphore[i], nullptr);
		vkDestroyFence(device, command_buffer_fence[i], nullptr);
	}
}

void DeviceBase::Init(GpuCreateParam &param)
{
	INFO("[Vulkan Gpu Device] Start init...");
	VkResult succ;

	// instance
	CreateInstance(param);
	assert(instance);

	// messenger
	CreateDebugExt();

	// surface creation
	swapchain_width = param.width;
	swapchain_height = param.height;
	succ = glfwCreateWindowSurface(
		instance, static_cast<GLFWwindow *>(param.window), nullptr, &window_surface);
	check_vk(succ);

	CreatePhysicalDevice();
	assert(physical_device);

	CreateDeviceAndQueue();
	assert(device);
	assert(queue);

	infra::InitVulkanInterface(device);

	CreateSwapChain();
	assert(swapchain);

	CreateVmaAllocator();
	assert(vma_allocator);

	CreateDescriptorPool();
	assert(descriptor_pool);

	CreateQueryPool(param);
	assert(timestamp_query_pool);

	gpu_resource_manager = std::make_unique<GPUResourceManager>(device,
																vma_allocator,
																device_resource,
																resource_deletion_queue,
																descriptor_set_updates,
																debug_utils_extension_present);
	CreateSyncMarkers();
	assert(render_complete_semaphore[0]);
	assert(image_acquired_semaphore[0]);
	assert(command_buffer_fence[0]);
	g_vulkan_cmd_buffer_ring.Init(this);

	SamplerCreation sc{};
	sc.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sc.min_filter = VK_FILTER_LINEAR;
	sc.mag_filter = VK_FILTER_LINEAR;
	sc.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sc.name = "Sampler Default";
	default_sampler = gpu_resource_manager->CreateSampler(sc);

	device_resource.dynamic_per_frame_size = 1024 * 1024 * 10;
	BufferCreation buffer_create_info;
	buffer_create_info.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
									 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
									 VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	buffer_create_info.usage_type = ResourceUsageType::Immutable;
	buffer_create_info.size = device_resource.dynamic_per_frame_size * MaxSwapchainImages;
	buffer_create_info.name = "Dynamic_Persistent_Buffer";
	device_resource.dynamic_buffer = gpu_resource_manager->CreateBuffer(buffer_create_info);
	render::MapBufferParameter map_buffer_param{device_resource.dynamic_buffer, 0, 0};
	device_resource.dynamic_mapped_memory =
		(uint8_t *)gpu_resource_manager->MapBuffer(map_buffer_param);
}

void DeviceBase::Shutdown()
{

	g_vulkan_cmd_buffer_ring.Destroy(this);
	render::MapBufferParameter map_buffer_param{device_resource.dynamic_buffer, 0, 0};
	gpu_resource_manager->UnMapBuffer(map_buffer_param);
	gpu_resource_manager->DestroyBuffer(device_resource.dynamic_buffer, current_frame);
	gpu_resource_manager->DestroySampler(default_sampler, current_frame);
	gpu_resource_manager->ReleaseResourcesInDeletionQueue();

	device_resource.samplers.Shutdown();
	device_resource.pipelines.Shutdown();
	device_resource.shaders.Shutdown();
	device_resource.descriptor_sets.Shutdown();

	DestroySyncMarkers();
	vkDestroyQueryPool(device, timestamp_query_pool, nullptr);
	vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	vmaDestroyAllocator(vma_allocator);
	DestroySwapChain();
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, window_surface, nullptr);

#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	auto vkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger, nullptr);
#endif
	vkDestroyInstance(instance, nullptr);
}

} // namespace cloud::vulkan