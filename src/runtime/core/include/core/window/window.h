#ifndef CLOUD_WINDOW_H
#define CLOUD_WINDOW_H

#include <string>
#include <vector>

namespace cloud
{
namespace window
{

struct WindowCreateInfo
{
    int width{800};
    int height{600};
    std::string title{"Cloud Application"};
    bool fullscreen{false};
};

class IWindow
{
public:
    virtual ~IWindow() = default;

    virtual bool Initialize(const WindowCreateInfo& create_info) = 0;
    virtual void Shutdown() = 0;

    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;

    virtual void* GetNativeWindow() const = 0;
    virtual std::vector<const char*> GetRequiredExtensions() const = 0;

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
};

IWindow* CreateWindow();
void DestroyWindow(IWindow* window);

} // namespace window
} // namespace cloud

#endif // CLOUD_WINDOW_H