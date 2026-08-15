#include "Camera.h"
#include "Window.h"
#include "Mesh.h"
#include "Material.h"
#include "OrbitalTransform.h"
#include "ShaderFactory.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <nfd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

#include "ModelLoader.h"


void openFileDialog(bool& openModelRequested, ModelLoader& loader, std::optional<Model>& model);

int main()
{
    Window window("Tiny3DViewer");
    Shader shader = ShaderFactory::MakeModelShader();
    Camera camera;
    ModelLoader loader;
    std::optional<Model> model;

    if (NFD_Init() != NFD_OKAY)
    {
        std::cerr << "Failed to initialize native file dialogs: " << NFD_GetError() << '\n';
        return 1;
    }

    struct NfdGuard
    {
        ~NfdGuard()
        {
            NFD_Quit();
        }
    } nfdGuard;

    OrbitalTransform cameraTransform;
    float cameraDistance = 3.0f;
    float cameraLatitude = 0.0f;
    float cameraLongitude = 0.0f;

    cameraTransform.SetCenter(glm::vec3{0.0f, 0.0f, 0.0f});
    cameraTransform.SetDistance(cameraDistance);
    cameraTransform.SetLatitude(cameraLatitude);
    cameraTransform.SetLongitude(cameraLongitude);

    const auto framebufferSizeCallbackId = window.addFramebufferSizeCallback([&camera](int width, int height) {
        if (height > 0)
            camera.SetAspectRatio(static_cast<float>(width) / height);
    });

    bool openModelRequested = false;
    const auto keyCallbackId = window.addKeyCallback(
        [&openModelRequested](int key, int, int action, int) {
            if (key == GLFW_KEY_O && action == GLFW_PRESS)
                openModelRequested = true;
        }
    );

    bool isRotating = false;
    bool firstMouseMove = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    const auto mouseButtonCallbackId = window.addMouseButtonCallback(
        [&](int button, int action, int) {
            if (button != GLFW_MOUSE_BUTTON_LEFT)
                return;

            if (action == GLFW_PRESS)
                isRotating = true;
            else if (action == GLFW_RELEASE)
                isRotating = false;

            firstMouseMove = true;
        }
    );

    const auto mouseCallbackId = window.addMouseCallback(
        [&](double mouseX, double mouseY) {
            if (!isRotating)
                return;

            if (firstMouseMove)
            {
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                firstMouseMove = false;
                return;
            }

            constexpr float rotationSensitivity = 0.005f;

            cameraLongitude -= static_cast<float>(mouseX - lastMouseX) * rotationSensitivity;
            cameraLatitude -= static_cast<float>(mouseY - lastMouseY) * rotationSensitivity;

            lastMouseX = mouseX;
            lastMouseY = mouseY;

            cameraLatitude = std::clamp(
                cameraLatitude,
                glm::radians(-90.0f),
                glm::radians(90.0f)
            );

            cameraTransform.SetLongitude(cameraLongitude);
            cameraTransform.SetLatitude(cameraLatitude);
        }
    );

    const auto scrollCallbackId = window.addScrollCallback(
        [&](double, double yOffset) {
            constexpr float zoomSensitivity = 0.1f;

            cameraDistance *= std::exp(-static_cast<float>(yOffset) * zoomSensitivity);
            cameraDistance = std::clamp(cameraDistance, 0.1f, 1000.0f);
            cameraTransform.SetDistance(cameraDistance);
        }
    );

    while (!window.shouldClose())
    {
        if (openModelRequested)
        {
            openFileDialog(openModelRequested, loader, model);
        }

        window.clear();
        shader.Bind();

        const glm::mat4 viewProjection = camera.GetProjectionMatrix() * glm::inverse(cameraTransform.GetMatrix());
        if (model)
            model->Draw(shader, viewProjection);
        window.update();
    }
    
    window.removeCallback(framebufferSizeCallbackId);
    window.removeCallback(keyCallbackId);
    window.removeCallback(mouseButtonCallbackId);
    window.removeCallback(mouseCallbackId);
    window.removeCallback(scrollCallbackId);

    return 0;
}

void openFileDialog(bool& openModelRequested, ModelLoader& loader, std::optional<Model>& model)
{
    openModelRequested = false;

    const nfdu8filteritem_t filters[] = {
        { "3D models", "3ds,3mf,blend,dae,fbx,glb,gltf,obj,off,ply,stl,x" }
    };

    nfdu8char_t* selectedPath = nullptr;
    nfdopendialogu8args_t dialogArguments{};
    dialogArguments.filterList = filters;
    dialogArguments.filterCount = 1;
    dialogArguments.title = "Open 3D model";

    const nfdresult_t dialogResult = NFD_OpenDialogU8_With(&selectedPath, &dialogArguments);
    if (dialogResult == NFD_OKAY)
    {
        const std::string selectedPathUtf8(selectedPath);
        NFD_FreePathU8(selectedPath);

        const std::filesystem::path modelPath{
            std::u8string(selectedPathUtf8.begin(), selectedPathUtf8.end())
        };

        std::optional<Model> selectedModel = loader.LoadModel(modelPath);
        if (selectedModel)
        {
            if (model)
            {
                selectedModel->setPosition(model->getPosition());
                selectedModel->setRotation(model->getRotation());
                selectedModel->setScale(model->getScale());
            }

            model = std::move(selectedModel);
        }
        else
        {
            std::cerr << "Failed to load model: " << modelPath << '\n';
        }
    }
    else if (dialogResult == NFD_ERROR)
    {
        std::cerr << "Failed to open file dialog: " << NFD_GetError() << '\n';
    }
}
