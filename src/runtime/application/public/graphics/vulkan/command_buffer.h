#pragma once
#include "graphics/vulkan/minimal_extern.h"
#include "graphics/vulkan/gpu_enums.h"
#include "graphics/vulkan/gpu_resource.h"

namespace cloud::vulkan
{
struct CommandBuffer
{
    VkCommandBuffer vk_command_buffer;
    VkDescriptorSet vk_descriptor_set[16];
    RenderPass* current_render_pass;
    Pipeline* current_pipeline;
    VkClearValue clear_value[2];
    bool is_recoding;
    uint32_t handle;
    uint32_t current_command;
    ResourceHandle resource_handle;
    QueueType queue_type;
    uint32_t buffer_size{0};
    bool backed {false};

    void Init(QueueType in_queue_type,  uint32_t in_buffer_size, uint32_t in_submit_size, bool in_backed);
    void Destroy();
    void Reset();
};
}