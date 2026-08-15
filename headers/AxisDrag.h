#pragma once

#include <glm/glm.hpp>

#include <optional>

namespace picking {
    struct Ray;
}

class AxisDrag
{
public:
    bool Begin(const picking::Ray& ray, const glm::vec3& origin, const glm::vec3& direction);
    std::optional<glm::vec3> Update(const picking::Ray& ray) const;

    void End();
    bool IsActive() const;

private:
    static std::optional<float> GetAxisParameter(const picking::Ray& ray, const glm::vec3& axisOrigin, const glm::vec3& axisDirection);

    glm::vec3 origin{0.0f};
    glm::vec3 direction{0.0f};
    float startAxisParameter = 0.0f;
    bool active = false;
};
