#include "graphics/device.h"

namespace cloud::render
{
GpuDevice *CreateGpuDevice(const TypeDevice &type);
void DestroyGpuDevice(GpuDevice *device);
} // namespace cloud::render
