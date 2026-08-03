#include "OrbitalTransform.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 OrbitalTransform::GetMatrix() const
{
    glm::mat4 transform = glm::identity<glm::mat4>();
    transform = glm::translate(transform, mCenter);
    transform = glm::rotate(transform, mLongitude, glm::vec3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, mLatitude, glm::vec3{1.0f, 0.0f, 0.0f});
    transform = glm::translate(transform, glm::vec3{0.0f, 0.0f, mDistance});

    return transform;
}

void OrbitalTransform::SetCenter(glm::vec3 center)
{
    mCenter = center;
}

void OrbitalTransform::SetDistance(float distance)
{
    mDistance = distance;
}

void OrbitalTransform::SetLatitude(float latitude)
{
    mLatitude = latitude;
}

void OrbitalTransform::SetLongitude(float longitude)
{
    mLongitude = longitude;
}
