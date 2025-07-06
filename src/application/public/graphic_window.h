#pragma once
#include <string>
#include <tuple>

struct GLFWwindow;

namespace cloud
{
class GraphicsWindow
{
  public:
    GraphicsWindow(int width, int height, const std::string &title);
    ~GraphicsWindow();

    GraphicsWindow(const GraphicsWindow &) = delete;
    GraphicsWindow(GraphicsWindow &&) = delete;
    GraphicsWindow &operator=(const GraphicsWindow &) = delete;
    GraphicsWindow &operator=(GraphicsWindow &&) = delete;
    bool ShouldExit() const;
    void Update(float dt);

    std::tuple<int, int> GetWindowSize() const;

  private:
    int width_;
    int height_;
    std::string title_;
    GLFWwindow *window_ptr_;
};
} // namespace cloud