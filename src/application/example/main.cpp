#include "graphic_application.h"
#include <iostream>
int main()
{
    cloud::GraphicsApplication app;
    app.Setup();
    app.Run();
    std::cout << "app finished!" << std::endl;
    return 0;
}