#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	Camera();
	Camera(float aspectRatio, float FOV);
	Camera(const glm::vec3& Position, const glm::vec3& Direction);
	Camera(const glm::vec3& Position, const glm::vec3& Direction, float aspectRatio, float FOV);

	void SetAspectRatio(float aspectRatio);
	void SetFOV(float FOV);
	void SetDirection(const glm::vec3& direction);
	void SetPosition(const glm::vec3& position);
	float GetFOV() const;
	glm::mat4 GetProjectionMatrix() const;
	glm::mat4 GetViewProjectionMatrix() const;

private:
	glm::vec3 mPosition  {0.0f, 0.0f,  3.0f};
	glm::vec3 mDirection {0.0f, 0.0f, -1.0f};
	float mAspectRatio = 16.0f / 9.0f;
	float mFOV = 45.0f;
};
