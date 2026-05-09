#pragma once
// #include "gpu_resource_manager.h"
#include "graphics/vulkan/device_data.h"

namespace cloud::vulkan
{

struct VulkanDeviceContext
{
	/* basic api object */
};

void Init(GpuCreateParam &param);
void Shutdown();

} // namespace cloud::vulkan