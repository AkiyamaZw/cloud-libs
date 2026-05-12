#pragma once
// #include "gpu_resource_manager.h"
#include "graphics/vulkan/device_data.h"

namespace cloud::vulkan
{

struct VulkanDeviceContext
{
	InstanceData instance_data;
	DeviceData device_data;
	WindowData window_data;
	ResourceData resource_data;
	RuntimeLoopData runtime_data;
};

void Init(VulkanDeviceContext &vdc, GpuCreateParam &param);
void Shutdown(VulkanDeviceContext &vdc);
void InitDefaultResource(VulkanDeviceContext &vdc);
void DestoryDefaultResource(VulkanDeviceContext &vdc);
} // namespace cloud::vulkan