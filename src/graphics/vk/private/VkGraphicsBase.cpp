#include "VkGraphicsBase.h"
#include <iostream>
#include <format>
#include <span>

namespace graphics::vk
{

VkResult CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    static PFN_vkDebugUtilsMessengerCallbackEXT debug_utils_callback =
        [](VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
           VkDebugUtilsMessageTypeFlagsEXT msg_types,
           const VkDebugUtilsMessengerCallbackDataEXT *cb_data,
           void *user_data) {
            std::cout << std::format("{}\n\n", cb_data->pMessage);
            return VK_FALSE;
        };
    VkDebugUtilsMessengerCreateInfoEXT debug_utils_mesg_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_utils_callback};

    PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messager =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (vk_create_debug_utils_messager == nullptr)
    {
        std::cout << std::format("failed to create debug messager due to get function pointer of "
                                 "vkCreateDebugUtilsMessengerEXT\n");
        return VK_RESULT_MAX_ENUM;
    }

    VkResult succ = vkCreateDebugUtilsMessengerEXT(
        instance, &debug_utils_mesg_create_info, nullptr, &messenger);
    if (succ)
    {
        std::cout << std::format("failed to create a debug messenger: error code :{}\n",
                                 int32_t(succ));
    }
    return succ;
}

VkResult CheckInstanceLayers(VkInstance instance, std::span<const char *> layer_check)
{
    uint32_t layer_count;
    std::vector<VkLayerProperties> avaliable_layers;
    if (VkResult succ = vkEnumerateInstanceLayerProperties(&layer_count, nullptr))
    {
        std::cout << std::format("enumerateInstanceLayer failed: error_code {}\n", int32_t(succ));
        return succ;
    }
    if (layer_count)
    {
        avaliable_layers.resize(layer_count);
        if (VkResult succ =
                vkEnumerateInstanceLayerProperties(&layer_count, avaliable_layers.data()))
        {
            std::cout << std::format("enumerateInstanceLayer failed: error_code {}\n",
                                     int32_t(succ));
            return succ;
        }
        for (auto &prop : layer_check)
        {
            bool found = false;
            for (auto &layer : avaliable_layers)
            {
                if (!strcmp(prop, layer.layerName))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                prop = nullptr;
            }
        }
    }
    else
    {
        for (auto &layer : layer_check)
        {
            layer = nullptr;
        }
    }
    return VK_SUCCESS;
}

VkResult CheckInstanceExtension(VkInstance instance,
                                std::span<const char *> ext_check,
                                const char *layer_name)
{
    uint32_t ext_count;
    std::vector<VkExtensionProperties> avaliable_exts;
    if (VkResult succ = vkEnumerateInstanceExtensionProperties(layer_name, &ext_count, nullptr))
    {
        std::cout << std::format("enumerateInstanceExtension failed: error_code {}, layer_name{}\n",
                                 int32_t(succ),
                                 layer_name);
        return succ;
    }
    if (ext_count)
    {
        avaliable_exts.resize(ext_count);
        if (VkResult succ = vkEnumerateInstanceExtensionProperties(
                layer_name, &ext_count, avaliable_exts.data()))
        {
            std::cout << std::format("enumerateInstanceLayer failed: error_code {}\n",
                                     int32_t(succ));
            return succ;
        }
        for (auto &prop : ext_check)
        {
            bool found = false;
            for (auto &layer : avaliable_exts)
            {
                if (!strcmp(prop, layer.extensionName))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                prop = nullptr;
            }
        }
    }
    else
    {
        for (auto &layer : ext_check)
        {
            layer = nullptr;
        }
    }
    return VK_SUCCESS;
}

