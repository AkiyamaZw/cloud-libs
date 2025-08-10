#include "VkGraphicsBase.h"
#include "VkUtility.h"
#include "app_utility.h"
#include <span>

namespace cloud::graphics::vk
{

VkResult CheckInstanceLayers(VkInstance instance, std::span<const char *> layer_check)
{
    uint32_t layer_count;
    std::vector<VkLayerProperties> avaliable_layers;
    if (VkResult succ = vkEnumerateInstanceLayerProperties(&layer_count, nullptr))
    {
        INFO("enumerateInstanceLayer failed: error_code {}\n", int32_t(succ));
        return succ;
    }
    if (layer_count)
    {
        avaliable_layers.resize(layer_count);
        if (VkResult succ =
                vkEnumerateInstanceLayerProperties(&layer_count, avaliable_layers.data()))
        {
            INFO("enumerateInstanceLayer failed: error_code {}\n", int32_t(succ));
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
        INFO("enumerateInstanceExtension failed: error_code {}, layer_name{}\n",
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
            INFO("enumerateInstanceLayer failed: error_code {}\n", int32_t(succ));
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

VkResult GetPhysicalDevices(VkInstance instance, std::vector<VkPhysicalDevice> &physical_devices)
{
    uint32_t device_cnt{0};
    if (VkResult succ = vkEnumeratePhysicalDevices(instance, &device_cnt, nullptr))
    {
        INFO("failed to get physical device count/n");
        return succ;
    }
    if (!device_cnt)
    {
        INFO("get physical device count equals to 0\n");
        return VkResult::VK_ERROR_DEVICE_LOST;
    }
    physical_devices.resize(device_cnt);
    VkResult succ = vkEnumeratePhysicalDevices(instance, &device_cnt, physical_devices.data());
    if (succ)
        INFO("get available device failed\n");
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
    ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
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
                INFO("failed to determine if the queue family support present");
                return VK_RESULT_MAX_ENUM;
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
    return VK_SUCCESS;
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

GraphicsBase::GraphicsBase() { device_data_ = std::make_unique<VkDeviceData>(); }

GraphicsBase::~GraphicsBase() {}

void GraphicsBase::RegInstanceLayer(const char *ins_layer_name)
{
    InsertToVector(ins_layer_name, device_data_->instance_layer);
}

void GraphicsBase::RegInstanceExt(const char *ins_ext_name)
{
    InsertToVector(ins_ext_name, device_data_->instance_extentions);
}

void GraphicsBase::RegInstanceLayers(const std::vector<std::string_view> &ins_layers)
{
    for (const auto &layer : ins_layers)
    {
        RegInstanceLayer(layer.data());
    }
}

void GraphicsBase::RegInstanceExts(const std::vector<std::string_view> &ins_exts)
{
    for (const auto &ext : ins_exts)
    {
        RegInstanceExt(ext.data());
    }
}

void GraphicsBase::SetSurface(VkSurfaceKHR surface)
{
    if (!surface_)
        surface_ = surface;
}

bool SetupGraphics(GraphicsBase *graphics)
{
    assert(graphics != nullptr);

    // 1.0 create device instance
    if (CreateVkInstance(graphics->device_data_.get()))
    {
        return false;
    }

    // 2.0 create window surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    // 3.0 get physical device

    return true;
}

bool DestoryGraphics(GraphicsBase *graphics)
{
    if (graphics->device_data_ == nullptr)
        return false;

    bool succ = DestoryInstance(graphics->device_data_.get());
    return succ;
}
} // namespace cloud::graphics::vk