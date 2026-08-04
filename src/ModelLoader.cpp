#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

glm::mat4 toGlmMatrix(const aiMatrix4x4& matrix)
{
    return {
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4
    };
}

std::optional<Model> ModelLoader::LoadModel(const std::filesystem::path& pathToModel)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(pathToModel.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        return std::nullopt;
    

    std::vector<std::shared_ptr<RenderMesh>> meshes;
    meshes.reserve(scene->mNumMeshes);
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        meshes.push_back(processMesh(scene->mMeshes[i]));

    std::vector<RenderObject> renderObjects;
    processNode(scene->mRootNode, glm::mat4{1.0f}, meshes, renderObjects);

    return Model(std::move(renderObjects));
}

void ModelLoader::processNode(  const aiNode* node, 
                                const glm::mat4& parentTransform, 
                                const std::vector<std::shared_ptr<RenderMesh>>& meshes,
                                std::vector<RenderObject>& renderObjects                ) const
{
    const glm::mat4 nodeTransform = parentTransform * toGlmMatrix(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const unsigned int meshIndex = node->mMeshes[i];
        if (meshIndex >= meshes.size())
            continue;

        renderObjects.push_back(RenderObject{
            meshes[meshIndex],
            {},
            nodeTransform
        });
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], nodeTransform, meshes, renderObjects);
}

std::shared_ptr<RenderMesh> ModelLoader::processMesh(const aiMesh* mesh) const
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vertex.texCoord = glm::vec2(0.0f, 0.0f);
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
        }
        vertices.push_back(vertex);
    }
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(static_cast<std::uint32_t>(face.mIndices[j]));
    }

    return std::make_shared<RenderMesh>(vertices, indices);
}
