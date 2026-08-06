#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>

class Shader;

class Material {
public:
    explicit Material(const std::filesystem::path& pathToTextures);
    Material(
        std::shared_ptr<Texture> diffuseTexture,
        std::shared_ptr<Texture> specularTexture,
        std::shared_ptr<Texture> normalTexture,
        const glm::vec4& baseColor) noexcept;

    void Bind(const Shader& shader) const;

private:
    std::shared_ptr<Texture> mDiffuseTexture;
    std::shared_ptr<Texture> mSpecularTexture;
    std::shared_ptr<Texture> mNormalTexture;
    glm::vec4 mBaseColor{1.0f};
};
