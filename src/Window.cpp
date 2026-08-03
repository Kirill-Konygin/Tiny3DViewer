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

    addFramebufferSizeCallback([](int width, int height) {
        glViewport(0, 0, width, height);
    });

    glfwSetWindowUserPointer(handle_, this);
    glfwSetFramebufferSizeCallback(handle_, framebufferSizeCallback);
    glfwSetCursorPosCallback(handle_, mouseCallback);
    glfwSetMouseButtonCallback(handle_, mouseButtonCallback);
    glfwSetScrollCallback(handle_, scrollCallback);
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

unsigned int Window::addFramebufferSizeCallback(framebufferSizeCallback_t callback)
{
    const unsigned int callbackId = getNextCallbackId();
    framebufferSizeCallbacks.emplace(callbackId, std::move(callback));
    return callbackId;
}

unsigned int Window::addMouseCallback(mouseCallback_t callback)
{
    const unsigned int callbackId = getNextCallbackId();
    mouseCallbacks.emplace(callbackId, std::move(callback));
    return callbackId;
}

unsigned int Window::addMouseButtonCallback(mouseButtonCallback_t callback)
{
    const unsigned int callbackId = getNextCallbackId();
    mouseButtonCallbacks.emplace(callbackId, std::move(callback));
    return callbackId;
}

unsigned int Window::addScrollCallback(scrollCallback_t callback)
{
    const unsigned int callbackId = getNextCallbackId();
    scrollCallbacks.emplace(callbackId, std::move(callback));
    return callbackId;
}

bool Window::removeCallback(unsigned int id)
{
    return framebufferSizeCallbacks.erase(id) || mouseCallbacks.erase(id) || mouseButtonCallbacks.erase(id) || scrollCallbacks.erase(id);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if(Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        for (const auto& [_,callback] : ptr->framebufferSizeCallbacks) {
            callback(width, height);
        }
}

void Window::mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        for (const auto& [_, callback] : ptr->mouseCallbacks) {
            callback(xposIn, yposIn);
        }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        for (const auto& [_, callback] : ptr->mouseButtonCallbacks) {
            callback(button, action, mods);
        }
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        for (const auto& [_, callback] : ptr->scrollCallbacks) {
            callback(xoffset, yoffset);
        }
}

unsigned int Window::getNextCallbackId()
{
    do
    {
        lastCallbackId++;
    }
    while (
          framebufferSizeCallbacks.contains(lastCallbackId)
      ||  mouseCallbacks.contains(lastCallbackId)
      ||  mouseButtonCallbacks.contains(lastCallbackId)
      ||  scrollCallbacks.contains(lastCallbackId)
    );

    return lastCallbackId;
}

void Window::processInput() const
{
    if (glfwGetKey(handle_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(handle_, GLFW_TRUE);
}
