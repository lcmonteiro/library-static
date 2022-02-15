add_executable(overview overview.cpp)
target_link_libraries(overview PRIVATE library-static)
add_test(NAME overview COMMAND overview)
