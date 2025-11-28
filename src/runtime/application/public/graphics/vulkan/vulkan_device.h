#pragma once
#include "graphics/device.h"
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/device_base.h"

namespace cloud::vulkan
{
struct DeviceBase;

class GpuDevice
	: public render::GpuDevice
	, public DeviceBase
{
  public:
	~GpuDevice() override;
    TypeDevice GetTypeDevice() const override {return TypeDevice::Vulkan;};
	void InitGpuDevice(GpuCreateParam &param) override;
	void ShutdownGpuDevice() override;

	SamplerHandle CreateSampler(const render::SamplerCreation &creation) override;
};
} // namespace cloud::vulkan