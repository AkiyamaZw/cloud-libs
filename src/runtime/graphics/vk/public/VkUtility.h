#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace cloud::graphics::vk
{
#define VK_SUCC(succ) succ == VkResult::VK_SUCCESS
#define VK_FAIL(succ) succ != VkResult::VK_SUCCESS

struct VkDeviceData
{
    uint32_t api_version{VK_API_VERSION_1_0};
    VkInstance instance;
    std::vector<const char *> instance_layer;
    std::vector<const char *> instance_extentions;
    VkDebugUtilsMessengerEXT debug_messager;
};

VkResult CreateVkInstance(VkDeviceData *device, VkInstanceCreateFlags flags = 0);
bool DestoryInstance(VkDeviceData *device);
} // namespace cloud::graphics::vk