#include "graphic_application.h"

namespace cloud
{
GraphicsApplication::GraphicsApplication() {}

GraphicsApplication::~GraphicsApplication() {}

void GraphicsApplication::Setup()
{
    ApplicationBase::Setup();
    window_.Setup();
    RegisterSlotUpdate(std::bind(&GraphicsWindow::Update, &window_, std::placeholders::_1));
}

void GraphicsApplication::OnTick()
{
    if (window_.ShouldExit())
    {
        RequestEndApplication();
    }
}
} // namespace cloud
