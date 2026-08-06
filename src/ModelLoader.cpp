#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

std::shared_ptr<Texture> getEmbeddedMaterialTexture(const aiMaterial* material, aiTextureType textureType, const aiScene* scene,const std::vector<std::shared_ptr<Texture>>& textures)
{
    aiString texturePath;
    if (material->GetTexture(textureType, 0, &texturePath) != AI_SUCCESS)
        return {};

    const auto [embeddedTexture, textureIndex] = scene->GetEmbeddedTextureAndIndex(texturePath.C_Str());
    if (embeddedTexture == nullptr)
    {
        std::cerr << "Material texture reference could not be matched to an embedded texture: '"
                  << texturePath.C_Str() << "' (scene contains "
                  << scene->mNumTextures << " embedded textures)\n";
        return {};
    }

    if (textureIndex < 0 || static_cast<std::size_t>(textureIndex) >= textures.size())
    {
        std::cerr << "Invalid embedded texture index for material: '"
                  << texturePath.C_Str() << "'\n";
        return {};
    }

    return textures[static_cast<std::size_t>(textureIndex)];
}

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
    const aiScene* scene = import.ReadFile(pathToModel.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        return std::nullopt;
    

    std::vector<std::shared_ptr<RenderMesh>> meshes;
    meshes.reserve(scene->mNumMeshes);
    std::vector<std::shared_ptr<Texture>> textures;
    textures.reserve(scene->mNumTextures);
    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve(scene->mNumMaterials);
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        meshes.push_back(processMesh(scene->mMeshes[i]));
    for (unsigned int i = 0; i < scene->mNumTextures; ++i)
        textures.push_back(processTexture(scene->mTextures[i]));
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
        materials.push_back(processMaterial(scene->mMaterials[i], scene, textures));

    std::vector<RenderObject> renderObjects;
    processNode(scene->mRootNode, scene, glm::mat4{1.0f}, meshes, materials, renderObjects);

    return Model(std::move(renderObjects));
}

void ModelLoader::processNode(  const aiNode* node,
                                const aiScene* scene,
                                const glm::mat4& parentTransform,
                                const std::vector<std::shared_ptr<RenderMesh>>& meshes,
                                const std::vector<std::shared_ptr<Material>>& materials,
                                std::vector<RenderObject>& renderObjects                ) const
{
    const glm::mat4 nodeTransform = parentTransform * toGlmMatrix(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const unsigned int meshIndex = node->mMeshes[i];
        assert(meshIndex < meshes.size());
        assert(meshIndex < scene->mNumMeshes);

        const unsigned int materialIndex = scene->mMeshes[meshIndex]->mMaterialIndex;
        assert(materialIndex < materials.size());

        renderObjects.push_back(RenderObject{
            meshes[meshIndex],
            materials[materialIndex],
            nodeTransform
        });
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, nodeTransform, meshes, materials, renderObjects);
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

        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        if (mesh->HasNormals())
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        vertex.texCoord = glm::vec2(0.0f, 0.0f);
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
        }

        if (mesh->HasTangentsAndBitangents())
        {
            const aiVector3D& tangent = mesh->mTangents[i];
            const aiVector3D& bitangent = mesh->mBitangents[i];
            const aiVector3D& normal = mesh->mNormals[i];
            const float tangentLengthSquared = tangent.SquareLength();
            const float bitangentLengthSquared = bitangent.SquareLength();

            if (tangentLengthSquared > 0.0f && bitangentLengthSquared > 0.0f)
            {
                const float handedness = ((normal ^ tangent) * bitangent) < 0.0f ? -1.0f : 1.0f;
                vertex.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, handedness);
            }
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

std::shared_ptr<Texture> ModelLoader::processTexture(const aiTexture* texture) const
{
    if (texture == nullptr || texture->pcData == nullptr || texture->mWidth == 0)
        return {};

    std::optional<Texture> loadedTexture;
    if (texture->mHeight == 0)
    {
        loadedTexture = Texture::LoadEncoded(
            reinterpret_cast<const unsigned char*>(texture->pcData),
            texture->mWidth);
    }
    else
    {
        loadedTexture = Texture::LoadRawBGRA(
            reinterpret_cast<const unsigned char*>(texture->pcData),
            texture->mWidth,
            texture->mHeight);
    }

    if (!loadedTexture)
        return {};

    return std::make_shared<Texture>(std::move(*loadedTexture));
}

std::shared_ptr<Material> ModelLoader::processMaterial(const aiMaterial* material, const aiScene* scene, const std::vector<std::shared_ptr<Texture>>& textures) const
{
    if (material == nullptr || scene == nullptr)
        return {};

    std::shared_ptr<Texture> diffuseTexture = getEmbeddedMaterialTexture( material, aiTextureType_BASE_COLOR, scene, textures);
    if (!diffuseTexture)
    {
        diffuseTexture = getEmbeddedMaterialTexture(material, aiTextureType_DIFFUSE, scene, textures);
    }
    std::shared_ptr<Texture> specularTexture = getEmbeddedMaterialTexture(material, aiTextureType_SPECULAR, scene, textures);
    std::shared_ptr<Texture> normalTexture = getEmbeddedMaterialTexture(material, aiTextureType_NORMALS, scene, textures);

    aiColor4D assimpBaseColor{1.0f, 1.0f, 1.0f, 1.0f};
    if (material->Get(AI_MATKEY_BASE_COLOR, assimpBaseColor) != AI_SUCCESS)
        material->Get(AI_MATKEY_COLOR_DIFFUSE, assimpBaseColor);

    const glm::vec4 baseColor{
        assimpBaseColor.r,
        assimpBaseColor.g,
        assimpBaseColor.b,
        assimpBaseColor.a
    };

    return std::make_shared<Material>(std::move(diffuseTexture), std::move(specularTexture), std::move(normalTexture), baseColor);
}
