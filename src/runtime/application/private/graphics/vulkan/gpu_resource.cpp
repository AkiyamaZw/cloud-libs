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

}