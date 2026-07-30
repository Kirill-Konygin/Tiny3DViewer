#pragma once

struct GLFWwindow;

class Window
{
public:
    Window(const char* title, int width = 1920, int height = 1080);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose();
    void update();
    void clear();

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    void processInput() const;

    GLFWwindow* handle_ = nullptr;
};
