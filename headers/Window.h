#pragma once
#include <unordered_map>
#include <functional>

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

    using framebufferSizeCallback_t = std::function<void(int width, int height)>;
    using keyCallback_t              = std::function<void(int key, int scancode, int action, int mods)>;
    using mouseCallback_t           = std::function<void(double xposIn, double yposIn)>;
    using mouseButtonCallback_t     = std::function<void(int button, int action, int mods)>;
    using scrollCallback_t          = std::function<void(double xoffset, double yoffset)>;

    unsigned int addFramebufferSizeCallback(framebufferSizeCallback_t callback);
    unsigned int addKeyCallback(keyCallback_t callback);
    unsigned int addMouseCallback(mouseCallback_t callback);
    unsigned int addMouseButtonCallback(mouseButtonCallback_t callback);
    unsigned int addScrollCallback(scrollCallback_t callback);

    bool removeCallback(unsigned int id);

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    unsigned int getNextCallbackId();
    void processInput() const;

    GLFWwindow* handle_ = nullptr;
    unsigned int lastCallbackId = 0;
    std::unordered_map<unsigned int, framebufferSizeCallback_t>framebufferSizeCallbacks;
    std::unordered_map<unsigned int, keyCallback_t>keyCallbacks;
    std::unordered_map<unsigned int, mouseCallback_t>mouseCallbacks;
    std::unordered_map<unsigned int, mouseButtonCallback_t>mouseButtonCallbacks;
    std::unordered_map<unsigned int, scrollCallback_t>scrollCallbacks;
};
