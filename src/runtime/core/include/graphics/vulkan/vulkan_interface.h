#pragma once
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/minimal_extern.h"

namespace cloud::vulkan::infra
{
void InitVulkanInterface(VkDevice device, bool debug_message=true);

void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char* name);

void CreateSampler(VkDevice device, const SamplerCreation& creation, VkSampler& sampler);

};
