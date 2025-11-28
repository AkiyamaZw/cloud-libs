#include "graphics/device.h"
#include "graphics/vulkan/vulkan_device.h"

namespace cloud::render
{
GpuDevice *CreateGpuDevice(const TypeDevice &type)
{
	if (type == TypeDevice::Vulkan)
	{
		return new vulkan::GpuDevice();
	}
	return nullptr;
}

void DestroyGpuDevice(GpuDevice* device)
{
    if (device->GetTypeDevice() == TypeDevice::Vulkan)
    {
        delete device;
    }
}
} // namespace cloud::render
