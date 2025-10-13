#pragma once
#include "vulkan/vulkan.h"
#include "graphics/device.h"

namespace cloud::vulkan
{

void InitGpuDevice(GpuCreateParam &param);
void ShutdownGpuDevice();

} // namespace cloud::vulkan