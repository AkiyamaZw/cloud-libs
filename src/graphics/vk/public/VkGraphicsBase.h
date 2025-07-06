#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace cloud::graphics::vk
{
class GraphicsBase
{
  public:
    GraphicsBase();
    virtual ~GraphicsBase();
    GraphicsBase(const GraphicsBase &) = delete;
    GraphicsBase(GraphicsBase &&) = delete;

    void RegInstanceLayer(const char *ins_layer_name);
    void RegInstanceExt(const char *ins_ext_name);

    void SetSurface(VkSurfaceKHR surface);

  private:
    friend VkResult CreateVkInstance(VkInstanceCreateFlags flags, GraphicsBase *graphics_base);

    uint32_t api_version_{VK_API_VERSION_1_0};
    VkInstance instance_;
    std::vector<const char *> instance_layer_;
    std::vector<const char *> instance_extentions_;
    VkDebugUtilsMessengerEXT debug_messager_;
    VkSurfaceKHR surface_;

    // physical device is just like a remote server, it provides base information.
    VkPhysicalDevice physical_device_;
    VkPhysicalDeviceProperties physical_device_properties_;
    VkPhysicalDeviceMemoryProperties physical_device_memroy_properties_;

    // a phisical device' proxy just like a proxy of a remote server.
    VkDevice vk_device_;
    uint32_t grapgics_queue_family_index_{VK_QUEUE_FAMILY_IGNORED};
    uint32_t present_queue_family_index_{VK_QUEUE_FAMILY_IGNORED};
    uint32_t compute_queue_family_index_{VK_QUEUE_FAMILY_IGNORED};
    VkQueue graphics_queue_;
    VkQueue present_queue_;
    VkQueue compute_queue_;
    std::vector<const char *> device_extensions_;

    // swapchain
    std::vector<VkSurfaceFormatKHR> available_surface_format_;
    VkSwapchainKHR swapchain_;
    std::vector<VkImage> swapchain_images_;
    std::vector<VkImageView> swapchain_views_;
    VkSwapchainCreateInfoKHR swapchain_create_info_{};
};

void SetupGraphics(GraphicsBase *graphics);
} // namespace cloud::graphics::vk
