#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "Model.h"
#include "Texture.h"

struct aiMesh;
struct aiNode;
struct aiScene;
struct aiTexture;
struct aiMaterial;
struct aiAABB;

class ModelLoader {
public:
	std::optional<Model> LoadModel(const std::filesystem::path& pathToModel);

private:
	static AABB transformAABBToModelSpace(const aiAABB& aabb, const glm::mat4& meshToModel);

	void processNode(	const aiNode* node,
						const aiScene* scene,
						const glm::mat4& parentTransform,
						const std::vector<std::shared_ptr<RenderMesh>>& meshes,
						const std::vector<std::shared_ptr<Material>>& materials,
						AABB& modelBound,
						std::vector<RenderObject>& renderObjects) const;

	std::shared_ptr<RenderMesh> processMesh(const aiMesh* mesh) const;
	std::shared_ptr<Texture> processTexture(const aiTexture* texture) const;
	std::shared_ptr<Material> processMaterial(const aiMaterial* material, const aiScene* scene, const std::vector<std::shared_ptr<Texture>>& textures) const;
};
