#pragma once
#include "graphics/core/gpu_enum.h"
#include "graphics/core/gpu_resource.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/vk_mem_alloc.h"

namespace cloud::vulkan
{
using ResourceType = render::ResourceType;

template <typename T>
struct ResourceTraits;

struct RenderPassOutput
{
	VkFormat color_formats[MaxSwapchainImages];
	VkFormat depth_stencil_format;
	uint32_t num_color_formats{0};

	RenderPassOperation color_operation{RenderPassOperation::DontCare};
	RenderPassOperation depth_operation{RenderPassOperation::DontCare};
	RenderPassOperation stencil_operation{RenderPassOperation::DontCare};

	RenderPassOutput &Reset();
	RenderPassOutput &SetColorFormat(VkFormat format);
	RenderPassOutput &SetDepthFormat(VkFormat format);
	RenderPassOutput &SetOperation(RenderPassOperation color_op,
								   RenderPassOperation depth_op,
								   RenderPassOperation stencil_op);
};

using ResourceHandle = render::ResourceHandle;

constexpr uint32_t InvalidFrameID = UINT32_MAX;

struct ResourceUpdate
{
	ResourceHandle handle;
	uint32_t current_frame;
};

struct DescriptorSetUpdate
{
	ResourceHandle handle;
	uint32_t current_frame;
};

struct ResourceBase
{
	ResourceHandle handle;
	const char *name{nullptr};
};

struct DescriptorBinding
{
	VkDescriptorType descriptor_type;
	uint16_t start{0};
	uint16_t count{0};
	uint16_t set{0};
	const char *name{nullptr};
};

struct DescriptorSetLayout : public ResourceBase
{
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSetLayoutBinding *binding{nullptr};
	DescriptorBinding *descriptor_bindings{nullptr};
	uint16_t num_bindings{0};
	uint16_t set_index{0};
};

struct ShaderState
{
	VkPipelineShaderStageCreateInfo shader_stage_create_info[GMaxShaderStages];
	const char *name{nullptr};
	uint32_t active_shaders{0};
	bool graphics_pipeline{false};
};

struct StencilOperationState
{
	VkStencilOpState fail{VK_STENCIL_OP_KEEP};
	VkStencilOpState pass{VK_STENCIL_OP_KEEP};
	VkStencilOpState depth_fail{VK_STENCIL_OP_KEEP};
	VkCompareOp compare_op{VK_COMPARE_OP_ALWAYS};
	uint32_t compare_mask{0xff};
	uint32_t write_mask{0xff};
	uint32_t reference{0xff};
};

struct DepthStencilCreation
{
	StencilOperationState front;
	StencilOperationState back;
	VkCompareOp depth_compare_op{VK_COMPARE_OP_ALWAYS};
	uint8_t depth_enabled : 1;
	uint8_t depth_write_enabled : 1;
	uint8_t stencil_enabled : 1;
	uint8_t pad : 5;
};

struct BlendState
{
	VkBlendFactor source_color{VK_BLEND_FACTOR_ONE};
	VkBlendFactor destination_color{VK_BLEND_FACTOR_ONE};
	VkBlendOp color_blend_op{VK_BLEND_OP_ADD};

	VkBlendFactor source_alpha{VK_BLEND_FACTOR_ONE};
	VkBlendFactor destination_alpha{VK_BLEND_FACTOR_ONE};
	VkBlendOp alpha_blend_op{VK_BLEND_OP_ADD};
	ColorWriteEnabledMask color_write_mask{ColorWriteEnabledMask::All_Mask};

	uint8_t blend_enabled : 1;
	uint8_t separate_blend : 1;
	uint8_t pad : 6;

	BlendState()
		: blend_enabled(0)
		, separate_blend(0) {};
	BlendState &SetColor(VkBlendFactor in_source_color,
						 VkBlendFactor in_destination_color,
						 VkBlendOp in_color_blend_op);
	BlendState &SetAlpha(VkBlendFactor in_source_alpha,
						 VkBlendFactor in_destination_alpha,
						 VkBlendOp in_alpha_blend_op);
	BlendState &SetColorWriteMask(ColorWriteEnabledMask in_mask);
};

struct BlendStateCreation
{
	BlendState blend_states[MaxSwapchainImages];
	uint32_t active_states{0};
	BlendStateCreation &Reset();
	BlendState &AddBlendState();
};

struct RasterizationCreation
{
	VkCullModeFlagBits cull_mode{VK_CULL_MODE_NONE};
	VkFrontFace front_face{VK_FRONT_FACE_COUNTER_CLOCKWISE};
	FillMode fill_mode{FillMode::Solid};
};

struct Pipeline : public ResourceBase
{
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	VkPipelineBindPoint pipeline_bind_point;
	ResourceHandle shader_state_handle;
	const DescriptorSetLayout *descriptor_set_layout[GMAXDescriptorSetLayouts];
	ResourceHandle descriptor_set_layout_handle[GMAXDescriptorSetLayouts];

	uint32_t num_active_layouts{0};
	DepthStencilCreation depth_stencil;
	BlendStateCreation blend_state;
	RasterizationCreation rasterization;
	bool graphics_pipeline{false};
};

} // namespace cloud::vulkan