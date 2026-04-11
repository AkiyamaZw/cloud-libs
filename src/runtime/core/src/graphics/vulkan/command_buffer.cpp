#include "graphics/vulkan/command_buffer.h"

namespace cloud::vulkan
{
void CommandBuffer::Init(QueueType in_queue_type,  uint32_t in_buffer_size, uint32_t in_submit_size, bool in_backed)
{
    queue_type = in_queue_type;
    buffer_size = in_buffer_size;
    backed = in_backed;
    Reset();
}

void CommandBuffer::Destroy()
{
    is_recoding = false;
}

void CommandBuffer::Reset()
{
    is_recoding = false;
    current_pipeline = nullptr;
    current_render_pass = nullptr;
    current_command = 0;
}
}