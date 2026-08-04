#pragma once
#include "Mesh.h"
#include "Material.h"
#include <glm/glm.hpp>
#include <memory>

struct RenderObject {
	std::shared_ptr<RenderMesh> mesh;
	std::shared_ptr<Material> material;
	glm::mat4 localToModel{1.0f};
};
