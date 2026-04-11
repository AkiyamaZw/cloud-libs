#include "core/window/window.h"
#include "core/runtime_log.h"
#include "GLFW/glfw3.h"

namespace cloud
{
namespace window
{

class GlfwWindow : public IWindow
{
  public:
	GlfwWindow()
		: m_window(nullptr)
		, m_width(0)
		, m_height(0)
	{
	}
	~GlfwWindow() override { Shutdown(); }

	bool Initialize(const WindowCreateInfo &create_info) override
	{
		m_width = create_info.width;
		m_height = create_info.height;

		glfwSetErrorCallback(GlfwErrorCallback);
		if (!glfwInit())
		{
			FATAL("Initialize GLFW Failed!");
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = glfwCreateWindow(m_width,
									m_height,
									create_info.title.c_str(),
									create_info.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
									nullptr);
		if (!m_window)
		{
			FATAL("[GLFW]window create failed!");
			glfwTerminate();
			return false;
		}

		if (!glfwVulkanSupported())
		{
			FATAL("[GLFW] Vulkan not suppported!");
			glfwDestroyWindow(m_window);
			glfwTerminate();
			return false;
		}

		glfwSetKeyCallback(m_window, KeyCallback);
		return true;
	}

	void Shutdown() override
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);
			m_window = nullptr;
		}
		glfwTerminate();
	}

	void PollEvents() override { glfwPollEvents(); }

	bool ShouldClose() const override { return glfwWindowShouldClose(m_window); }

	void *GetNativeWindow() const override { return m_window; }

	std::vector<const char *> GetRequiredExtensions() const override
	{
		uint32_t extension_count = 0;
		const char **extensions = glfwGetRequiredInstanceExtensions(&extension_count);
		return std::vector<const char *>(extensions, extensions + extension_count);
	}

	int GetWidth() const override { return m_width; }
	int GetHeight() const override { return m_height; }

  private:
	static void GlfwErrorCallback(int error, const char *description)
	{ FATAL("[Glfw] error %d, %s", error, description); }

	static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
	}

	GLFWwindow *m_window;
	int m_width;
	int m_height;
};

IWindow *CreateWindow() { return new GlfwWindow(); }

void DestroyWindow(IWindow *window)
{
	if (window)
	{
		delete window;
	}
}

} // namespace window
} // namespace cloud