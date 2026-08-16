#include "Picking.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

glm::vec3 perspectiveDivide(const glm::vec4& point)
{
    return glm::vec3{point} / point.w;
}


picking::Ray picking::screenPointToRay(const glm::vec2& screenPoint, const glm::vec2& viewportSize, const glm::mat4& viewProjection)
{
    assert(viewportSize.x > 0.0f && viewportSize.y > 0.0f);

    const glm::vec2 normalizedDeviceCoordinates{ 2.0f * screenPoint.x / viewportSize.x - 1.0f, 1.0f - 2.0f * screenPoint.y / viewportSize.y };
    const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
    const glm::vec3 nearPoint = perspectiveDivide( inverseViewProjection * glm::vec4{normalizedDeviceCoordinates, -1.0f, 1.0f});
    const glm::vec3 farPoint = perspectiveDivide(inverseViewProjection * glm::vec4{normalizedDeviceCoordinates, 1.0f, 1.0f});

    return Ray{
        .origin = nearPoint,
        .direction = glm::normalize(farPoint - nearPoint)
    };
}

std::optional<float> IntersectRayAABB(const glm::vec3& origin, const glm::vec3& direction, const AABB& aabb, const float threshold = 1e-8f)
{
    float tNear = 0.0f;
    float tFar = std::numeric_limits<float>::infinity();

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::abs(direction[axis]) < threshold)
        {
            // Ray parallel to this slab.
            if (origin[axis] < aabb.min[axis]
                || origin[axis] > aabb.max[axis])
            {
                return std::nullopt;
            }
            continue;
        }

        float t1 = (aabb.min[axis] - origin[axis]) / direction[axis];

        float t2 = (aabb.max[axis] - origin[axis]) / direction[axis];

        float slabNear = std::min(t1, t2);
        float slabFar = std::max(t1, t2);

        tNear = std::max(tNear, slabNear);
        tFar = std::min(tFar, slabFar);

        if (tNear > tFar)
            return std::nullopt;
    }

    return tNear;
}

std::optional<std::size_t> picking::pick(const Ray& ray, std::span<const PickTarget> targets)
{
    std::optional<std::size_t> closestTarget;
    float closestDistance = std::numeric_limits<float>::max();

    for (std::size_t index = 0; index < targets.size(); ++index)
    {
        const PickTarget& target = targets[index];
        const glm::mat4 worldToLocal = glm::inverse(target.localToWorld);
        const glm::vec3 localOrigin{ worldToLocal * glm::vec4{ray.origin, 1.0f} };
        const glm::vec3 localDirection{ worldToLocal * glm::vec4{ray.direction, 0.0f} };
        const std::optional<float> distance = IntersectRayAABB(localOrigin,localDirection,target.localBounds);

        if (distance && *distance < closestDistance)
        {
            closestTarget = index;
            closestDistance = *distance;
        }
    }

    return closestTarget;
}

std::optional<std::size_t> picking::pick(const Ray& ray, std::span<const Model> models)
{
    std::vector<PickTarget> targets;
    targets.reserve(models.size());

    for (const Model& model : models)
    {
        targets.push_back(PickTarget{
            .localBounds = model.getLocalBounds(),
            .localToWorld = model.getTransformMatrix()
        });
    }

    return pick(ray, targets);
}
