cmake_minimum_required (VERSION 3.30)

# set project name and version
project ("cloud-libs" VERSION 0.2)

# set cpp standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set output folders
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cecs)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cjob)