#pragma once
#include "vulkan/vulkan.h"
#include "graphics/device.h"

namespace cloud::vulkan
{
static constexpr uint32_t MaxSwapchainImages = 3;

void InitGpuDevice(GpuCreateParam &param);
void ShutdownGpuDevice();

} // namespace cloud::vulkan