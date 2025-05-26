#pragma once
#include "graphic_window.h"

namespace cloud
{
class GraphicsApplication
{
  public:
    GraphicsApplication();
    ~GraphicsApplication();

    void Run();

  private:
    static constexpr int WIDTH = 600;
    static constexpr int HEIGHT = 400;
    GraphicsWindow window_{WIDTH, HEIGHT, "demo"};
};
} // namespace cloud