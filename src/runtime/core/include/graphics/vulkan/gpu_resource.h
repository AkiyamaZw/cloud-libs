#pragma once
#include "graphics/core/gpu_enum.h"
#include "graphics/core/gpu_resource.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/vk_mem_alloc.h"

namespace cloud::vulkan
{

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
using DescriptorSetLayoutHandle = render::DescriptorSetLayoutHandle;
using PipelineHandle = render::PipelineHandle;
using SamplerHandle = render::SamplerHandle;
using DescriptorSetHandle = render::DescriptorSetHandle;
using ShaderStateHandle = render::ShaderStateHandle;
using RenderPassHandle = render::RenderPassHandle;

constexpr uint32_t InvalidFrameID = UINT32_MAX;

struct ResourceUpdate
{
	ResourceUpdateType type;
	ResourceHandle handle;
	uint32_t current_frame;
};

struct DescriptorSetUpdate
{
	DescriptorSetHandle descriptor_set;
	uint32_t current_frame;
};

struct BufferCreation
{
	VkBufferUsageFlags usage_flags{0};
	ResourceUsageType usage_type{ResourceUsageType::Immutable};
	uint32_t size{0};
	void *initial_data{nullptr};
	const char *name;
};

struct Buffer
{
	ResourceHandle handle;
	ResourceHandle parent_handle;
	VkBuffer buffer;
	VmaAllocation allocation;
	VkDeviceMemory memory;
	VkDeviceSize device_size;
	VkBufferUsageFlags usage_flags{0};
	ResourceUsageType usage_type{ResourceUsageType::Immutable};
	uint32_t size = 0;
	uint32_t global_offset{0};
	bool ready{false};
	uint8_t *mapped_data{nullptr};
	const char *name{nullptr};
};

struct Sampler
{
	VkSampler sampler;
	VkFilter min_filter{VK_FILTER_NEAREST};
	VkFilter mag_filter{VK_FILTER_NEAREST};
	VkSamplerMipmapMode mipmap_mode{VK_SAMPLER_MIPMAP_MODE_NEAREST};
	VkSamplerAddressMode address_mode_u{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_v{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_w{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerReductionMode reduction_mode{VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE};
	const char *name{nullptr};
};

struct SamplerCreation
{
	const char *name;
	VkFilter min_filter{VK_FILTER_NEAREST};
	VkFilter mag_filter{VK_FILTER_NEAREST};
	VkSamplerMipmapMode mip_filter{VK_SAMPLER_MIPMAP_MODE_NEAREST};
	VkSamplerAddressMode address_mode_u{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_v{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerAddressMode address_mode_w{VK_SAMPLER_ADDRESS_MODE_REPEAT};
	VkSamplerReductionMode reduction_mode{VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE};
};

struct TextureCreation
{
	void *initial_data{nullptr};
	uint16_t width = 1;
	uint16_t height = 1;
	uint16_t depth = 1;
	uint8_t mipmaps = 1;
	uint8_t flags = 1; // bitmask
	VkFormat format{VK_FORMAT_UNDEFINED};
	TextureType type{TextureType::Texture2D};
	const char *name{nullptr};
};

struct Texture
{
	ResourceHandle handle;
	ResourceHandle parent_handle;
	VkImage image;
	VkImageView view;
	VkFormat format;
	VkImageLayout layout;
	VkImageUsageFlags usage_flags;
	VmaAllocation allocation;
	ResourceState state{ResourceState::RESOURCE_STATE_UNDEFINED};
	uint16_t width{1};
	uint16_t height{1};
	uint16_t depth{1};
	uint8_t mipmaps{1};
	uint8_t flags{0};
	uint16_t mip_base_levels{0};
	uint16_t array_base_layer{0};
	bool sparse{false};
	TextureType type{TextureType::Texture2D};
	const char *name{nullptr};
	Sampler *sampler;
};

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

struct RenderPass
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

struct DescriptorSetLayout
{
	DescriptorSetLayoutHandle handle;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSetLayoutBinding *binding{nullptr};
	DescriptorBinding *descriptor_bindings{nullptr};
	uint16_t num_bindings{0};
	uint16_t set_index{0};
};

struct DescriptorSet
{
	VkDescriptorSet descriptor_set;
	ResourceHandle *resources{nullptr};
	SamplerHandle *samplers{nullptr};
	uint16_t *bindings{nullptr};
	const DescriptorSetLayout *layout{nullptr};
	uint32_t num_resources{0};
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

struct Pipeline
{
	PipelineHandle handle;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	VkPipelineBindPoint pipeline_bind_point;
	ShaderStateHandle shader_state_handle;
	const DescriptorSetLayout *descriptor_set_layout[GMAXDescriptorSetLayouts];
	DescriptorSetLayoutHandle descriptor_set_layout_handle[GMAXDescriptorSetLayouts];

	uint32_t num_active_layouts{0};
	DepthStencilCreation depth_stencil;
	BlendStateCreation blend_state;
	RasterizationCreation rasterization;
	bool graphics_pipeline{false};
};

} // namespace cloud::vulkan