#include "graphic_window.h"
#include "GLFW/glfw3.h"

namespace cloud
{
GraphicsWindow::GraphicsWindow(int width, int height, const std::string &title)
    : width_(width)
    , height_(height)
    , title_(title)
{
    glfwInit();
    glfwInitHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwInitHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ptr_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

GraphicsWindow::~GraphicsWindow()
{
    glfwDestroyWindow(window_ptr_);
    glfwTerminate();
}

bool GraphicsWindow::ShouldExit() const
{
    return window_ptr_ == nullptr || glfwWindowShouldClose(window_ptr_);
}

void GraphicsWindow::Update(float dt) { glfwPollEvents(); }
} // namespace cloud
