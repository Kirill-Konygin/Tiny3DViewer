#pragma once
#include <glm/glm.hpp>

class OrbitalTransform
{
public:
    glm::mat4 GetMatrix() const;

    void SetCenter(glm::vec3 center);
    void SetDistance(float distance);
    void SetLatitude(float latitude);
    void SetLongitude(float longitude);

private:
    glm::vec3 mCenter;
    float mDistance;
    float mLatitude;
    float mLongitude;
};
