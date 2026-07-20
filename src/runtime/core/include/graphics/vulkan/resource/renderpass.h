#pragma once
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct RenderPassCreation
{
	uint16_t num_render_targets{0};
	RenderPassType type{RenderPassType::Geometry};
	ResourceHandle output_textures[MaxSwapchainImages];
	ResourceHandle depth_stencil_texture;
	float scale_x{1.f};
	float scale_y{1.f};
	uint8_t resize = 1;
	RenderPassOperation color_op{RenderPassOperation::DontCare};
	RenderPassOperation depth_op{RenderPassOperation::DontCare};
	RenderPassOperation stencil_op{RenderPassOperation::DontCare};
	const char *name{nullptr};
};

struct RenderPass : public ResourceBase
{
	VkRenderPass vk_render_pass;
	VkFramebuffer vk_frame_buffer;
	RenderPassOutput output;
	ResourceHandle out_textures[MaxSwapchainImages];
	ResourceHandle out_depth;
	float scale_x{1.f};
	float scale_y{1.f};
	uint16_t width{0};
	uint16_t height{0};
	uint16_t dispatch_x{1};
	uint16_t dispatch_y{1};
	uint16_t dispatch_z{1};
	uint8_t resize{1};
	RenderPassType type;

	uint8_t num_render_targets{0};
	uint32_t multiview_mask{0};
};

template <>
struct ResourceTraits<RenderPass>
{
	static constexpr ResourceType type = ResourceType::RenderPass;
};

struct DeviceData;
struct ResourceData;
struct RuntimeLoopData;
struct WindowData;

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

void DestroyVkRenderPass(const ResourceHandle &handle,
						 const DeviceData &device_data,
						 ResourceData &resource_data);

} // namespace cloud::vulkan