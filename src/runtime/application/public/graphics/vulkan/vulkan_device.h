#pragma once
#include "graphics/device.h"
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct _GpuDevice;
class GpuDevice: public render::GpuDevice
{
public:
    static GpuDevice* Inst();
    ~GpuDevice() override;
    void InitGpuDevice(GpuCreateParam &param) override;
    void ShutdownGpuDevice() override;
    SamplerHandle CreateSampler(const render::SamplerCreation& creation) override;
private:
    _GpuDevice* impl_{nullptr};
};
} // namespace cloud::vulkan