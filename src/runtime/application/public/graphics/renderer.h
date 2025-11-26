#pragma once
#include "graphics/device.h"
#include "utility/SingletonSystem.h"
#include "data_structure/resource_manager.h"
#include "graphics/core/gpu_enum.h"
#include <string_view>

#include "data_structure/resource_pool.h"
#include "graphics/core/gpu_resource.h"

namespace cloud::render
{

struct BufferResource: public Resource
{
    BufferHandle buffer_;
    uint32_t pool_index_;
    static constexpr std::string_view resource_type{"buffer_resource"};
    BufferHandle handle;
    uint32_t pool_index;
};

struct TextureResource: public Resource
{
    static constexpr std::string_view resource_type{"texture_resource"};
    TextureHandle handle;
    uint32_t pool_index;
};

struct SamplerResource: public Resource
{
    static constexpr std::string_view resource_type{"sampler_resource"};
    SamplerHandle handle;
    uint32_t pool_index;
};

void CreateRenderer(GpuCreateParam &param);
void DestroyRenderer();


struct RendererCreation
{
    // GpuDevice* device;
    uint16_t num_texture{256};
    uint16_t num_sampler{64};
    uint16_t num_buffer{256};
    uint16_t num_material{256};
    uint16_t num_techniques{256};
};

class Renderer: public SingletonSystem
{
public:
    DEFINE_SINGLE_SYSTEM(Renderer);

    Renderer(const RendererCreation& creation);

    ~Renderer() override;

    void Init() override;

    void Exit() override;

    void BeginFrame();

    void EndFrame();

    void ResizeSwapChain(uint32_t width, uint32_t height);

    SamplerResource* CreateSampler(const SamplerCreation& creation);

    TypedResourcePool<SamplerResource> samplers_;

    GpuDevice * gpu_device_;

};
} // namespace cloud