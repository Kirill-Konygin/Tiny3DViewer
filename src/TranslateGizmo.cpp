#include "TranslateGizmo.h"

#include "Mesh.h"
#include "Model.h"
#include "ShaderFactory.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <numbers>
#include <vector>

constexpr float arrowStart = 0.08f;
constexpr float arrowShaftEnd = 0.72f;
constexpr float arrowEnd = 1.0f;
constexpr float arrowShaftHalfWidth = 0.025f;
constexpr float arrowHeadRadius = 0.085f;
constexpr int arrowHeadSegments = 16;
constexpr float minimumWorldScale = 1e-4f;

Vertex makeVertex(const glm::vec3& position)
{
    Vertex vertex{};
    vertex.position = position;
    return vertex;
}

void appendCuboid(std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices)
{
    const std::uint32_t firstVertex = static_cast<std::uint32_t>(vertices.size());
    const float h = arrowShaftHalfWidth;

    const std::array<glm::vec3, 8> positions{
        glm::vec3{arrowStart,    -h, -h},
        glm::vec3{arrowStart,     h, -h},
        glm::vec3{arrowStart,     h,  h},
        glm::vec3{arrowStart,    -h,  h},
        glm::vec3{arrowShaftEnd, -h, -h},
        glm::vec3{arrowShaftEnd,  h, -h},
        glm::vec3{arrowShaftEnd,  h,  h},
        glm::vec3{arrowShaftEnd, -h,  h}
    };

    for (const glm::vec3& position : positions)
        vertices.push_back(makeVertex(position));

    constexpr std::array<std::uint32_t, 36> cuboidIndices{
        0, 2, 1, 0, 3, 2,
        4, 5, 6, 4, 6, 7,
        0, 1, 5, 0, 5, 4,
        3, 7, 6, 3, 6, 2,
        0, 4, 7, 0, 7, 3,
        1, 2, 6, 1, 6, 5
    };

    for (const std::uint32_t index : cuboidIndices)
        indices.push_back(firstVertex + index);
}

void appendCone(std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices)
{
    constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    const std::uint32_t baseCenter = static_cast<std::uint32_t>(vertices.size());
    vertices.push_back(makeVertex({arrowShaftEnd, 0.0f, 0.0f}));

    const std::uint32_t ringStart = static_cast<std::uint32_t>(vertices.size());
    for (int segment = 0; segment < arrowHeadSegments; ++segment)
    {
        const float angle = twoPi * static_cast<float>(segment) / static_cast<float>(arrowHeadSegments);
        vertices.push_back(makeVertex({arrowShaftEnd, std::cos(angle) * arrowHeadRadius, std::sin(angle) * arrowHeadRadius}));
    }

    const std::uint32_t tip = static_cast<std::uint32_t>(vertices.size());
    vertices.push_back(makeVertex({arrowEnd, 0.0f, 0.0f}));

    for (int segment = 0; segment < arrowHeadSegments; ++segment)
    {
        const std::uint32_t current = ringStart + static_cast<std::uint32_t>(segment);
        const std::uint32_t next = ringStart + static_cast<std::uint32_t>((segment + 1) % arrowHeadSegments);

        indices.insert(indices.end(), {baseCenter, next, current});
        indices.insert(indices.end(), {tip, current, next});
    }
}

