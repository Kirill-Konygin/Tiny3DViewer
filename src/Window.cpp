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

    glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (glfwWindow == nullptr)
    {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        exit(1);
    }

    glfwMakeContextCurrent(glfwWindow);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwDestroyWindow(glfwWindow);
        glfwWindow = nullptr;
        glfwTerminate();
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    glEnable(GL_DEPTH_TEST);

    addFramebufferSizeCallback([](int width, int height) {
        glViewport(0, 0, width, height);
    });

    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, framebufferSizeCallback);
    glfwSetKeyCallback(glfwWindow, keyCallback);
    glfwSetCursorPosCallback(glfwWindow, mouseCallback);
    glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
    glfwSetScrollCallback(glfwWindow, scrollCallback);
}

Window::~Window()
{
    if (glfwWindow != nullptr)
        glfwDestroyWindow(glfwWindow);

    glfwTerminate();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(glfwWindow);
}

void Window::update()
{
    glfwSwapBuffers(glfwWindow);
    glfwPollEvents();
    processInput();
}

void Window::clear()
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::getSize(int& width, int& height) const
{
    glfwGetWindowSize(glfwWindow, &width, &height);
}

void Window::getFramebufferSize(int& width, int& height) const
{
    glfwGetFramebufferSize(glfwWindow, &width, &height);
}

void Window::getCursorPosition(double& x, double& y) const
{
    glfwGetCursorPos(glfwWindow, &x, &y);
}

unsigned int Window::addFramebufferSizeCallback(framebufferSizeCallback_t callback)
{
    const unsigned int callbackId = getNextCallbackId();
    framebufferSizeCallbacks.emplace(callbackId, std::move(callback));
    return callbackId;
}

unsigned int Window::addKeyCallback(keyCallback_t callback)
{
    const unsigned int callbackId = getNextCallbackId();
    keyCallbacks.emplace(callbackId, std::move(callback));
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
    return framebufferSizeCallbacks.erase(id) || keyCallbacks.erase(id) || mouseCallbacks.erase(id) || mouseButtonCallbacks.erase(id) || scrollCallbacks.erase(id);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if(Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        for (const auto& [_,callback] : ptr->framebufferSizeCallbacks) {
            callback(width, height);
        }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window)))
        for (const auto& [_, callback] : ptr->keyCallbacks) {
            callback(key, scancode, action, mods);
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
      ||  keyCallbacks.contains(lastCallbackId)
      ||  mouseCallbacks.contains(lastCallbackId)
      ||  mouseButtonCallbacks.contains(lastCallbackId)
      ||  scrollCallbacks.contains(lastCallbackId)
    );

    return lastCallbackId;
}

void Window::processInput() const
{
    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
}
