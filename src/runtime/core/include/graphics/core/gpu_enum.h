#pragma once
#include <cstdint>
#include <type_traits>

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
// 临时的hash函数
template <class T>
constexpr void hash_combine(std::size_t &seed, const T &v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class T>
constexpr std::size_t generate_hash(const std::size_t &seed, const T &v)
{
	std::hash<T> hasher;
	return seed ^ (hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

using ResourceHandle = uint32_t;

#define HANDLE_DECLARE(name)                                                                       \
	struct name##Handle                                                                            \
	{                                                                                              \
		ResourceHandle index;                                                                      \
	};                                                                                             \
	static constexpr name##Handle name##InvalidHandle {}

HANDLE_DECLARE(DescriptorSetLayout);
HANDLE_DECLARE(ShaderState);
HANDLE_DECLARE(Pipeline);
HANDLE_DECLARE(Sampler);
HANDLE_DECLARE(DescriptorSet);
HANDLE_DECLARE(RenderPass);

template <typename FLAG_T, typename T>
bool HasAny(const FLAG_T flag, T code)
{
	FLAG_T enum_num = static_cast<FLAG_T>(code);
	return (flag & enum_num) == enum_num;
}

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

enum class TextureType
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

struct TextureFlags
{
	enum class Enum
	{
		Default,
		RenderTarget,
		Compute,
		Count,
	};
	enum class Mask
	{
		Default = 1 << 0,
		RenderTarget = 1 << 1,
		Compute = 1 << 2,
	};
};
} // namespace cloud::render