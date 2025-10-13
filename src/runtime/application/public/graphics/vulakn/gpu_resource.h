#pragma once
#include "gpu_enums.h"
#include "vulkan/vulkan.h"

namespace cloud::vulkan{

    
struct RenderPassOutput{
    VkFormat color_formats[MaxSwapchainImages];
    VkFormat depth_stencil_format;
    uint32_t num_corlor_formats{0};

    RenderPassOperation color_operation{RenderPassOperation::DontCare};
    RenderPassOperation depth_operation{RenderPassOperation::DontCare};
    RenderPassOperation stencil_operation{RenderPassOperation::DontCare};

    RenderPassOutput& Reset();
    RenderPassOutput& SetColorFormat(VkFormat format);
    RenderPassOutput& SetDepthFormat(VkFormat format);
    RenderPassOutput& SetOperation(RenderPassOperation color_op, RenderPassOperation depth_operation, RenderPassOperation stencil_op);
};

}