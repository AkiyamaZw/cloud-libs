#pragma once
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/device_data.h"
#include "core/runtime_log.h"

#define COPY_MEMBER(obj_left, obj_right, member) obj_left.member = obj_right.member

namespace cloud::vulkan::infra
{
void InitVulkanInterface(VkDevice device, bool debug_message = true);

void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char *name);

bool CreateVkInstance(InstanceData &instance_data, const GpuCreateParam &param);

void DestroyVkInstance(InstanceData &instance_data);

bool CreateVkPhysicalDevice(const InstanceData &in_instance_data,
							const WindowData &in_window_data,
							DeviceData &out_device_data);

bool CreateVkWindowSurfaceFromGlfw(const InstanceData &instance_data,
								   const GpuCreateParam &param,
								   WindowData &window_data);
void DestroyWindowSurface(const InstanceData &instance_data, WindowData &window_data);

bool CreateVkDeviceAndQueue(DeviceData &device_data);
void DestroyVkDeviceAndQueue(DeviceData &device);

bool CreateVkQueryPool(const GpuCreateParam &param, DeviceData &device_data);

void DestroyVkQueryPool(DeviceData &device_data);

bool CreateVkSwapChain(const DeviceData &device_data, WindowData &window_data);

void DestroyVkSwapchain(const DeviceData &device_data, WindowData &window_data);

// void ResizeVkSwapchain(const DeviceData& device_data, WindowData& window_data);

bool CreateVmaAllocator(const InstanceData &instance_data,
						const DeviceData &device_data,
						ResourceData &resource_data);

void DestroyVmaAllocator(ResourceData &resource_data);

bool CreateVkDescriptorPool(const DeviceData &device_data, ResourceData &resource_data);
void DestroyVkDescriptorPool(const DeviceData &device_data, ResourceData &resource_data);

bool CreateVkSyncMarkers(const DeviceData &device_data, RuntimeLoopData &rl_data);
void DestroyVkSyncMarkers(const DeviceData &device_data, const RuntimeLoopData &rl_data);

bool InitRuntimeLoopData(const DeviceData &device_data, RuntimeLoopData &rl_data);

CommandBuffer *GetInstantCommandBuffer(RuntimeLoopData &rl_data);
void DestoryRuntimeLoopData(const DeviceData &device_data, RuntimeLoopData &rl_data);

void TransitionImageLayout(VkCommandBuffer command_buffer,
						   VkImage &image,
						   VkFormat format,
						   VkImageLayout oldLayout,
						   VkImageLayout newLayout,
						   bool is_depth);

/* buffer start */
/* buffer end */

/* texture start */

/* texture end */

/* render pass start*/

/* render pass end*/

/*  dynamic mapping buffer start */
void *MapBuffer(const DynamicBuffer::MapBufferParameters &param,
				DynamicBuffer &dynamic_buffer,
				ResourceData &resource_data);
void UnMapBuffer(const DynamicBuffer::MapBufferParameters &param,
				 DynamicBuffer &dynamic_buffer,
				 ResourceData &resource_data);
/*  dynamic mapping buffer start end */

/* resource traits function */

void PendingToQueue(RuntimeLoopData &resource_data, ResourceHandle &handle);

void PendingToDestroy(RuntimeLoopData &resource_data, ResourceHandle &handle);

void ReleaseResource(ResourceData &resource_data, const ResourceHandle &handle);

void ReleaseResourceBase(ResourceData &resource_data, const ResourceBase *res);

template <typename T>
T *AllocResource(ResourceData &rd, ResourceType type, const char *name)
{
	ResourceHandle handle = FetchResource(rd, type);
	if (handle.index == ResourcePool::INVALID_NUM)
		return nullptr;
	T *res = Access<T>(rd, handle);
	res->handle = handle;
	res->name = name;
	INFO("Resource \"%s\" created with handle %d", name, handle.index);
	return res;
}
/* resource traits function */

/* descriptor set update */
void update_descriptor_set_instance(DeviceData &device_data,
									RuntimeLoopData &rl_data,
									ResourceData &resource_data,
									const DescriptorSetUpdate &update);
/* descriptor set update */

}; // namespace cloud::vulkan::infra
