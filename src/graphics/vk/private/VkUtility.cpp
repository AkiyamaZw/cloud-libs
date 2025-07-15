#include "VkUtility.h"
#include <iostream>
#include <format>

namespace cloud::graphics::vk
{
VkResult CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);

VkResult CreateVkInstance(VkDeviceData *device, VkInstanceCreateFlags flags)
{
#ifndef NDEBUG
    device->instance_layer.push_back("VK_LAYER_KHRONOS_Validation");
    device->instance_extentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                  .apiVersion = device->api_version};
    VkInstanceCreateInfo ins_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                     .flags = flags,
                                     .pApplicationInfo = &app_info,
                                     .enabledLayerCount = uint32_t(device->instance_layer.size()),
                                     .ppEnabledLayerNames = device->instance_layer.data(),
                                     .enabledExtensionCount =
                                         uint32_t(device->instance_extentions.size()),
                                     .ppEnabledExtensionNames = device->instance_extentions.data()};

    if (VkResult succ = vkCreateInstance(&ins_info, nullptr, &device->instance))
    {
        std::cerr << "ceate vk instance failed" << std::endl;
        return succ;
    }
    std::cout << std::format("vulkan {}.{}.{} standing by.",
                             VK_VERSION_MAJOR(device->api_version),
                             VK_VERSION_MINOR(device->api_version),
                             VK_VERSION_PATCH(device->api_version))
              << std::endl;
#ifndef NDEBUG
    CreateDebugMessenger(device->instance, device->debug_messager);
#endif
    return VK_SUCCESS;
}

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
        std::cout << std::format("failed to create debug messager due to get function pointer of"
                                 "vkCreateDebugUtilsMessengerEXT\n");
        return VK_RESULT_MAX_ENUM;
    }

    VkResult succ = vk_create_debug_utils_messager(
        instance, &debug_utils_mesg_create_info, nullptr, &messenger);
    if (succ)
    {
        std::cout << std::format("failed to create a debug messenger: error code :{}\n",
                                 int32_t(succ));
    }
    return VkResult::VK_ERROR_DEVICE_LOST;
}
} // namespace cloud::graphics::vk