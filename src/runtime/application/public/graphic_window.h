#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <string_view>

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
    bool Setup();
    bool ShouldExit() const;
    void Update(float dt);

    std::tuple<int, int> GetWindowSize() const;
    const std::vector<std::string_view> &GetWindowExtensions() const;

  private:
    int width_;
    int height_;
    std::string title_;
    GLFWwindow *window_ptr_;
    std::vector<std::string_view> window_extension_;
};
} // namespace cloud