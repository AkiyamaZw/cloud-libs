#pragma once
#include <cstdint>
#include <array>
#include "vulkan.h"

namespace cloud::vulkan
{
static constexpr uint32_t MaxSwapchainImages = 3;

struct GpuCreateParam
{
    void *window{nullptr};
    int width;
    int height;
    uint16_t gpu_time_queries_per_frame{32};
    bool enable_gpu_time_queries{false};
    bool enable_debug{false};
    uint32_t max_swapchain_images{3};
    uint32_t max_frames{2};
};

struct GpuDevice
{
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkDevice device;
    VkQueue queue;
    uint32_t queue_family;
    VkDescriptorPool descriptor_pool;

    /* swapchain */
    std::array<VkImage, MaxSwapchainImages> swapchain_images;
    std::array<VkImage, MaxSwapchainImages> swapchain_image_views;
    std::array<VkFramebuffer, MaxSwapchainImages> swapchain_freamebuffers[MaxSwapchainImages];

    /* sync */
    std::array<VkSemaphore, MaxSwapchainImages> render_complete_semaphore;
    std::array<VkSemaphore, MaxSwapchainImages> image_acquired_semaphore;
    std::array<VkFence, MaxSwapchainImages> command_buffer_fence;
};

void InitGpuDevice(GpuCreateParam &param, GpuDevice &gpu_device);
} // namespace cloud::vulkan