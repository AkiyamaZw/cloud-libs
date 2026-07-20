#include "graphics/vulkan/resource/descriptorset.h"
#include "core/data_structure/resource_pool.h"
#include "graphics/vulkan/device_data.h"
#include "graphics/vulkan/vulkan_interface.h"

namespace cloud::vulkan
{
using namespace cloud::vulkan::infra;
void DestroyVkDescriptorSet(const ResourceHandle &handle,
							const DeviceData &device_data,
							ResourceData &resource_data)
{
	DescriptorSet *res = Access<DescriptorSet>(resource_data, handle);
	if (res && res->resources)
	{
		assert(false);
	}
	ReleaseResource(resource_data, handle);
}

}