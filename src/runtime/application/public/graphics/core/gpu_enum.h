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

namespace cloud::render
{
using ResourceHandle = uint32_t;

#define HANDLE_DECLARE(name) \
struct name##Handle{ ResourceHandle index; }

HANDLE_DECLARE(Buffer);
HANDLE_DECLARE(Texture);
HANDLE_DECLARE(DescriptorSetLayout);
HANDLE_DECLARE(ShaderState);
HANDLE_DECLARE(Pipeline);
HANDLE_DECLARE(Sampler);

enum class FilterMode: uint32_t
{
    Point,
    Linear,
};

enum class AddressMode : uint32_t
{
    Wrap,
    Mirror,
    Clamp,
    Border,
    MirrorOnce,
};

enum class ReductionMode : uint32_t
{
    Filter,
    Comparison,
    Minimum,
    Maximum,
};
}