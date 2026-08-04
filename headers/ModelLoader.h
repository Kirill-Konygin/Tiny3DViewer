#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "Model.h"

struct aiMesh;
struct aiNode;

class ModelLoader {
public:
	std::optional<Model> LoadModel(const std::filesystem::path& pathToModel);

private:
	void processNode(	const aiNode* node,
						const glm::mat4& parentTransform,
						const std::vector<std::shared_ptr<RenderMesh>>& meshes,
						std::vector<RenderObject>& renderObjects) const;

	std::shared_ptr<RenderMesh> processMesh(const aiMesh* mesh) const;
};
