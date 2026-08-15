#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <span>
#include <vector>

class Mover
{
public:
    struct Target
    {
        glm::vec3 startPosition{0.0f};
        std::function<void(const glm::vec3&)> setPosition;
    };

    void Begin(std::span<const Target> targets);
    void Move(const glm::vec3& offset);
    void End();

    bool IsActive() const;

private:
    std::vector<Target> targets;
};
