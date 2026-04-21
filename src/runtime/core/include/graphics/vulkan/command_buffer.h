#pragma once
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct CommandBuffer
{
	VkCommandBuffer vk_command_buffer{nullptr};
	VkDescriptorSet vk_descriptor_set[16];
	RenderPass *current_render_pass;
	Pipeline *current_pipeline;
	VkClearValue clear_value[2];
	bool is_recoding;
	uint32_t handle;
	uint32_t current_command;
	ResourceHandle resource_handle;
	QueueType queue_type;
	uint32_t buffer_size{0};
	bool backed{false};

	void Init(QueueType in_queue_type,
			  uint32_t in_buffer_size,
			  uint32_t in_submit_size,
			  bool in_backed);
	void Destroy();
	void Reset();
};

struct CommandBufferRing
{
	static const uint16_t GMaxThreads = 1;
	static const uint16_t GMaxPools = MaxSwapchainImages * GMaxThreads;
	static const uint16_t GBuffersPerPool = 4;
	static const uint16_t GMaxBuffers = GBuffersPerPool * GMaxPools;

	VkCommandPool vk_command_pool[GMaxPools];
	CommandBuffer command_buffer[GMaxBuffers];
	uint8_t next_free_per_thread_frame[GMaxPools];
	void Init(VkDevice device, uint32_t queue_family);
	void Destroy(VkDevice device);
	void Reset(VkDevice device, uint32_t frame_index);
	static uint32_t IndexInPool(uint32_t index) { return (uint16_t)index / GBuffersPerPool; }
	CommandBuffer *GetCommandBuffer(uint32_t frame_index, bool begin);
	CommandBuffer *GetCommandBufferInstant(uint32_t frame_index, bool begin);
};

} // namespace cloud::vulkan