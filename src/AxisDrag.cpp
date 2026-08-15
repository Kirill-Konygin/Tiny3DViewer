#include "AxisDrag.h"
#include "Picking.h"
#include <cmath>

bool AxisDrag::Begin(const picking::Ray& ray, const glm::vec3& newOrigin, const glm::vec3& newDirection)
{
    End();

    const float directionLength = glm::length(newDirection);
    if (directionLength <= 1e-6f)
        return false;

    const glm::vec3 normalizedDirection = glm::normalize(newDirection);
    const std::optional<float> axisParameter = GetAxisParameter(ray,newOrigin,normalizedDirection);
    if (!axisParameter)
        return false;

    origin = newOrigin;
    direction = normalizedDirection;
    startAxisParameter = *axisParameter;
    active = true;
    return true;
}

std::optional<glm::vec3> AxisDrag::Update(const picking::Ray& ray) const
{
    if (!active)
        return std::nullopt;

    const std::optional<float> axisParameter = GetAxisParameter(ray, origin, direction);
    if (!axisParameter)
        return std::nullopt;

    const float distance = *axisParameter - startAxisParameter;
    return direction * distance;
}

void AxisDrag::End()
{
    active = false;
}

bool AxisDrag::IsActive() const
{
    return active;
}

// Uses the closest-point parameter formula
//  t = dot(p1 - p2, cross(d1, n)) / dot(d2, cross(d1, n)),
// where p1 = ray.origin, p2 = axisOrigin, d1 = ray.direction,
// d2 = axisDirection, and n = cross(d1, d2).
// Applying the vector triple-product identity
//  a x (b x c) = b * dot(a, c) - c * dot(a, b)
std::optional<float> AxisDrag::GetAxisParameter(const picking::Ray& ray, const glm::vec3& axisOrigin, const glm::vec3& axisDirection)
{
    const glm::vec3 offset = ray.origin - axisOrigin;
    const float rayAxisDot = glm::dot(ray.direction, axisDirection);
    const float denominator = 1.0f - rayAxisDot * rayAxisDot;
    if (std::abs(denominator) < 1e-5f)
        return std::nullopt;

    const float rayOffsetDot = glm::dot(ray.direction, offset);
    const float axisOffsetDot = glm::dot(axisDirection, offset);
    return (axisOffsetDot - rayAxisDot * rayOffsetDot) / denominator;
}
