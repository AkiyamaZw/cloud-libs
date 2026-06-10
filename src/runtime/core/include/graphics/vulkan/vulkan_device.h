#pragma once
#include "graphics/device.h"
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/vulkan_device_context.h"

namespace cloud::vulkan
{

/* this class implement GpuDevice interface */
class GpuDevice : public render::GpuDevice
{
  public:
	~GpuDevice() override;
	TypeDevice GetTypeDevice() const override { return TypeDevice::Vulkan; };
	void InitGpuDevice(GpuCreateParam &param) override;
	void ShutdownGpuDevice() override;

	ResourceHandle CreateSampler(const render::SamplerCreation &creation) override;
	void DestroySampler(const ResourceHandle &handle) override;
	void present() override;

  private:
	VulkanDeviceContext device_context;
};
} // namespace cloud::vulkan