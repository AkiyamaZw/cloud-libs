#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
RenderPassOutput& RenderPassOutput::Reset()
{
    num_color_formats = 0;
    for (auto & color_format : color_formats)
    {
        color_format = VK_FORMAT_UNDEFINED;
    }
    depth_stencil_format = VK_FORMAT_UNDEFINED;
    color_operation = RenderPassOperation::DontCare;
    depth_operation = RenderPassOperation::DontCare;
    stencil_operation = RenderPassOperation::DontCare;
    return *this;
}

RenderPassOutput & RenderPassOutput::SetColorFormat(VkFormat format)
{
    color_formats[num_color_formats++] = format;
    return *this;
}

RenderPassOutput & RenderPassOutput::SetDepthFormat(VkFormat format)
{
    depth_stencil_format = format;
    return *this;
}

RenderPassOutput & RenderPassOutput::SetOperation(RenderPassOperation color_op,
    RenderPassOperation depth_op,
    RenderPassOperation stencil_op)
{
    color_operation = color_op;
    depth_operation = depth_op;
    stencil_operation = stencil_op;
    return *this;
}
BlendState &BlendState::SetColor(VkBlendFactor in_source_color,
								 VkBlendFactor in_destination_color,
								 VkBlendOp in_color_blend_op)
{
    source_color = in_source_color;
    destination_color = in_destination_color;
    color_blend_op = in_color_blend_op;
    blend_enabled = 1;
    return *this;

}
BlendState &BlendState::SetAlpha(VkBlendFactor in_source_alpha,
								 VkBlendFactor in_destination_alpha,
								 VkBlendOp in_alpha_blend_op)
{
    source_alpha = in_source_alpha;
    destination_alpha = in_destination_alpha;
    alpha_blend_op = in_alpha_blend_op;
    separate_blend = 1;
    return *this;
}

BlendState &BlendState::SetColorWriteMask(ColorWriteEnabledMask in_mask)
{
    color_write_mask = in_mask;
    return *this;
}
BlendStateCreation &BlendStateCreation::Reset()
{
    active_states = 0;
    return *this;
}

BlendState &BlendStateCreation::AddBlendState()
{
    return blend_states[active_states++];
}

} // namespace cloud::vulkan