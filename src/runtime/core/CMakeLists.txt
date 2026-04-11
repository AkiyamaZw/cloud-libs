# Include common build utilities
include(${CMAKE_SOURCE_DIR}/tool/cmake/CloudBuild.cmake)

# Cloud Minicore Object Library
add_object_library(cloud_minicore_obj modules/core
    HEADER_PATTERNS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/*.inc
    SOURCE_PATTERNS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp
    INCLUDES PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    LIBS PRIVATE spdlog glfw
)

# Graphics Object Library
add_object_library(graphics_obj modules/core
    HEADER_PATTERNS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/*.inc
    SOURCE_PATTERNS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/vulkan/*.cpp
    INCLUDES
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${vulkan_include}
            ${EXTERN_LIB_DIR}/glfw/include
    LIBS PRIVATE spdlog ${vulkan_lib} glfw
)

# Core Runtime Library
add_library(core_runtime STATIC
    $<TARGET_OBJECTS:cloud_minicore_obj>
    $<TARGET_OBJECTS:graphics_obj>
)

target_link_libraries(core_runtime
    PUBLIC
        spdlog
        ${vulkan_lib}
        glfw
)

target_include_directories(core_runtime
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(core_runtime PROPERTIES FOLDER modules)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${${PROJECT_NAME}_obj_HEAD_FILES}
    ${graphics_obj_HEAD_FILES}
    ${${PROJECT_NAME}_obj_SOURCE_FILES}
    ${graphics_obj_SOURCE_FILES}
)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)
