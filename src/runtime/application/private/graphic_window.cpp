#include "graphic_window.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "app_utility.h"

namespace cloud
{
GraphicsWindow::GraphicsWindow(int width, int height, const std::string &title)
    : width_(width)
    , height_(height)
    , title_(title)
    , window_ptr_(nullptr)
{
}

GraphicsWindow::~GraphicsWindow()
{
    glfwDestroyWindow(window_ptr_);
    window_ptr_ = nullptr;
    glfwTerminate();
}

void GLFWErrorUserDefinedCallback(int error, const char *description)
{
    ERROR("[Glfw] error {}, {}", error, description);
}

bool GraphicsWindow::Setup()
{
    glfwSetErrorCallback(GLFWErrorUserDefinedCallback);
    if (!glfwInit())
    {
        ERROR("Initialize GLFW Failed! ");
        return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, false);
    window_ptr_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!window_ptr_)
    {
        ERROR("[GLFW]window create failed!");
        return false;
    }
    uint32_t extension_count = 0;
    const char **extension_names = nullptr;
    extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
    if (extension_names == nullptr)
    {
        ERROR("[GLFW] cannot get Vulkan Extentions Info!");
        return false;
    }
    INFO("[GLFW] need window extensions:");
    for (int i = 0; i < extension_count; i++)
    {
        window_extension_.push_back(extension_names[i]);
        INFO("Window EXTENSION: {}", window_extension_.back());
    }
    return true;
}

bool GraphicsWindow::ShouldExit() const
{
    return window_ptr_ == nullptr || glfwWindowShouldClose(window_ptr_);
}

void GraphicsWindow::Update(float dt) { glfwPollEvents(); }
} // namespace cloud
