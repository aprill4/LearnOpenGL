cmake_minimum_required(VERSION 3.10)
project(opengl-projects) # 孩补补母鸡风补补改糯

# Explicitly set the compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# More explicit compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add glad library
add_library(glad STATIC
    glad/src/glad.c
)


target_include_directories(glad PUBLIC
    glad/include
)

set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_STATIC_LIB ON)
set(ASSIMP_BUILD_SHARED_LIB OFF)
add_subdirectory(assimp)

add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Common function to set up an OpenGL project
function(setup_opengl_project PROJECT_NAME SOURCE_FILE)
    add_executable(${PROJECT_NAME} ${SOURCE_FILE})
    
    target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/glad/include
        ${CMAKE_SOURCE_DIR}/assimp/include
        ${CMAKE_SOURCE_DIR} # This will allow including stb_image.h from the source directory
        ${CMAKE_SOURCE_DIR}/glm  # Add this line to include GLM headers
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        glad
        assimp
        "-framework OpenGL"
        "-framework CoreVideo"
        "-framework IOKit"
        "-framework Cocoa"
        "-framework Carbon"
        glfw
    )
endfunction()

# Add subdirectories for each project
add_subdirectory(1-getting-started)
add_subdirectory(2-lighting)
add_subdirectory(3-model-loading)
