#include "graphic_application.h"

namespace cloud
{
GraphicsApplication::GraphicsApplication() {}

GraphicsApplication::~GraphicsApplication() {}

void GraphicsApplication::Run()
{
    while (!window_.ShouldExit())
    {
        window_.Update(0.0f);
    }
}
} // namespace cloud
