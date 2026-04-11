# CloudBuild.cmake - Common CMake utility functions for cloud-libs project

# Helper function: Glob files from multiple patterns
# Usage: glob_files(output_var pattern1 pattern2 ...)
function(glob_files output_var)
    set(files)
    foreach(pattern ${ARGN})
        file(GLOB_RECURSE matched ${pattern})
        list(APPEND files ${matched})
    endforeach()
    set(${output_var} ${files} PARENT_SCOPE)
endfunction()

# Helper function: Add object library with auto-globbing support
# Usage: add_object_library(target_name folder
#     HEADER_PATTERNS pattern1 pattern2 ...
#     SOURCE_PATTERNS pattern1 pattern2 ...
#     INCLUDES [PUBLIC|PRIVATE|INTERFACE] include_path ...
#     LIBS [PUBLIC|PRIVATE|INTERFACE] lib ...
# )
function(add_object_library target_name folder)
    cmake_parse_arguments(ARG "" "" "HEADER_PATTERNS;SOURCE_PATTERNS;INCLUDES;LIBS" ${ARGN})

    # Glob files if patterns provided
    if(ARG_HEADER_PATTERNS)
        glob_files(ARG_HEADERS ${ARG_HEADER_PATTERNS})
    endif()
    if(ARG_SOURCE_PATTERNS)
        glob_files(ARG_SOURCES ${ARG_SOURCE_PATTERNS})
    endif()

    add_library(${target_name} OBJECT ${ARG_HEADERS} ${ARG_SOURCES})

    if(ARG_INCLUDES)
        target_include_directories(${target_name} ${ARG_INCLUDES})
    endif()

    if(ARG_LIBS)
        target_link_libraries(${target_name} ${ARG_LIBS})
    endif()

    set_target_properties(${target_name} PROPERTIES FOLDER ${folder})

    # Return files for source_group
    set(${target_name}_HEAD_FILES ${ARG_HEADERS} PARENT_SCOPE)
    set(${target_name}_SOURCE_FILES ${ARG_SOURCES} PARENT_SCOPE)
endfunction()
