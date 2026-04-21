#pragma once
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/device_data.h"

namespace cloud::vulkan::infra
{
void InitVulkanInterface(VkDevice device, bool debug_message = true);

void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char *name);

void CreateSampler(VkDevice device, const SamplerCreation &creation, VkSampler &sampler);

bool CreateVkInstance(InstanceData &instance_data, const GpuCreateParam &param);

void DestoryVkInstance(InstanceData &instance_data);

bool CreateVkPhysicalDevice(const InstanceData &in_instance_data,
							const WindowData &in_window_data,
							DeviceData &out_device_data);

bool CreateVkWindowDataFromGlfw(const InstanceData &instance_data,
								const GpuCreateParam &param,
								WindowData &window_data);
bool DestoryWindowData(const InstanceData &instance_data, WindowData &window_data);

bool CreateVkDeviceAndQueue(DeviceData &device_data);

bool CreateVkQueryPool(const GpuCreateParam &param, DeviceData &device_data);

void DestoryVkQueryPool(DeviceData &device_data);

bool CreateVkSwapChain(const DeviceData &device_data, WindowData &window_data);

void DestorySwapchain(const DeviceData &device_data, WindowData &window_data);

bool CreateVmaAllocator(const InstanceData &instance_data,
						const DeviceData &device_data,
						ResourceData &resource_data);

void DestoryVmaAllocator(ResourceData &resource_data);

bool CreateVkRenderPass(const WindowData &window_data,
					  const DeviceData &device_data,
					  RenderPipelineData &rp_data);

void DestoryVkRenderPass(const DeviceData &device_data, RenderPipelineData &rp_data);

void CreateVkFramebuffers(const DeviceData &device_data,
						  const RenderPipelineData &rp_data,
						  WindowData &window_data);


}; // namespace cloud::vulkan::infra