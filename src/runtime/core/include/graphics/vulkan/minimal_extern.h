#pragma  once

// clang-format off
#ifdef WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_INCLUDE_VULKAN
    #include "Windows.h"
    // #include <vulkan/vulkan_win32.h>
    #include "GLFW/glfw3.h"
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include "GLFW/glfw3native.h"
#elif defined(__APPLE__)
    #define GLFW_INCLUDE_VULKAN
    #include "GLFW/glfw3.h"
#endif
// clang-format on