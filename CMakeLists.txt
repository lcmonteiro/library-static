cmake_minimum_required (VERSION 3.13)

# project
project(library-static)

# properties
set(CMAKE_CXX_STANDARD 14)

# target
add_library(library-static INTERFACE)
# target configuration
target_include_directories(library-static INTERFACE ./include)

# test
enable_testing()
add_subdirectory(test)
