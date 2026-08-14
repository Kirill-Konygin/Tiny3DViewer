#include "Picking.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <optional>

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
            if (origin[axis] < aabb.min[axis] ||
                origin[axis] > aabb.max[axis])
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

const Model* picking::pickModel(const Ray& ray, std::span<Model> Models)
{
    const Model* model_ptr = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (const auto& model : Models) {
        glm::mat4 ToModelSpace = glm::inverse(model.getTransformMatrix());
        auto dist_opt = IntersectRayAABB(ToModelSpace * glm::vec4{ ray.origin,1.f }, ToModelSpace * glm::vec4{ ray.direction, 0.f }, model.getLocalBounds());
        if (dist_opt.has_value()) {
            if (dist_opt.value() < minDist) {
                model_ptr = &model;
                minDist = dist_opt.value();
            }
        }
    }

    return model_ptr;
}
