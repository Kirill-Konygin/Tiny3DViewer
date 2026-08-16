#pragma once

#include "Model.h"

#include <glm/glm.hpp>

#include <cstddef>
#include <optional>
#include <span>

namespace picking {

	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
	};

	struct PickTarget
	{
		AABB localBounds;
		glm::mat4 localToWorld{1.0f};
	};

	// screenPoint uses top-left as its origin; viewportSize must be positive.
	Ray screenPointToRay(const glm::vec2& screenPoint, const glm::vec2& viewportSize, const glm::mat4& viewProjection);

	std::optional<std::size_t> pick(const Ray& ray, std::span<const PickTarget> targets);
	std::optional<std::size_t> pick(const Ray& ray, std::span<const Model> models);
}
