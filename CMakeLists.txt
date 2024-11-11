cmake_minimum_required(VERSION 3.10)
project(learn-opengl)

# Explicitly set the compiler
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# More explicit compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Add glad library
add_library(glad STATIC
    glad/src/glad.c
)

target_include_directories(glad PUBLIC
    glad/include
)

# Add executable
add_executable(learn-opengl learn-opengl.cpp)

# Link libraries
target_link_libraries(learn-opengl
    PRIVATE
    glad
    "-framework OpenGL"
    "-framework CoreVideo"
    "-framework IOKit"
    "-framework Cocoa"
    "-framework Carbon"
    glfw
)

target_include_directories(learn-opengl
    PRIVATE
    glad/include
    ${CMAKE_SOURCE_DIR}  # This will allow including stb_image.h from the source directory
)
