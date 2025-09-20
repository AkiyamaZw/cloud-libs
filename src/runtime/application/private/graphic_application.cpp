#include "graphic_application.h"
#include "GLFW/glfw3.h"
#include "logger.h"

namespace cloud
{
struct TimerData
{
    float last_update_ts{0.0};
    std::chrono::high_resolution_clock::time_point start_ts;
};

struct AppContext
{
    int width;
    int height;
    std::string title;
    void *window_ptr{nullptr};
    std::vector<std::string_view> window_extension;
    bool should_exit{false};
    TimerData frame_ts;
} GAppContext;

void GLFWErrorUserDefinedCallback(int error, const char *description)
{
    ERROR("[Glfw] error {}, {}", error, description);
}

void InputCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

void CreateWindow(AppContext *context)
{
    glfwSetErrorCallback(GLFWErrorUserDefinedCallback);
    if (!glfwInit())
    {
        ERROR("Initialize GLFW Failed!");
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window_handle =
        glfwCreateWindow(context->width, context->height, context->title.c_str(), nullptr, nullptr);
    if (!window_handle)
    {
        ERROR("[GLFW]window create failed!");
        return;
    }
    // glfwMakeContextCurrent(window_handle);
    glfwSetKeyCallback((GLFWwindow *)window_handle, InputCallback);

    context->window_ptr = window_handle;
#ifdef __WIN32
    extension_count = 2;
    context->window_extension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    context->window_extension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
#elif __APPLE__
    uint32_t extension_count = 0;
    const char **extension_names = nullptr;
    extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
    if (extension_names == nullptr)
    {
        ERROR("[GLFW] cannot get Vulkan Extentions Info!");
        return;
    }
    INFO("[GLFW] need window extensions:");
    for (int i = 0; i < extension_count; i++)
    {
        context->window_extension.push_back(extension_names[i]);
    }
#endif
}

void UpdateWindow(AppContext *context)
{
    glfwPollEvents();
    context->should_exit = glfwWindowShouldClose((GLFWwindow *)context->window_ptr);
}

void DestoryWindow(AppContext *context)
{
    if (context->window_ptr)
    {
        glfwDestroyWindow((GLFWwindow *)context->window_ptr);
        context->window_ptr = nullptr;
        glfwTerminate();
    }
}

void CreateRenderer(AppContext *context) {}

void SetupApp(AppParam *param)
{
    GAppContext.height = param->window_height;
    GAppContext.width = param->window_width;
    GAppContext.title = param->window_title;
    GAppContext.frame_ts.start_ts = std::chrono::high_resolution_clock::now();

    CreateWindow(&GAppContext);
    CreateRenderer(&GAppContext);
    std::chrono::high_resolution_clock::time_point cur_ts;
    while (!GAppContext.should_exit)
    {
        cur_ts = std::chrono::high_resolution_clock::now();
        GAppContext.frame_ts.last_update_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                  cur_ts - GAppContext.frame_ts.start_ts)
                                                  .count();
        GAppContext.frame_ts.start_ts = cur_ts;
        UpdateWindow(&GAppContext);
    }

    DestoryWindow(&GAppContext);
}

} // namespace cloud
