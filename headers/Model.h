#pragma once
#include "RenderObject.h"
#include <limits>
#include <vector>

class ModelLoader;
class Shader;

struct AABB {
	glm::vec3 min{
		std::numeric_limits<float>::max()
	};

	glm::vec3 max{
		std::numeric_limits<float>::lowest()
	};
};

class Model {
public:
	Model() = delete;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;
	~Model() = default;

	void Draw(const Shader& shader, const glm::mat4& viewProjection) const;

	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);
	void setScale(const glm::vec3& newScale);

	const AABB& getLocalBounds() const;

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	glm::vec3 getScale() const;
	glm::mat4 getTransformMatrix() const;

private:
	friend class ModelLoader;

	explicit Model(std::vector<RenderObject>&& renderObjects, AABB&& aabb) noexcept;
	std::vector<RenderObject> renderObjects;
	AABB localBounds;

	glm::vec3 position{0.0f};
	glm::vec3 rotation{0.0f};
	glm::vec3 scale{1.0f};
};
