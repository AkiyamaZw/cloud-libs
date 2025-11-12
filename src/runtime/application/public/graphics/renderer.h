#pragma once
#include "graphics/device.h"
#include "data_structure/resource_manager.h"
#include "vulkan/gpu_resource.h"

namespace cloud::render
{
struct BufferResource: public Resource
{
    BufferHandle buffer_;
    uint32_t pool_index_;
};


void CreateRenderer(GpuCreateParam &param);
void DestroyRenderer();

} // namespace cloud