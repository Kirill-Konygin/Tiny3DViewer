#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

Window::Window(const char* title, int width, int height)
{
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    handle_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (handle_ == nullptr)
    {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        exit(1);
    }

    glfwMakeContextCurrent(handle_);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwDestroyWindow(handle_);
        handle_ = nullptr;
        glfwTerminate();
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    glfwSetFramebufferSizeCallback(handle_, framebufferSizeCallback);

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(handle_, &framebufferWidth, &framebufferHeight);
    framebufferSizeCallback(handle_, framebufferWidth, framebufferHeight);
}

Window::~Window()
{
    if (handle_ != nullptr)
        glfwDestroyWindow(handle_);

    glfwTerminate();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(handle_);
}

void Window::update()
{
    glfwSwapBuffers(handle_);
    glfwPollEvents();
    processInput();
}

void Window::clear()
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::processInput() const
{
    if (glfwGetKey(handle_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(handle_, GLFW_TRUE);
}
