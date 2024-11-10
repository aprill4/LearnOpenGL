cmake_minimum_required(VERSION 3.10)
project(hello_triangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add glad library
add_library(glad STATIC
    glad/src/glad.c
)

target_include_directories(glad PUBLIC
    glad/include
)

# Add executable
add_executable(hello_triangle hello_triangle.cpp)

# Link libraries
target_link_libraries(hello_triangle
    PRIVATE
    glad
    "-framework OpenGL"
    "-framework CoreVideo"
    "-framework IOKit"
    "-framework Cocoa"
    "-framework Carbon"
    glfw
)

target_include_directories(hello_triangle
    PRIVATE
    glad/include
)
