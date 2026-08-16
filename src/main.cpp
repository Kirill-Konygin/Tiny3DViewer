#include "AxisDrag.h"
#include "Camera.h"
#include "Window.h"
#include "Mesh.h"
#include "Material.h"
#include "Mover.h"
#include "OrbitalTransform.h"
#include "Picking.h"
#include "ShaderFactory.h"
#include "TranslateGizmo.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <nfd.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "ModelLoader.h"

constexpr float gizmoPixelLength = 120.0f;

struct CursorRay
{
    picking::Ray ray;
    glm::vec2 viewportSize;
};

std::optional<CursorRay> getCursorRay(
    const Window& window,
    const Camera& camera,
    const OrbitalTransform& cameraTransform,
    double mouseX,
    double mouseY)
{
    int windowWidth = 0;
    int windowHeight = 0;
    int framebufferWidth = 0;
    int framebufferHeight = 0;

    window.getSize(windowWidth, windowHeight);
    window.getFramebufferSize(framebufferWidth, framebufferHeight);

    if (windowWidth <= 0 || windowHeight <= 0 || framebufferWidth <= 0 || framebufferHeight <= 0)
        return std::nullopt;

    const glm::vec2 screenPoint{
        static_cast<float>(mouseX * framebufferWidth / windowWidth),
        static_cast<float>(mouseY * framebufferHeight / windowHeight)
    };
    const glm::vec2 viewportSize{
        static_cast<float>(framebufferWidth),
        static_cast<float>(framebufferHeight)
    };
    const glm::mat4 viewProjection =
        camera.GetProjectionMatrix() * glm::inverse(cameraTransform.GetMatrix());

    return CursorRay{
        .ray = picking::screenPointToRay(screenPoint, viewportSize, viewProjection),
        .viewportSize = viewportSize
    };
}

std::array<picking::PickTarget, 3> getArrowPickTargets(
    const TranslateGizmo& gizmo)
{
    const std::span<const TranslateArrow> arrows = gizmo.GetArrows();
    std::array<picking::PickTarget, 3> targets{};

    for (std::size_t index = 0; index < targets.size(); ++index)
    {
        targets[index] = picking::PickTarget{
            .localBounds = arrows[index].GetLocalBounds(),
            .localToWorld = arrows[index].GetTransform()
        };
    }

    return targets;
}

void openFileDialog(bool& openModelRequested, ModelLoader& loader, std::optional<Model>& model);

int main()
{
    Window window("Tiny3DViewer");
    Shader shader = ShaderFactory::MakeModelShader();
    Camera camera;
    ModelLoader loader;
    std::optional<Model> model;
    TranslateGizmo translateGizmo;
    AxisDrag axisDrag;
    Mover mover;

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
    bool modelSelected = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    const auto mouseButtonCallbackId = window.addMouseButtonCallback(
        [&](int button, int action, int) {
            if (button != GLFW_MOUSE_BUTTON_LEFT)
                return;

            if (action == GLFW_RELEASE)
            {
                isRotating = false;
                firstMouseMove = true;
                axisDrag.End();
                mover.End();
                translateGizmo.DeselectAll();
                return;
            }

            if (action != GLFW_PRESS)
                return;

            isRotating = false;
            firstMouseMove = true;

            if (!model)
                return;

            double mouseX = 0.0;
            double mouseY = 0.0;

            window.getCursorPosition(mouseX, mouseY);

            const std::optional<CursorRay> cursorRay = getCursorRay(
                window,
                camera,
                cameraTransform,
                mouseX,
                mouseY);
            if (!cursorRay)
                return;

            translateGizmo.SetPosition(model->getPosition());
            translateGizmo.UpdateScaleForPixelLength(
                glm::inverse(cameraTransform.GetMatrix()),
                camera.GetFOV(),
                cursorRay->viewportSize.y,
                gizmoPixelLength);

            if (modelSelected)
            {
                const std::array<picking::PickTarget, 3> targets =
                    getArrowPickTargets(translateGizmo);
                const std::optional<std::size_t> pickedIndex =
                    picking::pick(cursorRay->ray, targets);

                if (pickedIndex)
                {
                    std::span<TranslateArrow> arrows = translateGizmo.GetArrows();
                    TranslateArrow& arrow = arrows[*pickedIndex];

                    if (axisDrag.Begin(
                            cursorRay->ray,
                            translateGizmo.GetPosition(),
                            arrow.GetDirection()))
                    {
                        std::array<Mover::Target, 2> moveTargets{
                            Mover::Target{
                                .startPosition = model->getPosition(),
                                .setPosition = [&model](const glm::vec3& position) {
                                    if (model)
                                        model->setPosition(position);
                                }
                            },
                            Mover::Target{
                                .startPosition = translateGizmo.GetPosition(),
                                .setPosition = [&translateGizmo](const glm::vec3& position) {
                                    translateGizmo.SetPosition(position);
                                }
                            }
                        };
                        mover.Begin(moveTargets);

                        translateGizmo.DeselectAll();
                        arrow.Select();
                        return;
                    }
                }
            }

            modelSelected = picking::pick(
                cursorRay->ray,
                std::span<Model>{&*model, 1}).has_value();
            isRotating = true;
        }
    );

    const auto mouseCallbackId = window.addMouseCallback(
        [&](double mouseX, double mouseY) {
            if (axisDrag.IsActive() && mover.IsActive())
            {
                const std::optional<CursorRay> cursorRay = getCursorRay(
                    window,
                    camera,
                    cameraTransform,
                    mouseX,
                    mouseY);
                if (!cursorRay)
                    return;

                if (const std::optional<glm::vec3> offset = axisDrag.Update(cursorRay->ray))
                    mover.Move(*offset);
                return;
            }

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

        const glm::mat4 projection = camera.GetProjectionMatrix();
        const glm::mat4 view = glm::inverse(cameraTransform.GetMatrix());
        const glm::mat4 viewProjection = projection * view;
        if (model)
        {
            model->Draw(shader, viewProjection);
            if (modelSelected)
            {
                int framebufferWidth = 0;
                int framebufferHeight = 0;
                window.getFramebufferSize(framebufferWidth, framebufferHeight);

                translateGizmo.SetPosition(model->getPosition());
                translateGizmo.UpdateScaleForPixelLength(
                    view,
                    camera.GetFOV(),
                    static_cast<float>(framebufferHeight),
                    gizmoPixelLength);
                translateGizmo.Draw(viewProjection);
            }
        }
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
