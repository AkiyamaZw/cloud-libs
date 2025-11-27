#include "graphics/vulkan/vulkan_interface.h"

namespace cloud::vulkan::infra
{
PFN_vkSetDebugUtilsObjectNameEXT    pfnSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT    pfnCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT      pfnCmdEndDebugUtilsLabelEXT;

void InitVulkanInterface(VkDevice device, bool debug_message)
{
    if (debug_message)
    {
        pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT" );
        pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT" );
        pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT" );
    }
}


void SetResourceName(VkDevice device, VkObjectType type, uint64_t handle, const char *name)
{
    VkDebugUtilsObjectNameInfoEXT name_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    name_info.objectType = type;
    name_info.objectHandle = handle;
    name_info.pObjectName = name;
    pfnSetDebugUtilsObjectNameEXT(device, &name_info);
}

void CreateSampler(VkDevice device, const SamplerCreation& creation, VkSampler& sampler)
{
    VkSamplerCreateInfo create_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    create_info.addressModeU = creation.address_mode_u;
    create_info.addressModeV = creation.address_mode_v;
    create_info.addressModeW = creation.address_mode_w;
    create_info.minFilter = creation.min_filter;
    create_info.magFilter = creation.mag_filter;
    create_info.mipmapMode = creation.mip_filter;
    create_info.anisotropyEnable = 0;
    create_info.compareEnable = 0;
    create_info.unnormalizedCoordinates = 0;
    create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    auto succ = vkCreateSampler(device, &create_info, nullptr, &sampler);
    check_vk(succ);
}
}