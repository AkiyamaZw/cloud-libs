#include "graphic_application.h"
#include "runtime_log.h"

#ifndef NEO_GLFW_INCLUDE
#define NEO_GLFW_INCLUDE
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#elif defined(__APPLE__)
// #define VK_USE_PLATFORM_MAXOS_MVK
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
// #define GLFW_EXPOSE_NATIVE_COCOA
// #include "GLFW/glfw3native.h"
#endif
#endif
#include "graphics/renderer.h"

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

void InitWindow(AppContext *context)
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
	if (!glfwVulkanSupported())
	{
		ERROR("[GLFW] Vulkan not suppported!");
		return;
	}
	// glfwMakeContextCurrent(window_handle);
	glfwSetKeyCallback((GLFWwindow *)window_handle, InputCallback);

	context->window_ptr = window_handle;
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

void InitModules()
{
	InitWindow(&GAppContext);
	GpuCreateParam param{
		.window = GAppContext.window_ptr, .width = GAppContext.width, .height = GAppContext.height};
	CreateRenderer(param);
}

void ShutdownModules()
{
	DestroyRenderer();
	DestoryWindow(&GAppContext);
}

void SetupApp(AppParam *param)
{
	Utility::AppEnvInit();
	GAppContext.height = param->window_height;
	GAppContext.width = param->window_width;
	GAppContext.title = param->window_title;
	GAppContext.frame_ts.start_ts = std::chrono::high_resolution_clock::now();

	InitModules();
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
	ShutdownModules();
}

} // namespace cloud
