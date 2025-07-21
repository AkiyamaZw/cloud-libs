set(PROJECT_NAME graphics_vk)
file(GLOB_RECURSE HEAD_FILES    ${CMAKE_CURRENT_SOURCE_DIR}/public/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.inc)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp)

set(vulkan_include ${EXTERN_LIB_DIR}/VulkanSDK/include PARENT)

add_library(${PROJECT_NAME} STATIC ${HEAD_FILES} ${SOURCE_FILES})
target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
         ${vulkan_lib}
         cloud_minicore
)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ${CMAKE_CURRENT_SOURCE_DIR} ${vulkan_include}
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER modules/graphics_vk)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEAD_FILES} ${SOURCE_FILES})
