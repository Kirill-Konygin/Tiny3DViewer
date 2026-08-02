#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() = default;

Camera::Camera(float aspectRatio, float FOV)
    :   mAspectRatio(aspectRatio),
        mFOV(FOV)
{
}

Camera::Camera(const glm::vec3& Position, const glm::vec3& Direction)
    :   mPosition(Position),
        mDirection(glm::normalize(Direction))
{
}

Camera::Camera(const glm::vec3& Position, const glm::vec3& Direction, float aspectRatio, float FOV)
    :   mPosition(Position),
        mDirection(glm::normalize(Direction)),
        mAspectRatio(aspectRatio),
        mFOV(FOV)
{
}

void Camera::SetAspectRatio(float aspectRatio)
{
    mAspectRatio = aspectRatio;
}

void Camera::SetFOV(float FOV)
{
    mFOV = FOV;
}

void Camera::SetDirection(const glm::vec3& direction)
{
    mDirection = glm::normalize(direction);
}

void Camera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(mFOV), mAspectRatio, 0.1f, 100.0f);
}

glm::mat4 Camera::GetViewProjectionMatrix() const
{
    const glm::mat4 view = glm::lookAt(mPosition, mPosition + mDirection, glm::vec3{0.0f, 1.0f, 0.0f});

    return GetProjectionMatrix() * view;
}
