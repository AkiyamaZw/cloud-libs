set(PROJECT_NAME application)
file(GLOB_RECURSE HEAD_FILES    ${CMAKE_CURRENT_SOURCE_DIR}/public/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.inc)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp)

add_library(${PROJECT_NAME} STATIC ${HEAD_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        cloud_minicore
		PRIVATE
        ${vulkan_lib}
		glfw
)
MESSAGE(STATUS ${vulkan_lib})
target_include_directories(${PROJECT_NAME}
        PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/public
        PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${vulkan_include})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER modules/application)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEAD_FILES} ${SOURCE_FILES})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)