#pragma once
#include <cstdint>

namespace cloud::vulkan
{
static constexpr uint32_t MaxSwapchainImages = 3;

enum class RenderPassOperation
{
	DontCare,
	Load,
	Clear,
	Count
};
}