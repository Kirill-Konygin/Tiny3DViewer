#include "Material.h"
#include "Shader.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

enum class TextureType {
    Diffuse,
    Specular,
    Normal
};

std::optional<TextureType> getTextureType(const std::filesystem::path& texturePath)
{
    std::string suffix = texturePath.stem().string();
    const std::size_t separator = suffix.find_last_of('_');
    if (separator != std::string::npos)
    {
        suffix.erase(0, separator + 1);
    }


    std::transform(suffix.begin(), suffix.end(), suffix.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

    if (suffix == "diff" || suffix == "diffuse")
    {
        return TextureType::Diffuse;
    }
    if (suffix == "spec" || suffix == "specular")
    {
        return TextureType::Specular;
    }
    if (suffix == "norm" || suffix == "normal")
    {
        return TextureType::Normal;
    }

    return std::nullopt;
}

void loadIfMissing(std::shared_ptr<Texture>& texture, const std::filesystem::path& path)
{
    if (texture)
        return;

    std::optional<Texture> loadedTexture = Texture::Load(path);
    if (loadedTexture)
        texture = std::make_shared<Texture>(std::move(*loadedTexture));
}

void bindTexture(const Shader& shader, const std::shared_ptr<Texture>& texture, const char* uniformName, GLuint unit)
{
    shader.SetInt(uniformName, static_cast<int>(unit));
    if (texture)
    {
        texture->Bind(unit);
    }
    else
    {
        Texture::Unbind(unit);
    }
}

Material::Material(const std::filesystem::path& pathToTextures)
{
    std::error_code error;
    if (!std::filesystem::is_directory(pathToTextures, error))
    {
        std::cerr << "Texture directory does not exist or is not a directory: '"
                  << pathToTextures.string() << "'\n";
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(pathToTextures, error))
    {
        if (error)
        {
            std::cerr << "Failed to read texture directory '" << pathToTextures.string()
                      << "': " << error.message() << '\n';
            break;
        }

        const std::optional<TextureType> type = getTextureType(entry.path());
        if (!type)
        {
            continue;
        }

        switch (*type)
        {
        case TextureType::Diffuse:
            loadIfMissing(mDiffuseTexture, entry.path());
            break;
        case TextureType::Specular:
            loadIfMissing(mSpecularTexture, entry.path());
            break;
        case TextureType::Normal:
            loadIfMissing(mNormalTexture, entry.path());
            break;
        }
    }
}

Material::Material(
    std::shared_ptr<Texture> diffuseTexture,
    std::shared_ptr<Texture> specularTexture,
    std::shared_ptr<Texture> normalTexture,
    const glm::vec4& baseColor) noexcept
    : mDiffuseTexture(std::move(diffuseTexture)),
      mSpecularTexture(std::move(specularTexture)),
      mNormalTexture(std::move(normalTexture)),
      mBaseColor(baseColor)
{
}

void Material::Bind(const Shader& shader) const
{
    shader.SetVec4("baseColor", mBaseColor);
    shader.SetInt("hasDiffuseTexture", mDiffuseTexture ? 1 : 0);

    bindTexture(shader, mDiffuseTexture, "diffuseTexture", 0);
    bindTexture(shader, mSpecularTexture, "specularTexture", 1);
    bindTexture(shader, mNormalTexture, "normalTexture", 2);
}
