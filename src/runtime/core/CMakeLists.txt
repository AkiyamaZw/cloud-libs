set(PROJECT_NAME cloud_minicore)
file(GLOB_RECURSE HEAD_FILES    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/include/*.inc)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}_obj OBJECT ${HEAD_FILES} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME}_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}_obj PRIVATE spdlog)
set_target_properties(${PROJECT_NAME}_obj PROPERTIES FOLDER modules/core)

add_library(core_runtime STATIC
	$<TARGET_OBJECTS:${PROJECT_NAME}_obj>
)

target_link_libraries(
		core_runtime
        PUBLIC
        spdlog
)

target_include_directories(core_runtime
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(core_runtime PROPERTIES FOLDER modules)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEAD_FILES} ${SOURCE_FILES})
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)