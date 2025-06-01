#include "VkGraphicsBase.h"
#include <iostream>

namespace graphics::vk
{

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
    CreateDebugMessenger();
#endif

    return VK_SUCCESS;

} // namespace graphics::vk

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

} // namespace graphics::vk