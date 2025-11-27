#include "graphics/device.h"
#include "graphics/vulkan/vulkan_device.h"

namespace cloud::render
{
GpuDevice *CreateGpuDevice(const TypeDevice &type)
{
	if (type == TypeDevice::Vulkan)
	{
		return vulkan::GpuDevice::Inst();
	}
	return nullptr;
}
} // namespace cloud::render
