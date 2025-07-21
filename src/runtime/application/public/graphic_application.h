#pragma once
#include "application_base.h"
#include "graphic_window.h"
#include "VkGraphicsBase.h"

namespace cloud
{
class GraphicsApplication : public ApplicationBase
{
  public:
    GraphicsApplication();
    ~GraphicsApplication();
    void Setup() override;

  protected:
    void OnTick() override;
    void Exit() override;

  private:
    static constexpr int WIDTH = 600;
    static constexpr int HEIGHT = 400;
    GraphicsWindow window_{WIDTH, HEIGHT, "demo"};
    cloud::graphics::vk::GraphicsBase graphics_base_;
};
} // namespace cloud