glm::mat4 axisRotation(GizmoAxis axis)
{
    switch (axis)
    {
    case GizmoAxis::Y:
        return glm::rotate(glm::mat4{1.0f}, glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
    case GizmoAxis::Z:
        return glm::rotate(glm::mat4{1.0f}, glm::radians(-90.0f), glm::vec3{0.0f, 1.0f, 0.0f});
    case GizmoAxis::X:
    default:
        return glm::mat4{1.0f};
    }
}

glm::vec4 axisColor(const TranslateArrow& arrow)
{
    if (arrow.IsSelected())
        return {1.0f, 0.85f, 0.1f, 1.0f};

    switch (arrow.GetAxis())
    {
    case GizmoAxis::X:
        return {0.95f, 0.15f, 0.12f, 1.0f};
    case GizmoAxis::Y:
        return {0.2f, 0.85f, 0.2f, 1.0f};
    case GizmoAxis::Z:
        return {0.15f, 0.35f, 1.0f, 1.0f};
    default:
        return glm::vec4{1.0f};
    }
}

glm::mat4 axisTransform(
    const glm::vec3& position,
    float scale,
    GizmoAxis axis)
{
    return glm::translate(glm::mat4{1.0f}, position)
        * axisRotation(axis)
        * glm::scale(glm::mat4{1.0f}, glm::vec3{scale});
}

TranslateArrow::TranslateArrow(GizmoAxis newAxis)
    : axis(newAxis)
{
}

void TranslateArrow::Select()
{
    selected = true;
}

void TranslateArrow::Deselect()
{
    selected = false;
}

bool TranslateArrow::IsSelected() const
{
    return selected;
}

GizmoAxis TranslateArrow::GetAxis() const
{
    return axis;
}

glm::vec3 TranslateArrow::GetDirection() const
{
    switch (axis)
    {
    case GizmoAxis::X:
        return {1.0f, 0.0f, 0.0f};
    case GizmoAxis::Y:
        return {0.0f, 1.0f, 0.0f};
    case GizmoAxis::Z:
        return {0.0f, 0.0f, 1.0f};
    default:
        return {0.0f, 0.0f, 0.0f};
    }
}

AABB TranslateArrow::GetLocalBounds() const
{
    return AABB{
        .min = {arrowStart, -arrowHeadRadius, -arrowHeadRadius},
        .max = {arrowEnd, arrowHeadRadius, arrowHeadRadius}
    };
}

void TranslateArrow::SetTransform(const glm::mat4& newTransform)
{
    transform = newTransform;
}

const glm::mat4& TranslateArrow::GetTransform() const
{
    return transform;
}

TranslateGizmo::TranslateGizmo()
    : arrows{TranslateArrow{GizmoAxis::X}, TranslateArrow{GizmoAxis::Y}, TranslateArrow{GizmoAxis::Z}},
      shader(ShaderFactory::MakeGizmoShader())
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    appendCuboid(vertices, indices);
    appendCone(vertices, indices);
    arrowMesh = std::make_unique<RenderMesh>(vertices, indices);
    UpdateArrowTransforms();
}

TranslateGizmo::~TranslateGizmo() = default;

void TranslateGizmo::SetPosition(const glm::vec3& newPosition)
{
    position = newPosition;
    UpdateArrowTransforms();
}

const glm::vec3& TranslateGizmo::GetPosition() const
{
    return position;
}

void TranslateGizmo::SetScale(float newScale)
{
    scale = newScale;
    UpdateArrowTransforms();
}

std::span<TranslateArrow> TranslateGizmo::GetArrows()
{
    return arrows;
}

std::span<const TranslateArrow> TranslateGizmo::GetArrows() const
{
    return arrows;
}

void TranslateGizmo::DeselectAll()
{
    for (TranslateArrow& arrow : arrows)
        arrow.Deselect();
}

void TranslateGizmo::Draw(const glm::mat4& viewProjection) const
{
    if (scale <= 0.0f || !arrowMesh)
        return;

    const GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    shader.Bind();

    for (const TranslateArrow& arrow : arrows)
    {
        shader.SetMat4("transformMatrix", viewProjection * arrow.GetTransform());
        shader.SetVec4("color", axisColor(arrow));
        arrowMesh->Draw();
    }

    if (depthTestWasEnabled == GL_TRUE)
        glEnable(GL_DEPTH_TEST);
}

void TranslateGizmo::UpdateScaleForPixelLength(const glm::mat4& viewMatrix, float verticalFieldOfViewDegrees, float viewportHeight, float desiredPixelLength)
{
    if (viewportHeight <= 0.0f || desiredPixelLength <= 0.0f)
    {
        SetScale(0.0f);
        return;
    }

    const glm::vec3 viewSpacePosition{viewMatrix * glm::vec4{position, 1.0f}};
    const float viewDepth = std::abs(viewSpacePosition.z);
    const float halfFieldOfViewRadians = glm::radians(verticalFieldOfViewDegrees) * 0.5f;

    const float visibleWorldHeight = 2.0f * viewDepth * std::tan(halfFieldOfViewRadians);
    const float worldUnitsPerPixel = visibleWorldHeight / viewportHeight;

    SetScale(std::max(desiredPixelLength * worldUnitsPerPixel, minimumWorldScale));
}

void TranslateGizmo::UpdateArrowTransforms()
{
    for (TranslateArrow& arrow : arrows)
        arrow.SetTransform(axisTransform(position,scale,arrow.GetAxis()));
}
