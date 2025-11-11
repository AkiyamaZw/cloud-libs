#pragma once
#include "graphics/device.h"

namespace cloud
{
struct GpuDevice;

class Renderer
{
public:
    static Renderer* Inst();
};
void CreateRenderer(GpuCreateParam &param);
void DestroyRenderer();


} // namespace cloud