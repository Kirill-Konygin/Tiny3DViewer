#include "ViewerApplication.h"

#include "ShaderFactory.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <nfd.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <utility>

namespace {

constexpr float gizmoPixelLength = 120.0f;
constexpr float rotationSensitivity = 0.005f;
constexpr float zoomSensitivity = 0.1f;
constexpr float minimumCameraDistance = 0.1f;
constexpr float maximumCameraDistance = 1000.0f;

struct NfdGuard
{
    ~NfdGuard()
    {
        NFD_Quit();
    }
};

}

ViewerApplication::ViewerApplication()
    : window("Tiny3DViewer"),
      modelShader(ShaderFactory::MakeModelShader())
{
    cameraTransform.SetCenter(glm::vec3{0.0f});
    cameraTransform.SetDistance(cameraDistance);
    cameraTransform.SetLatitude(cameraLatitude);
    cameraTransform.SetLongitude(cameraLongitude);

    RegisterCallbacks();
}

int ViewerApplication::Run()
{
    if (NFD_Init() != NFD_OKAY)
    {
        std::cerr << "Failed to initialize native file dialogs: " << NFD_GetError() << '\n';
        return 1;
    }

    [[maybe_unused]] NfdGuard nfdGuard;

    while (!window.shouldClose())
    {
        if (openModelRequested)
            OpenModel();

        RenderFrame();
    }

    return 0;
}

void ViewerApplication::RegisterCallbacks()
{
    window.addFramebufferSizeCallback([this](int width, int height) {
        if (height > 0)
            camera.SetAspectRatio(static_cast<float>(width) / height);
    });

    window.addKeyCallback([this](int key, int, int action, int) {
        if (key == GLFW_KEY_O && action == GLFW_PRESS)
            openModelRequested = true;
    });

    window.addMouseButtonCallback([this](int button, int action, int) {
        HandleMouseButton(button, action);
    });

    window.addMouseCallback([this](double mouseX, double mouseY) {
        HandleMouseMove(mouseX, mouseY);
    });

    window.addScrollCallback([this](double, double yOffset) {
        HandleScroll(yOffset);
    });
}

void ViewerApplication::HandleMouseButton(int button, int action)
{
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

    const std::optional<CursorRay> cursorRay = GetCursorRay(mouseX, mouseY);
    if (!cursorRay)
        return;

    translateGizmo.SetPosition(model->getPosition());
    translateGizmo.UpdateScaleForPixelLength(
        glm::inverse(cameraTransform.GetMatrix()),
        camera.GetFOV(),
        cursorRay->viewportSize.y,
        gizmoPixelLength);

    if (modelSelected && TryBeginAxisDrag(*cursorRay))
        return;

    modelSelected = picking::pick(
        cursorRay->ray,
        std::span<Model>{&*model, 1}).has_value();
    isRotating = true;
}

void ViewerApplication::HandleMouseMove(double mouseX, double mouseY)
{
    if (axisDrag.IsActive() && mover.IsActive())
    {
        const std::optional<CursorRay> cursorRay = GetCursorRay(mouseX, mouseY);
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

    cameraLongitude -= static_cast<float>(mouseX - lastMouseX) * rotationSensitivity;
    cameraLatitude -= static_cast<float>(mouseY - lastMouseY) * rotationSensitivity;

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    cameraLatitude = std::clamp(
        cameraLatitude,
        glm::radians(-90.0f),
        glm::radians(90.0f));

    cameraTransform.SetLongitude(cameraLongitude);
    cameraTransform.SetLatitude(cameraLatitude);
}

void ViewerApplication::HandleScroll(double yOffset)
{
    cameraDistance *= std::exp(-static_cast<float>(yOffset) * zoomSensitivity);
    cameraDistance = std::clamp(
        cameraDistance,
        minimumCameraDistance,
        maximumCameraDistance);
    cameraTransform.SetDistance(cameraDistance);
}

std::optional<ViewerApplication::CursorRay> ViewerApplication::GetCursorRay(
    double mouseX,
    double mouseY) const
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

std::array<picking::PickTarget, 3> ViewerApplication::GetArrowPickTargets() const
{
    const std::span<const TranslateArrow> arrows = translateGizmo.GetArrows();
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

bool ViewerApplication::TryBeginAxisDrag(const CursorRay& cursorRay)
{
    const std::array<picking::PickTarget, 3> targets = GetArrowPickTargets();
    const std::optional<std::size_t> pickedIndex = picking::pick(cursorRay.ray, targets);
    if (!pickedIndex)
        return false;

    std::span<TranslateArrow> arrows = translateGizmo.GetArrows();
    TranslateArrow& arrow = arrows[*pickedIndex];
    if (!axisDrag.Begin(
            cursorRay.ray,
            translateGizmo.GetPosition(),
            arrow.GetDirection()))
    {
        return false;
    }

    const std::array<Mover::Target, 2> moveTargets{
        Mover::Target{
            .startPosition = model->getPosition(),
            .setPosition = [this](const glm::vec3& position) {
                if (model)
                    model->setPosition(position);
            }
        },
        Mover::Target{
            .startPosition = translateGizmo.GetPosition(),
            .setPosition = [this](const glm::vec3& position) {
                translateGizmo.SetPosition(position);
            }
        }
    };
    mover.Begin(moveTargets);

    translateGizmo.DeselectAll();
    arrow.Select();
    return true;
}

void ViewerApplication::OpenModel()
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

        std::optional<Model> selectedModel = modelLoader.LoadModel(modelPath);
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

void ViewerApplication::RenderFrame()
{
    window.clear();
    modelShader.Bind();

    const glm::mat4 projection = camera.GetProjectionMatrix();
    const glm::mat4 view = glm::inverse(cameraTransform.GetMatrix());
    const glm::mat4 viewProjection = projection * view;
    if (model)
    {
        model->Draw(modelShader, viewProjection);
        if (modelSelected)
        {
            int framebufferWidth = 0;
            int framebufferHeight = 0;
            window.getFramebufferSize(framebufferWidth, framebufferHeight);

            translateGizmo.SetPosition(model->getPosition());
            translateGizmo.UpdateScaleForPixelLength(view,camera.GetFOV(),static_cast<float>(framebufferHeight),gizmoPixelLength);
            translateGizmo.Draw(viewProjection);
        }
    }

    window.update();
}
