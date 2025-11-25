#pragma once
#include <cstdint>


namespace cloud::render
{
using ResourceHandle = uint32_t;

#define HANDLE_DECLARE(name) \
struct name##Handle{ ResourceHandle index; }

HANDLE_DECLARE(Buffer);
HANDLE_DECLARE(Texture);
HANDLE_DECLARE(DescriptorSetLayout);
HANDLE_DECLARE(ShaderState);
HANDLE_DECLARE(Pipeline);
HANDLE_DECLARE(Sampler);

enum class FilterMode: uint32_t
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
}