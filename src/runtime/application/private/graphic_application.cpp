#include "graphic_application.h"
#include "core/runtime_log.h"
#include "core/app_utils.h"
#include "core/window/window.h"
#include <chrono>
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
	window::IWindow *window{nullptr};
	std::vector<std::string_view> window_extension;
	bool should_exit{false};
	TimerData frame_ts;
} GAppContext;

void InitWindow(AppContext *context)
{
	context->window = window::CreateWindow();
	if (!context->window)
	{
		FATAL("Create window failed!");
		return;
	}

	window::WindowCreateInfo create_info;
	create_info.width = context->width;
	create_info.height = context->height;
	create_info.title = context->title;

	if (!context->window->Initialize(create_info))
	{
		FATAL("Initialize window failed!");
		window::DestroyWindow(context->window);
		context->window = nullptr;
		return;
	}
}

void UpdateWindow(AppContext *context)
{
	if (context->window)
	{
		context->window->PollEvents();
		context->should_exit = context->window->ShouldClose();
	}
}

void DestoryWindow(AppContext *context)
{
	if (context->window)
	{
		window::DestroyWindow(context->window);
		context->window = nullptr;
	}
}

void InitModules()
{
	InitWindow(&GAppContext);
	GpuCreateParam param{.window = GAppContext.window->GetNativeWindow(),
						 .width = GAppContext.width,
						 .height = GAppContext.height};

	render::CreateRenderer(param);
}

void ShutdownModules()
{
	render::DestroyRenderer();
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
		
		// 开始帧
		render::Renderer::Inst()->BeginFrame();
		
		// 更新窗口
		UpdateWindow(&GAppContext);
		
		// 结束帧（执行渲染）
		render::Renderer::Inst()->EndFrame();
	}
	ShutdownModules();
}

} // namespace cloud
