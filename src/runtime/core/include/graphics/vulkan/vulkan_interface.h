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

/* pool resource access start */
Texture *AccessTexture(ResourceData &resource_data, const ResourceHandle &handle);

Buffer *AccessBuffer(ResourceData &resource_data, const ResourceHandle &handle);

Sampler *AccessSampler(ResourceData &resource_data, const ResourceHandle &handle);

RenderPass *AccessRenderPass(ResourceData &resource_data, const ResourceHandle &handle);
/* pool resource access end */

/* sampler start */
ResourceHandle CreateVkSampler(const DeviceData &device_data,
							   ResourceData &resource_data,
							   const SamplerCreation &creation);
// 即将被弃用
void CreateSampler(VkDevice device, const SamplerCreation &creation, VkSampler &sampler);

void DestroyVkSampler(const ResourceHandle &handle, RuntimeLoopData &rl_data);

void DestroyVkSamplerInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data);
/* sampler end*/

/* buffer start */
ResourceHandle CreateVkBuffer(const BufferCreation &creation,
							  const DeviceData &device_data,
							  ResourceData &resource_data);

void DestroyVkBuffer(const ResourceHandle &handle, RuntimeLoopData &rl_data);
void DestroyVkBufferInstance(const ResourceHandle &handle,
							 const DeviceData &device_data,
							 ResourceData &resource_data);
/* buffer end */

/* texture start */
ResourceHandle CreateVkTexture(const DeviceData &device_data,
							   RuntimeLoopData &rl_data,
							   const TextureCreation &creation,
							   ResourceData &resource_data);
void DestroyVkTexture(ResourceHandle &handle, RuntimeLoopData &rl_data);

void DestroyVkTextureInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data);

/* texture end */

/* render pass start*/
void CreateVkFrameBuffer(const DeviceData &device_data,
						 ResourceData &resource_data,
						 RenderPass &rp,
						 const ResourceHandle *out_textures,
						 const uint32_t num_rt,
						 const ResourceHandle &depth_stencil_tex);

RenderPassOutput FillRenderPassOutput(const RenderPassCreation &creation,
									  ResourceData &resource_data);

void CreateVkSwapchainRenderPass(const DeviceData &device_data,
								 RuntimeLoopData &rl_data,
								 WindowData &window_data,
								 ResourceData &resource_data,
								 RenderPass &render_pass);

VkRenderPass GetVkRenderPass(const DeviceData &device_data,
							 ResourceData &resource_data,
							 const RenderPassOutput &output,
							 const char *name);

ResourceHandle CreateVkRenderPass(const RenderPassCreation &creation,
								  const DeviceData &device_data,
								  RuntimeLoopData &rl_data,
								  WindowData &window_data,
								  ResourceData &resource_data);

void DestroyVkRenderPass(ResourceHandle &handle, RuntimeLoopData &rl_data);

void DestroyVkRenderPassInstance(const ResourceHandle &handle,
								 const DeviceData &device_data,
								 ResourceData &resource_data);
/* render pass end*/

/*  dynamic mapping buffer start */
void *MapBuffer(const DynamicBuffer::MapBufferParameters &param,
				DynamicBuffer &dynamic_buffer,
				ResourceData &resource_data);
void UnMapBuffer(const DynamicBuffer::MapBufferParameters &param,
				 DynamicBuffer &dynamic_buffer,
				 ResourceData &resource_data);
/*  dynamic mapping buffer start end */

void DestroyResourceInstance(RuntimeLoopData &rl_data,
							 const DeviceData &device_data,
							 ResourceData &resource_data);

/* abstract impl */

}; // namespace cloud::vulkan::infra