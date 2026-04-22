#pragma once
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/device_data.h"

namespace cloud::vulkan::infra
{
void InitVulkanInterface(VkDevice device, bool debug_message = true);

void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char *name);

bool CreateVkInstance(InstanceData &instance_data, const GpuCreateParam &param);

void DestroyVkInstance(InstanceData &instance_data);

bool CreateVkPhysicalDevice(const InstanceData &in_instance_data,
							const WindowData &in_window_data,
							DeviceData &out_device_data);

bool CreateVkWindowDataFromGlfw(const InstanceData &instance_data,
								const GpuCreateParam &param,
								WindowData &window_data);
bool DestroyWindowData(const InstanceData &instance_data, WindowData &window_data);

bool CreateVkDeviceAndQueue(DeviceData &device_data);

bool CreateVkQueryPool(const GpuCreateParam &param, DeviceData &device_data);

void DestroyVkQueryPool(DeviceData &device_data);

bool CreateVkSwapChain(const DeviceData &device_data, WindowData &window_data);

void DestroySwapchain(const DeviceData &device_data, WindowData &window_data);

bool CreateVmaAllocator(const InstanceData &instance_data,
						const DeviceData &device_data,
						ResourceData &resource_data);

void DestroyVmaAllocator(ResourceData &resource_data);

bool CreateVkRenderPass(const WindowData &window_data,
						const DeviceData &device_data,
						RenderPipelineData &rp_data);

void DestroyVkRenderPass(const DeviceData &device_data, RenderPipelineData &rp_data);

void CreateVkFramebuffers(const DeviceData &device_data,
						  const RenderPipelineData &rp_data,
						  WindowData &window_data);

bool CreateVkSyncMarkers(const DeviceData &device_data, RuntimeLoopData &rl_data);
void DestroyVkSyncMarkers(const DeviceData &device_data, const RuntimeLoopData &rl_data);

/* sampler start */
SamplerHandle CreateVkSampler(const DeviceData &device_data,
							  ResourceData &resource_data,
							  const SamplerCreation &creation);
// 即将被弃用
void CreateSampler(VkDevice device, const SamplerCreation &creation, VkSampler &sampler);

void DestroyVkSampler(const SamplerHandle &handle, RuntimeLoopData &rl_data);

void DestroyVkSamplerInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data);
/* sampler end*/

/* buffer start */
BufferHandle CreateVkBuffer(const BufferCreation &creation,
						  const DeviceData &device_data,
						  ResourceData &resource_data);

void DestroyVkBuffer(const BufferHandle &handle, RuntimeLoopData &rl_data);
void DestroyVkBufferInstance(const ResourceHandle &handle, ResourceData &resource_data);
/* buffer end */

/* texture start */

/* texture end */
}; // namespace cloud::vulkan::infra