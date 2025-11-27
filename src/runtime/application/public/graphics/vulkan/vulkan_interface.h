#pragma once
#include "graphics/vulkan/gpu_resource.h"
#include "graphics/vulkan/minimal_extern.h"

namespace cloud::vulkan::infra
{
void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char* name);

void CreateSampler(VkDevice device, const SamplerCreation& creation, VkSampler sampler);
};
