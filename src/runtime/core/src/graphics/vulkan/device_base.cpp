#include "graphics/vulkan/device_base.h"
#include "graphics/vulkan/vulkan_interface.h"
#include "core/runtime_log.h"
#include <algorithm>
#include <fstream>
#include <string>

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
		INFO("[Vulkan Device] Extension %s for debugging non presenting",
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
	INFO("api version: %d.%d.%d",
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

	INFO("[vulkan device] select gpu %s, gpu_timestamp_frequency:%f",
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
	INFO("Create swapchain %d, %d - Saved %d %d, min image %d\n",
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

void DeviceBase::CreateRenderPass()
{
	VkAttachmentDescription color_attachment{};
	color_attachment.format = window_surface_format.format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass);
	check_vk(result);
	INFO("[Vulkan Gpu Device] Render Pass Created..");
}

void DeviceBase::CreateFramebuffers()
{
	for (size_t i = 0; i < swapchain_image_count; i++)
	{
		VkImageView attachments[] = {swapchain_image_views[i]};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = swapchain_width;
		framebuffer_info.height = swapchain_height;
		framebuffer_info.layers = 1;

		VkResult result =
			vkCreateFramebuffer(device, &framebuffer_info, nullptr, &swapchain_framebuffers[i]);
		check_vk(result);
	}
	INFO("[Vulkan Gpu Device] Framebuffers Created..");
}

void DeviceBase::DestroySwapChain()
{
	for (size_t i = 0; i < swapchain_image_count; i++)
	{
		vkDestroyImageView(device, swapchain_image_views[i], nullptr);
		vkDestroyFramebuffer(device, swapchain_framebuffers[i], nullptr);
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

void DeviceBase::CreatePipelineLayout()
{
	// 创建pipeline layout（不需要descriptor set layout）
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = nullptr;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	VkResult result =
		vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout);
	check_vk(result);
	INFO("[Vulkan Gpu Device] Pipeline Layout Created..");
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

	CreateQueryPool(param);
	assert(timestamp_query_pool);

	CreateRenderPass();
	assert(render_pass);

	CreateFramebuffers();
	assert(swapchain_framebuffers[0]);

	CreatePipelineLayout();
	assert(pipeline_layout);

	CreateGraphicsPipeline();
	assert(graphics_pipeline);

	gpu_resource_manager = std::make_unique<GPUResourceManager>(device,
																vma_allocator,
																descriptor_pool,
																current_frame,
																device_resource,
																default_resource,
																resource_deletion_queue,
																descriptor_set_updates,
																debug_utils_extension_present);

	// 初始化时间
	start_time = std::chrono::high_resolution_clock::now();
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
	default_resource.default_sampler = gpu_resource_manager->CreateSampler(sc);
}

VkShaderModule DeviceBase::CreateShaderModule(const std::vector<char> &code)
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
	check_vk(result);

	return shader_module;
}

std::vector<char> DeviceBase::LoadShader(const std::string &filename)
{
	auto path = std::string(RESOURCE_ROOT_DIR) + "/" + filename; // 资源根路径 + shaders目录

	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (file.is_open())
	{
		INFO("Loading shader from: %s", path.c_str());
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		if (buffer.size() > 0)
		{
			return buffer;
		}
	}

	FATAL("Failed to open shader file: %s", path.c_str());
	return {};
}

void DeviceBase::AdvanceFrame() {
	previous_frame = current_frame;
	current_frame = (current_frame + 1) % swapchain_image_count;
	++absolute_frame;
}

void DeviceBase::ResizeSwapChain() {

}

void DeviceBase::CreateGraphicsPipeline()
{
	// 加载着色器代码
	auto vert_code = LoadShader("shaders/vert.spv");
	auto frag_code = LoadShader("shaders/frag.spv");

	VkShaderModule vert_module = CreateShaderModule(vert_code);
	VkShaderModule frag_module = CreateShaderModule(frag_code);

	VkPipelineShaderStageCreateInfo vert_stage_info{};
	vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_info.module = vert_module;
	vert_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_stage_info{};
	frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage_info.module = frag_module;
	frag_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

	// 顶点输入
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// 输入装配
	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	// 视口和裁剪
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain_width;
	viewport.height = (float)swapchain_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {swapchain_width, swapchain_height};

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	// 光栅化
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// 多重采样
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// 深度和模板测试
	VkPipelineDepthStencilStateCreateInfo depth_stencil{};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_FALSE;
	depth_stencil.depthWriteEnable = VK_FALSE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;

	// 颜色混合
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
											VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	// 图形管线
	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;

	VkResult result = vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
	check_vk(result);

	vkDestroyShaderModule(device, frag_module, nullptr);
	vkDestroyShaderModule(device, vert_module, nullptr);

	INFO("[Vulkan Gpu Device] Graphics Pipeline Created..");
}

void DeviceBase::Commit()
{
	// submit command
	VkFence *render_complete_fence = &command_buffer_fence[current_frame];
	VkSemaphore *render_complemte_semaphore = &render_complete_semaphore[current_frame];

	VkCommandBuffer enqueued_command_buffers[4];
	for (uint32_t i = 0; i < num_queued_command_buffers; i++)
	{
		CommandBuffer *cb = queued_command_buffers[i];
		enqueued_command_buffers[i] = cb->vk_command_buffer;
		if (cb->is_recoding && cb->current_render_pass &&
			(cb->current_render_pass->type != RenderPassType::Compute))
		{
			vkCmdEndRenderPass(cb->vk_command_buffer);
		}
		vkEndCommandBuffer(cb->vk_command_buffer);
	}
	VkSemaphore wait_semaphores[] = {image_acquired_semaphore[current_frame]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = num_queued_command_buffers;
	submit_info.pCommandBuffers = enqueued_command_buffers;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = render_complemte_semaphore;

	vkQueueSubmit(queue, 1, &submit_info, *render_complete_fence);

	// bake to render buffer
	VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = render_complemte_semaphore;
	VkSwapchainKHR swap_chains[] = {swapchain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &vulkan_image_index;
	present_info.pResults = nullptr;
	VkResult succ = vkQueuePresentKHR(queue, &present_info);
	check_vk(succ);

	num_queued_command_buffers = 0;
	if (succ == VK_ERROR_OUT_OF_DATE_KHR || succ == VK_SUBOPTIMAL_KHR || resize)
	{
		resize = false;
		RezieSwapChain();

		AdvanceFrame();
		return;
	}

	AdvanceFrame();
	if (resource_deletion_queue.size() > 0)
	{
		gpu_resource_manager.
	}
	gpu_resource_manager.ReleaseResourcesInDeletionQueue();
}

void DeviceBase::Shutdown()
{
	// 等待所有命令缓冲区完成执行
	for (uint32_t i = 0; i < MaxSwapchainImages; ++i)
	{
		vkWaitForFences(device, 1, &command_buffer_fence[i], VK_TRUE, UINT64_MAX);
	}

	// 等待所有队列操作完成
	vkDeviceWaitIdle(device);

	g_vulkan_cmd_buffer_ring.Destroy(this);
	render::MapBufferParameter map_buffer_param{device_resource.dynamic_buffer, 0, 0};
	gpu_resource_manager->UnMapBuffer(map_buffer_param);

	// 销毁uniform buffer
	gpu_resource_manager->DestroyBuffer(uniform_buffer, current_frame);

	// 销毁dynamic buffer和sampler
	gpu_resource_manager->DestroyBuffer(device_resource.dynamic_buffer, current_frame);
	gpu_resource_manager->DestroySampler(default_resource.default_sampler, current_frame);

	// 释放deletion queue中的资源
	gpu_resource_manager->ReleaseResourcesInDeletionQueue();

	device_resource.samplers.Shutdown();
	device_resource.pipelines.Shutdown();
	device_resource.shaders.Shutdown();
	device_resource.descriptor_sets.Shutdown();

	// 销毁渲染相关资源
	vkDestroyPipeline(device, graphics_pipeline, nullptr);

	// 销毁descriptor set
	vkFreeDescriptorSets(device, descriptor_pool, 1, &descriptor_set);

	// 销毁descriptor set layout
	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);

	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
	vkDestroyRenderPass(device, render_pass, nullptr);

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

void DeviceBase::StartFrame() {
	VkFence* render_complete_fence = &command_buffer_fence[current_frame];
	if (vkGetFenceStatus(device, *render_complete_fence) != VK_SUCCESS)
	{
		vkWaitForFences(device, 1, render_complete_fence, VK_TRUE, UINT64_MAX);
	}
	vkResetFences(device, 1, render_complete_fence);
	VkResult succ = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_acquired_semaphore[current_frame], VK_NULL_HANDLE, &vulkan_image_index);
	if (succ == VK_ERROR_OUT_OF_DATE_KHR){
		ResizeSwapChain();
	}
	g_vulkan_cmd_buffer_ring.Reset();
	gpu_resource_manager.UpdateDynamicBuffer();
	gpu_resource_manager.UpdateDescriptorSet();

}

} // namespace cloud::vulkan

namespace cloud::vulkan
{
bool InitializeContextInstance(VulaknDeviceContext::InstanceData &instance_data,
							   const GpuCreateParam &param)
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

bool DestroyContextInstance(VulaknDeviceContext::InstanceData &instance_data)
{
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
	auto vkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance_data.instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(
		instance_data.instance, instance_data.debug_utils_messenger, nullptr);
#endif
	vkDestroyInstance(instance_data.instance, nullptr);
	return true;
}
} // namespace cloud::vulkan