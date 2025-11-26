#pragma once
#include <cstdint>
#include "graphics/core/gpu_resource.h"

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
struct GpuDevice
{
	virtual ~GpuDevice() = default;
	virtual void InitGpuDevice(GpuCreateParam& param) = 0;
    virtual void ShutdownGpuDevice() = 0;
    virtual SamplerHandle CreateSampler(const SamplerCreation& creation) = 0;
};

GpuDevice* CreateGpuDevice(const TypeDevice &type);
} // namespace cloud::render