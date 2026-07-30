cmake_minimum_required(VERSION 3.20)

project(Tiny3DViewer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GLFW is included in the repository, so only build the library itself.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)

add_library(glad STATIC
    libs/glad/src/glad.c
)

target_include_directories(glad
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include"
)

find_package(OpenGL REQUIRED)

add_executable(Tiny3DViewer
    src/main.cpp
    src/Window.cpp
    headers/Window.h
)

target_include_directories(Tiny3DViewer
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/headers"
)

target_link_libraries(Tiny3DViewer
    PRIVATE
        glad
        glfw
        OpenGL::GL
)

if(MSVC)
    target_compile_options(Tiny3DViewer PRIVATE /W4 /permissive-)
else()
    target_compile_options(Tiny3DViewer PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Tiny3DViewer)
