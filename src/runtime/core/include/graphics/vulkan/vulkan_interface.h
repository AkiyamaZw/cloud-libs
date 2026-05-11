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

Texture *Access(ResourceData &resource_data, const TextureHandle &hanle);
Texture *AccessTexture(ResourceData &resource_data, const ResourceHandle &handle);

Buffer *Access(ResourceData &resource_data, const BufferHandle &handle);
Buffer *AccessBuffer(ResourceData &resource_data, const ResourceHandle &handle);

Sampler *Access(ResourceData &resource_data, const SamplerHandle &handle);
Sampler *AccessSampler(ResourceData &resource_data, const ResourceHandle &handle);

RenderPass *Access(ResourceData &resource_data, const RenderPassHandle &handle);
RenderPass *AccessRenderPass(ResourceData &resource_data, const ResourceHandle &handle);

/* pool resource access end */

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
TextureHandle CreateVkTexture(const DeviceData &device_data,
							  RuntimeLoopData &rl_data,
							  const TextureCreation &creation,
							  ResourceData &resource_data);
void DestroyTexture(RuntimeLoopData &rl_data, TextureHandle &handle);

void DestroyVkSamplerInstance(const ResourceHandle &handle,
							  const DeviceData &device_data,
							  ResourceData &resource_data);

/* texture end */

/* render pass start*/
void CreateVkFrameBuffer(const DeviceData &device_data,
						 ResourceData &resource_data,
						 RenderPass &rp,
						 const TextureHandle *out_textures,
						 const uint32_t num_rt,
						 const TextureHandle &depth_stencil_tex);

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

RenderPassHandle CreateVkRenderPass(const RenderPassCreation &creation,
									const DeviceData &device_data,
									RuntimeLoopData &rl_data,
									WindowData &window_data,
									ResourceData &resource_data,
									RenderPass &render_pass);

void DestroyRenderPass(RuntimeLoopData &rl_data, RenderPassHandle &handle);

void DestroyRenderPassInstance(const DeviceData &device_data,
							   ResourceData &resource_data,
							   ResourceHandle handle);
/* render pass end*/
}; // namespace cloud::vulkan::infra