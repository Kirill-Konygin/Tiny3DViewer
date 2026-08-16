#pragma once

#include "AxisDrag.h"
#include "Camera.h"
#include "ModelLoader.h"
#include "Mover.h"
#include "OrbitalTransform.h"
#include "Picking.h"
#include "Shader.h"
#include "TranslateGizmo.h"
#include "Window.h"

#include <array>
#include <optional>

class ViewerApplication
{
public:
    ViewerApplication();

    int Run();

private:
    struct CursorRay
    {
        picking::Ray ray;
        glm::vec2 viewportSize;
    };

    void RegisterCallbacks();
    void HandleMouseButton(int button, int action);
    void HandleMouseMove(double mouseX, double mouseY);
    void HandleScroll(double yOffset);

    std::optional<CursorRay> GetCursorRay(double mouseX, double mouseY) const;
    std::array<picking::PickTarget, 3> GetArrowPickTargets() const;
    bool TryBeginAxisDrag(const CursorRay& cursorRay);

    void OpenModel();
    void RenderFrame();

    Window window;
    Shader modelShader;
    Camera camera;
    ModelLoader modelLoader;
    std::optional<Model> model;
    TranslateGizmo translateGizmo;
    AxisDrag axisDrag;
    Mover mover;

    OrbitalTransform cameraTransform;
    float cameraDistance = 3.0f;
    float cameraLatitude = 0.0f;
    float cameraLongitude = 0.0f;

    bool openModelRequested = false;
    bool isRotating = false;
    bool firstMouseMove = true;
    bool modelSelected = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};
