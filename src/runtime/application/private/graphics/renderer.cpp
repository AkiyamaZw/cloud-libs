#include "graphics/renderer.h"
#include "graphics/vulakn/vulkan_device.h"

namespace cloud
{

void CreateRenderer(GpuCreateParam &param) { vulkan::InitGpuDevice(param); }

void DestroyRenderer() { vulkan::ShutdownGpuDevice(); }
} // namespace cloud