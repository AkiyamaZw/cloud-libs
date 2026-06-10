#include "graphics/vulkan/device_data.h"
#include "core/runtime_log.h"

namespace cloud::vulkan
{
void AdvanceFrameCounter(FrameAdanceCounter &counter) {
	
	counter.previous_frame = counter.current_frame;
	counter.current_frame = (counter.current_frame + 1) % s_swapchain_image_count;
	counter.absolute_frame++;
}

bool InitializeContextInstance(InstanceData &instance_data, const GpuCreateParam &param)
{
	return true;
}

bool DestroyContextInstance(InstanceData &instance_data)
{
	bool succ = false;
	return succ;
}
} // namespace cloud::vulkan
