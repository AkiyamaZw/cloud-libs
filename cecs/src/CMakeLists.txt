file(GLOB_RECURSE HEAD_FILES ${CMAKE_CURRENT_SOURCE_DIR}/public/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.inc)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/public/*.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp)

add_library(CloudECS STATIC ${HEAD_FILES} ${SOURCE_FILES})
target_include_directories(CloudECS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public
                                    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
