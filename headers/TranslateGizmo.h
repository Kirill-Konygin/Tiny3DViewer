#pragma once

#include "Shader.h"

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <span>

struct AABB;
class RenderMesh;

enum class GizmoAxis
{
    X,
    Y,
    Z
};

class TranslateArrow
{
public:
    explicit TranslateArrow(GizmoAxis axis);

    void Select();
    void Deselect();
    bool IsSelected() const;

    GizmoAxis GetAxis() const;
    glm::vec3 GetDirection() const;

    AABB GetLocalBounds() const;
    const glm::mat4& GetTransform() const;

private:
    friend class TranslateGizmo;

    void SetTransform(const glm::mat4& transform);

    GizmoAxis axis;
    glm::mat4 transform{1.0f};
    bool selected = false;
};

class TranslateGizmo
{
public:
    TranslateGizmo();

    TranslateGizmo(const TranslateGizmo&) = delete;
    TranslateGizmo& operator=(const TranslateGizmo&) = delete;
    TranslateGizmo(TranslateGizmo&&) = delete;
    TranslateGizmo& operator=(TranslateGizmo&&) = delete;
    ~TranslateGizmo();

    void SetPosition(const glm::vec3& position);
    const glm::vec3& GetPosition() const;

    void SetScale(float scale);

    std::span<TranslateArrow> GetArrows();
    std::span<const TranslateArrow> GetArrows() const;

    void DeselectAll();
    void Draw(const glm::mat4& viewProjection) const;
    void UpdateScaleForPixelLength(const glm::mat4& viewMatrix, float verticalFieldOfViewDegrees, float viewportHeight, float desiredPixelLength);

private:
    void UpdateArrowTransforms();

    glm::vec3 position{0.0f};
    float scale = 1.0f;
    std::array<TranslateArrow, 3> arrows;
    Shader shader;
    std::unique_ptr<RenderMesh> arrowMesh;
};
