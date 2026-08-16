cmake_minimum_required(VERSION 3.20)

project(Tiny3DViewer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build bundled dependencies as static libraries unless explicitly overridden.
option(BUILD_SHARED_LIBS "Build dependencies as shared libraries" OFF)

# GLFW is included in the repository, so only build the library itself.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)

# GLM is a header-only mathematics library.
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glm)

# Assimp is included in the repository. Only build the library required by the
# viewer; its tests, tools, samples, and install rules are not part of this app.
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/assimp)

# Native File Dialog Extended is bundled with the project. Build only the
# library; its tests and install rules are not needed by the viewer.
set(NFD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_BUILD_SDL2_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_BUILD_GLFW3_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/nativefiledialog-extended)

add_library(glad STATIC
    libs/glad/src/glad.c
)

target_include_directories(glad
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include"
)

find_package(OpenGL REQUIRED)

add_executable(Tiny3DViewer
    src/AxisDrag.cpp
    src/Camera.cpp
    src/main.cpp
    src/Material.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/ModelLoader.cpp
    src/Mover.cpp
    src/OrbitalTransform.cpp
    src/Picking.cpp
    src/Shader.cpp
    src/ShaderFactory.cpp
    src/Texture.cpp
    src/TranslateGizmo.cpp
    src/ViewerApplication.cpp
    src/Window.cpp
    headers/AxisDrag.h
    headers/Camera.h
    headers/Material.h
    headers/Mesh.h
    headers/Model.h
    headers/ModelLoader.h
    headers/Mover.h
    headers/OrbitalTransform.h
    headers/Picking.h
    headers/RenderObject.h
    headers/Shader.h
    headers/ShaderFactory.h
    headers/Texture.h
    headers/TranslateGizmo.h
    headers/ViewerApplication.h
    headers/Window.h
)

target_include_directories(Tiny3DViewer
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/headers"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb"
)

target_link_libraries(Tiny3DViewer
    PRIVATE
        glad
        glfw
        glm::glm
        assimp::assimp
        nfd::nfd
        OpenGL::GL
)

if(MSVC)
    target_compile_options(Tiny3DViewer PRIVATE /W4 /permissive-)
else()
    target_compile_options(Tiny3DViewer PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Tiny3DViewer)
