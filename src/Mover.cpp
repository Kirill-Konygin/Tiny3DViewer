#include "Mover.h"

void Mover::Begin(std::span<const Target> newTargets)
{
    End();
    targets.reserve(newTargets.size());

    for (const Target& target : newTargets)
    {
        if (!target.setPosition)
            continue;

        targets.push_back(target);
    }
}

void Mover::Move(const glm::vec3& offset)
{
    for (const Target& target : targets)
        target.setPosition(target.startPosition + offset);
}

void Mover::End()
{
    targets.clear();
}

bool Mover::IsActive() const
{
    return !targets.empty();
}
