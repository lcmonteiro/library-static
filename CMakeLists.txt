cmake_minimum_required (VERSION 3.13)

# project
project(static)

# properties
set(CMAKE_CXX_STANDARD 14)

# target
add_library(static INTERFACE)

# target configuration
target_include_directories(static INTERFACE ./include)

# test
enable_testing()
add_subdirectory(test)
