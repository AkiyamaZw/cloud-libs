#include "graphic_application.h"
#include "logger.h"
#include "VkGraphicsBase.h"

int main()
{
    cloud::AppParam app_param{
        .window_width = 600, .window_height = 400, .window_title = "hello window"};

    SetupApp(&app_param);

    INFO("app finished!");
    return 0;
}