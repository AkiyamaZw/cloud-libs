#pragma once
#include <cstdint>

namespace cloud
{
enum class TypeDevice
{
	Vulkan
};
struct GpuCreateParam
{
	void *window{nullptr};
	int width;
	int height;
	uint16_t gpu_time_queries_per_frame{32};
	bool enable_gpu_time_queries{false};
	bool enable_debug{false};
	uint32_t max_swapchain_images{3};
	uint32_t max_frames{2};
	TypeDevice type_device{TypeDevice::Vulkan};
};
} // namespace cloud

namespace cloud::render
{
using ResourceHandle = uint32_t;

#define HANDLE_DECLARE(name)                                                                       \
	struct name##Handle                                                                            \
	{                                                                                              \
		ResourceHandle index;                                                                      \
	};                                                                                             \
	static constexpr name##Handle name##InvalidHandle {}

HANDLE_DECLARE(Buffer);
HANDLE_DECLARE(Texture);
HANDLE_DECLARE(DescriptorSetLayout);
HANDLE_DECLARE(ShaderState);
HANDLE_DECLARE(Pipeline);
HANDLE_DECLARE(Sampler);
HANDLE_DECLARE(DescriptorSet);

enum class FilterMode : uint32_t
{
	Point,
	Linear,
};

enum class AddressMode : uint32_t
{
	Wrap,
	Mirror,
	Clamp,
	Border,
	MirrorOnce,
};

enum class ReductionMode : uint32_t
{
	Filter,
	Comparison,
	Minimum,
	Maximum,
};

struct TextureType
{
	enum Enum
	{
		Texture1D,
		Texture2D,
		Texture3D,
		TextureCube,
		Texture_1D_Array,
		Texture_2D_Array,
		Texture_Cube_Array,
		Count
	};
};

struct TextureFlags
{
	enum Enum
	{
		Default,
		RenderTarget,
		Compute,
		Count,
	};
	enum Mask {
		Default = 1 << 0,
		RenderTarget = 1 << 1,
		Compute = 1 << 2, 
	};
};
} // namespace cloud::render