#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_device.h"

namespace cloud
{

void CreateRenderer(GpuCreateParam &param) { vulkan::InitGpuDevice(param); }

void DestroyRenderer() { vulkan::ShutdownGpuDevice(); }
} // namespace cloud