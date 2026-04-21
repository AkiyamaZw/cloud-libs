#include "graphics/vulkan/command_buffer.h"

namespace cloud::vulkan
{
void CommandBuffer::Init(QueueType in_queue_type,
						 uint32_t in_buffer_size,
						 uint32_t in_submit_size,
						 bool in_backed)
{
	queue_type = in_queue_type;
	buffer_size = in_buffer_size;
	backed = in_backed;
	Reset();
}

void CommandBuffer::Destroy() { is_recoding = false; }

void CommandBuffer::Reset()
{
	is_recoding = false;
	current_pipeline = nullptr;
	current_render_pass = nullptr;
	current_command = 0;
}

void CommandBufferRing::Init(VkDevice device, uint32_t queue_family)
{
	for (auto &i : vk_command_pool)
	{
		VkCommandPoolCreateInfo cmd_pool_info{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
		};
		cmd_pool_info.queueFamilyIndex = queue_family;
		cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		check_vk(vkCreateCommandPool(device, &cmd_pool_info, nullptr, &i));
	}

	for (uint32_t i = 0; i < GMaxBuffers; i++)
	{
		VkCommandBufferAllocateInfo cmd_alloc_cmd = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
													 nullptr};
		const uint32_t pool_index = IndexInPool(i);
		cmd_alloc_cmd.commandPool = vk_command_pool[pool_index];
		cmd_alloc_cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_alloc_cmd.commandBufferCount = 1;
		check_vk(
			vkAllocateCommandBuffers(device, &cmd_alloc_cmd, &command_buffer[i].vk_command_buffer));
		command_buffer[i].handle = i;
		command_buffer[i].Reset();
	}
}

void CommandBufferRing::Destroy(VkDevice device)
{
	for (auto &i : vk_command_pool)
	{
		vkDestroyCommandPool(device, i, nullptr);
	}
}

void CommandBufferRing::Reset(VkDevice device, uint32_t frame_index)
{
	for (uint32_t i = 0; i < GMaxThreads; ++i)
	{
		vkResetCommandPool(device, vk_command_pool[frame_index * GMaxThreads + i], 0);
	}
}

CommandBuffer *CommandBufferRing::GetCommandBuffer(uint32_t frame_index, bool begin)
{
	CommandBuffer *cmd_buffer = &command_buffer[frame_index * GBuffersPerPool];
	if (begin)
	{
		cmd_buffer->Reset();
		VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd_buffer->vk_command_buffer, &begin_info);
	}
	return cmd_buffer;
}

CommandBuffer *CommandBufferRing::GetCommandBufferInstant(uint32_t frame_index, bool begin)
{
	CommandBuffer *cmd_buffer = &command_buffer[frame_index * GBuffersPerPool + 1];
	return cmd_buffer;
}
} // namespace cloud::vulkan