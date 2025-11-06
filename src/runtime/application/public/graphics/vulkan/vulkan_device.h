#pragma once
#include "graphics/device.h"

namespace cloud::vulkan
{

void InitGpuDevice(GpuCreateParam &param);
void ShutdownGpuDevice();

} // namespace cloud::vulkan