#pragma once

#include <string_view>

#include "graphics/core/gpu_enum.h"

namespace cloud::render
{
struct SamplerCreation
{
    std::string_view name;
    FilterMode min_filter;
    FilterMode mag_filter;
    FilterMode mip_filter;
    AddressMode address_mode_u;
    AddressMode address_mode_v;
    AddressMode address_mode_w;
    ReductionMode reduction_mode;
};
}