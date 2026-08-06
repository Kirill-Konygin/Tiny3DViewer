#pragma once
#include "RenderObject.h"

#include <vector>

class ModelLoader;
class Shader;

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

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	glm::vec3 getScale() const;
	glm::mat4 getTransformMatrix() const;

private:
	friend class ModelLoader;

	explicit Model(std::vector<RenderObject>&& renderObjects) noexcept;
	std::vector<RenderObject> renderObjects;

	glm::vec3 position{0.0f};
	glm::vec3 rotation{0.0f};
	glm::vec3 scale{1.0f};
};
