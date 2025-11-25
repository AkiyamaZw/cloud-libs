#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_device.h"
namespace cloud::render
{
Renderer g_render(RendererCreation{});

Renderer * Renderer::Inst()
{
    return &g_render;
}

Renderer::Renderer(const RendererCreation &creation)
    :samplers_(128)
{

}

Renderer::~Renderer()
{
}

void Renderer::Init()
{
}

void Renderer::Exit()
{
    samplers_.Shutdown();
}

void Renderer::BeginFrame()
{
}

void Renderer::EndFrame()
{
}

void Renderer::ResizeSwapChain(uint32_t width, uint32_t height)
{
}

SamplerResource * Renderer::CreateSampler(const SamplerCreation& creation)
{
    SamplerResource * sampler = samplers_.Fetch();
    if (sampler)
    {
        SamplerHandle handle = vulkan::GpuDevice::Inst()->CreateSampler(creation);
        sampler->handle = handle;
        sampler->name = creation.name;

    }
    return sampler;
}

void CreateRenderer(GpuCreateParam &param)
{
    Renderer* renderer = Renderer::Inst();
    vulkan::GpuDevice::Inst()->InitGpuDevice(param);
}

void DestroyRenderer()
{
    vulkan::GpuDevice::Inst()->ShutdownGpuDevice();
    Renderer::Inst()->Exit();
}
} // namespace cloud