VkResult CreateVkInstance(VkInstanceCreateFlags flags = 0, GraphicsBase *graphics_base)
{
#ifndef NDEBUG
    graphics_base->RegInstanceLayer("VK_LAYER_KHRONOS_Validation");
    graphics_base->RegInstanceExt(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                  .apiVersion = graphics_base->api_version_};
    VkInstanceCreateInfo ins_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = flags,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = uint32_t(graphics_base->instance_layer_.size()),
        .ppEnabledLayerNames = graphics_base->instance_layer_.data(),
        .enabledExtensionCount = uint32_t(graphics_base->instance_extentions_.size()),
        .ppEnabledExtensionNames = graphics_base->instance_extentions_.data()};

    if (VkResult succ = vkCreateInstance(&ins_info, nullptr, &graphics_base->instance_))
    {
        std::cerr << "ceate vk instance failed" << std::endl;
        return succ;
    }
    std::cout << std::format("vulkan {}.{}.{} standing by.",
                             VK_VERSION_MAJOR(graphics_base->api_version_),
                             VK_VERSION_MINOR(graphics_base->api_version_),
                             VK_VERSION_PATCH(graphics_base->api_version_));
#ifndef NDEBUG
    CreateDebugMessenger(graphics_base->instance_, graphics_base->debug_messager_);
#endif
    return VK_SUCCESS;
}

VkResult GetPhysicalDevices(VkInstance instance, std::vector<VkPhysicalDevice> &physical_devices)
{
    uint32_t device_cnt{0};
    if (VkResult succ = vkEnumeratePhysicalDevices(instance, &device_cnt, nullptr))
    {
        std::cout << std::format("failed to get physical device count/n");
        return succ;
    }
    if (!device_cnt)
    {
        std::cout << std::format("get physical device count equals to 0\n");
        VkResult::VK_ERROR_DEVICE_LOST;
    }
    physical_devices.resize(device_cnt);
    VkResult succ = vkEnumeratePhysicalDevices(instance, &device_cnt, physical_devices.data());
    if (succ)
        std::cout << std::format("get available device failed\n");
    return succ;
}

VkResult GetQueueFamilyIndices(VkPhysicalDevice physical_device,
                               bool enable_graphics_queue,
                               bool enable_compute_queue,
                               VkSurfaceKHR surface,
                               uint32_t (&queue_family_indices)[3])
{
    uint32_t queue_family_count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    if (!queue_family_count)
    {
        return VK_RESULT_MAX_ENUM;
    }
    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_family_properties.data());
    auto &[ig, ip, ic] = queue_family_indices;
    ig, ip, ic = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        VkBool32 support_graphics =
            enable_graphics_queue && queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        VkBool32 support_present = false;
        VkBool32 support_compute =
            enable_compute_queue && queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
        if (surface)
        {
            if (VkResult succ = vkGetPhysicalDeviceSurfaceSupportKHR(
                    physical_device, i, surface, &support_present))
            {
                std::cout << "failed to determine if the queue family support present\n";
                return;
            }
        }
        if (support_graphics && support_compute)
        {
            if (support_present)
            {
                ig = ip = ic = i;
                break;
            }
            if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED)
            {
                ig = ic = i;
            }
            if (!surface)
            {
                break;
            }
        }
        if (support_graphics && ig == VK_QUEUE_FAMILY_IGNORED)
        {
            ig = i;
        }
        if (support_compute && ic == VK_QUEUE_FAMILY_IGNORED)
        {
            ic = i;
        }
        if (support_present && ip == VK_QUEUE_FAMILY_IGNORED)
        {
            ip = i;
        }
    }
    if (ig == VK_QUEUE_FAMILY_IGNORED && enable_graphics_queue ||
        ip == VK_QUEUE_FAMILY_IGNORED && surface ||
        ic == VK_QUEUE_FAMILY_IGNORED && enable_compute_queue)
    {
        return VK_RESULT_MAX_ENUM;
    }
    return;
}

void InsertToVector(const char *name, std::vector<const char *> &vec)
{
    for (auto &ins_name : vec)
    {
        if (!strcmp(name, ins_name))
        {
            return;
        }
    }
    vec.push_back(name);
}

GraphicsBase::GraphicsBase() {}

GraphicsBase::~GraphicsBase() {}

void GraphicsBase::RegInstanceLayer(const char *ins_layer_name)
{
    InsertToVector(ins_layer_name, instance_layer_);
}

void GraphicsBase::RegInstanceExt(const char *ins_ext_name)
{
    InsertToVector(ins_ext_name, instance_extentions_);
}

void GraphicsBase::SetSurface(VkSurfaceKHR surface)
{
    if (!surface_)
        surface_ = surface;
}

} // namespace graphics::vk