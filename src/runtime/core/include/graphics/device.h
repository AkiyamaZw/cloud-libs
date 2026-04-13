#pragma once
#include <cstdint>
#include "graphics/core/gpu_resource.h"

namespace cloud::render
{

struct GpuDevice
{
	virtual ~GpuDevice() = default;
	virtual TypeDevice GetTypeDevice() const = 0;
	virtual void InitGpuDevice(GpuCreateParam &param) = 0;
	virtual void ShutdownGpuDevice() = 0;
	virtual SamplerHandle CreateSampler(const SamplerCreation &creation) = 0;
	virtual void DestroySampler(const SamplerHandle &handle) = 0;
	virtual void DrawFrame() = 0;
};

GpuDevice *CreateGpuDevice(const TypeDevice &type);
void DestroyGpuDevice(GpuDevice *device);
} // namespace cloud::render