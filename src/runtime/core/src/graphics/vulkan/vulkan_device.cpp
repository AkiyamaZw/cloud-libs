#include "graphics/vulkan/vulkan_device.h"
#include "core/runtime_log.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/device_base.h"

namespace cloud::vulkan
{

GpuDevice::~GpuDevice() {}

void GpuDevice::InitGpuDevice(GpuCreateParam &param) { DeviceBase::Init(param); }

void GpuDevice::ShutdownGpuDevice() { DeviceBase::Shutdown(); }

SamplerHandle GpuDevice::CreateSampler(const render::SamplerCreation &creation)
{
	SamplerCreation sampler_creation{};
	sampler_creation.name = creation.name.data();
	ToVKEnum(creation.min_filter, sampler_creation.min_filter);
	ToVKEnum(creation.mag_filter, sampler_creation.mag_filter);
	ToVKEnum(creation.mip_filter, sampler_creation.mip_filter);
	ToVKEnum(creation.address_mode_u, sampler_creation.address_mode_u);
	ToVKEnum(creation.address_mode_v, sampler_creation.address_mode_v);
	ToVKEnum(creation.address_mode_w, sampler_creation.address_mode_w);
	ToVKEnum(creation.reduction_mode, sampler_creation.reduction_mode);
	return gpu_resource_manager->CreateSampler(sampler_creation);
}
void GpuDevice::DestroySampler(const SamplerHandle &handle)
{ gpu_resource_manager->DestroySampler(handle, current_frame); }

void GpuDevice::Present()
{ DeviceBase::Commit(); }
} // namespace cloud::vulkan