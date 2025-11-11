#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_device.h"
namespace cloud
{
Renderer g_render;

Renderer * Renderer::Inst()
{
    return &g_render;
}

void CreateRenderer(GpuCreateParam &param)
{
    Renderer* renderer = Renderer::Inst();
    vulkan::GpuDevice::Inst()->InitGpuDevice(param);
}

void DestroyRenderer()
{
    vulkan::GpuDevice::Inst()->ShutdownGpuDevice();
}
} // namespace cloud