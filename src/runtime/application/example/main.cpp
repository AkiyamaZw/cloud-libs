#include "graphic_application.h"
#include "app_utility.h"
#include "VkGraphicsBase.h"

int main()
{
    cloud::GraphicsApplication app;
    cloud::graphics::vk::GraphicsBase graphics_device;
    app.Setup();
    app.Run();

    INFO("app finished!");
    return 0;
}