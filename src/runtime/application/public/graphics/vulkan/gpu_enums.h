#pragma once
#include <cstdint>

namespace cloud::vulkan
{
static constexpr uint32_t MaxSwapchainImages = 3;
static constexpr uint32_t GlobalPoolElements = 128;
static constexpr uint8_t GMaxShaderStages = 5;
static constexpr uint8_t GMAXDescriptorSetLayouts=8;

enum class RenderPassOperation
{
    DontCare,
    Load,
    Clear,
    Count
};

enum class PresentMode
{
    Immediate,
    VSync,
    VSyncFast,
    VSyncRelaxed,
    Count
};

enum class ResourceUsageType
{
    Immutable,
    Dynamic,
    Stream,
    Staging,
    ReadBack,
    Count
};

typedef enum ResourceState {
    RESOURCE_STATE_UNDEFINED = 0,
    RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
    RESOURCE_STATE_INDEX_BUFFER = 0x2,
    RESOURCE_STATE_RENDER_TARGET = 0x4,
    RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
    RESOURCE_STATE_DEPTH_WRITE = 0x10,
    RESOURCE_STATE_DEPTH_READ = 0x20,
    RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
    RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
    RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
    RESOURCE_STATE_STREAM_OUT = 0x100,
    RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
    RESOURCE_STATE_COPY_DEST = 0x400,
    RESOURCE_STATE_COPY_SOURCE = 0x800,
    RESOURCE_STATE_GENERIC_READ = ( ( ( ( ( 0x1 | 0x2 ) | 0x40 ) | 0x80 ) | 0x200 ) | 0x800 ),
    RESOURCE_STATE_PRESENT = 0x1000,
    RESOURCE_STATE_COMMON = 0x2000,
    RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
    RESOURCE_STATE_SHADING_RATE_SOURCE = 0x8000,
} ResourceState;


enum class TextureType
{
    Texture1D, Texture2D, Texture3D, TextureCube, Texture_1D_Array, Texture_2D_Array, Texture_Cube_Array, Count
};

enum class  ColorWriteEnabled
{
    Red, Green, Blue, Alpha, All, Count
};
enum class  ColorWriteEnabledMask
{
    Red_Mask = 1 << 0,
    Green_Mask = 1 << 1,
    Blue_Mask = 1 << 2,
    Alpha_Mask = 1 << 3,
    All_Mask = Red_Mask | Green_Mask | Blue_Mask | Alpha_Mask
};
}