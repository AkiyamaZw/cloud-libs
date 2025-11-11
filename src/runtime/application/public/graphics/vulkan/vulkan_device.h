#pragma once
#include "graphics/device.h"
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct _GpuDevice;
class GpuDevice
{
public:
    static GpuDevice* Inst();
    ~GpuDevice();
    void InitGpuDevice(GpuCreateParam &param);
    void ShutdownGpuDevice();
    // SamplerHandle CreateSampler(const SamplerCreation& creation);
private:
    _GpuDevice* impl_{nullptr};
};
} // namespace cloud::vulkan