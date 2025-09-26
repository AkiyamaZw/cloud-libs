#pragma once
#include <cstdint>

namespace cloud
{
enum class TypeDevice
{
    Vulkan
};

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
    TypeDevice type_device{TypeDevice::Vulkan};
};
}