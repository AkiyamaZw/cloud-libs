#pragma once
#include "utility/SingletonSystem.h"
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


class Renderer: public SingletonSystem
{
public:
    DEFINE_SINGLE_SYSTEM(Renderer);

    void Init() override;

    void Exit() override;

    void BeginFrame();

    void EndFrame();

    void ResizeSwapChain(uint32_t width, uint32_t height);
private:
};
} // namespace cloud