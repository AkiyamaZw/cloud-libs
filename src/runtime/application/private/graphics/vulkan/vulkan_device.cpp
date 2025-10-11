#include "graphics/vulakn/vulkan_device.h"
#include "runtime_log.h"
#include <array>
// clang-format off
#ifdef WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
	#define GLFW_INCLUDE_VULKAN
	#include "Windows.h"
	#include <vulkan/vulkan_win32.h>
	#include "GLFW/glfw3.h"
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include "GLFW/glfw3native.h"
#elif defined(__APPLE__)
	#define GLFW_INCLUDE_VULKAN
	#include "GLFW/glfw3.h"
#endif
// clang-format on


#define ArraySize(array) (sizeof(array) / sizeof(array)[0])
#define check_vk(succ)                                                                             \
	if (succ != VK_SUCCESS)                                                                        \
		printf("%d", succ);
#define check_true(succ) assert(succ)

namespace cloud::vulkan
{

struct GpuDevice
{
	/* basic api object */
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	float gpu_timestamp_frequency;
	VkDevice device;
	VkQueue queue;
	uint32_t queue_family;
	VkDescriptorPool descriptor_pool;

	/* extension debug */
	bool debug_utils_extension_present{false};
	VkDebugReportCallbackEXT debug_callback;
	VkDebugUtilsMessengerEXT debug_utils_messenger;

	/* window */
	VkSurfaceKHR window_surface;
	VkSurfaceFormatKHR window_surface_format;
	VkPresentModeKHR present_mode;

	/* swapchain */
	VkSwapchainKHR swapchain;
	std::array<VkImage, MaxSwapchainImages> swapchain_images;
	std::array<VkImage, MaxSwapchainImages> swapchain_image_views;
	std::array<VkFramebuffer, MaxSwapchainImages> swapchain_freamebuffers;
	uint32_t swapchain_width;
	uint32_t swapchain_height;

	/* sync */
	std::array<VkSemaphore, MaxSwapchainImages> render_complete_semaphore;
	std::array<VkSemaphore, MaxSwapchainImages> image_acquired_semaphore;
	std::array<VkFence, MaxSwapchainImages> command_buffer_fence;

} g_vulkan_device;

static const char *s_instance_layer[] = {
#ifdef DEBUG
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

#if defined(DEBUG)
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
#ifdef DEBUG
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

bool get_family_queue( VkPhysicalDevice physical_device, VkSurfaceKHR window_surface, uint32_t& queue_family_index) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr );

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    uint32_t family_index = 0;
    VkBool32 surface_supported;
    for ( ; family_index < queue_family_count; ++family_index ) {
        VkQueueFamilyProperties queue_family = queue_families[ family_index ];
        if ( queue_family.queueCount > 0 && queue_family.queueFlags & ( VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT ) ) {
            vkGetPhysicalDeviceSurfaceSupportKHR( physical_device, family_index, window_surface, &surface_supported);

            if ( surface_supported ) {
                queue_family_index = family_index;
                break;
            }
        }
    }
    return surface_supported;
}

void CreateInstance(GpuCreateParam &param)
{
	VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
								  .apiVersion = VK_MAKE_VERSION(1, 0, 0)};
	VkInstanceCreateInfo ins_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
									 .flags = 0,
									 .pApplicationInfo = &app_info,
#ifdef DEBUG
									 .enabledLayerCount = ArraySize(s_instance_layer),
									 .ppEnabledLayerNames = s_instance_layer,
									 .enabledExtensionCount = ArraySize(s_requested_extensions),
									 .ppEnabledExtensionNames = s_requested_extensions
#endif
	};

#ifdef DEBUG
	const VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
		create_debug_utils_messenger_info();
	ins_info.pNext = &debug_create_info;
#endif
	auto succ = vkCreateInstance(&ins_info, nullptr, &g_vulkan_device.instance);
	check_vk(succ);
	INFO("[Vulkan Gpu Device] Instance Created..");
}

void InitGpuDevice(GpuCreateParam &param)
{
	INFO("[Vulkan Gpu Device] Start init...");
	VkResult succ;

	std::vector<std::string_view> window_extension;
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
	// instance
	CreateInstance(param);
	// messenger
	CreateDebugExt();

	// surface creation
	g_vulkan_device.swapchain_width = param.width;
	g_vulkan_device.swapchain_height = param.height;
	succ = glfwCreateWindowSurface(g_vulkan_device.instance, (GLFWwindow*)param.window, nullptr, &g_vulkan_device.window_surface);
	check_vk(succ);

	uint32_t num_physical_device;
	succ = vkEnumeratePhysicalDevices(g_vulkan_device.instance, &num_physical_device, NULL);
	check_vk(succ);

	std::vector<VkPhysicalDevice> gpus(num_physical_device);
	succ = vkEnumeratePhysicalDevices(g_vulkan_device.instance, &num_physical_device, gpus.data());
	check_vk(succ);
	VkPhysicalDeviceProperties device_property;
	VkPhysicalDevice discrate_device;
	VkPhysicalDevice intergrate_device;

	for(uint32_t index = 0; index < num_physical_device; ++index)
	{
		vkGetPhysicalDeviceProperties(gpus[index], &g_vulkan_device.physical_device_properties);
		VkPhysicalDeviceType device_type = g_vulkan_device.physical_device_properties.deviceType;
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			if (get_family_queue(gpus[index], g_vulkan_device.window_surface, g_vulkan_device.queue_family))
			{
				discrate_device = gpus[index];
				break;			
			}
			continue;
		}
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			if (get_family_queue(gpus[index], g_vulkan_device.window_surface, g_vulkan_device.queue_family))
			{
				intergrate_device = gpus[index];
				break;
			}
			continue;;
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
	g_vulkan_device.gpu_timestamp_frequency = g_vulkan_device.physical_device_properties.limits.timestampPeriod / (1000 * 1000);
	INFO("[vulkan device] select gpu {}, gpu_timestamp_frequency:{:.8f}", g_vulkan_device.physical_device_properties.deviceName, g_vulkan_device.gpu_timestamp_frequency);

	/* device */
}

void ShutdownGpuDevice()
{

	vkDestroySurfaceKHR(g_vulkan_device.instance, g_vulkan_device.window_surface, nullptr);
	
#ifdef DEBUG
	auto vkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			g_vulkan_device.instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(
		g_vulkan_device.instance, g_vulkan_device.debug_utils_messenger, nullptr);
#endif
	vkDestroyInstance(g_vulkan_device.instance, nullptr);
}
} // namespace cloud::vulkan