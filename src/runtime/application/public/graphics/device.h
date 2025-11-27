#pragma once
#include <cstdint>
#include "graphics/core/gpu_resource.h"

namespace cloud::render
{
struct GpuDevice
{
	virtual ~GpuDevice() = default;
	virtual void InitGpuDevice(GpuCreateParam &param) = 0;
	virtual void ShutdownGpuDevice() = 0;
	virtual SamplerHandle CreateSampler(const SamplerCreation &creation) = 0;
};

GpuDevice *CreateGpuDevice(const TypeDevice &type);
} // namespace cloud::render