#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_device.h"

namespace cloud::render
{

static Renderer g_renderer;

Renderer* Renderer::Inst()
{
    return &g_renderer;
}

void CreateRenderer(GpuCreateParam &param) { vulkan::InitGpuDevice(param); }

void DestroyRenderer() { vulkan::ShutdownGpuDevice(); }
} // namespace cloud