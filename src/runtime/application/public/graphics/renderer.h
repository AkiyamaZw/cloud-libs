#pragma once
#include "graphics/device.h"
#include "data_structure/resource_manager.h"
#include "vulkan/gpu_resource.h"

namespace cloud::render
{
struct BufferResource: public Resource
{
struct GpuDevice;
    BufferHandle buffer_;
    uint32_t pool_index_;
};


class Renderer
{
public:
    static Renderer* Inst();
};
void CreateRenderer(GpuCreateParam &param);
void DestroyRenderer();



} // namespace cloud