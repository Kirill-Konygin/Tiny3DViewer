#pragma once

#include "Model.h"

#include <glm/glm.hpp>

#include <span>

namespace picking {

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
	};

	// screenPoint uses top-left as its origin; viewportSize must be positive.
	Ray screenPointToRay(const glm::vec2& screenPoint, const glm::vec2& viewportSize, const glm::mat4& viewProjection);

	const Model* pickModel(const Ray& ray, std::span<Model>);
}
