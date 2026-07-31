#define STB_IMAGE_IMPLEMENTATION
#include "Material.h"
#include "Shader.h"

#include <stb_image.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

enum class TextureType {
    Diffuse,
    Specular,
    Normal,
    Unknown
};

TextureType getTextureType(const std::filesystem::path& texturePath)
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

    return TextureType::Unknown;
}

GLenum formatForChannels(int channels)
{
    switch (channels)
    {
    case 1:
        return GL_RED;
    case 2:
        return GL_RG;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        return 0;
    }
}

GLuint loadTexture(const std::filesystem::path& path)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
    if (data == nullptr)
    {
        std::cerr << "Failed to load texture '" << path.string() << "': "
                  << stbi_failure_reason() << '\n';
        return 0;
    }

    const GLenum format = formatForChannels(channels);
    if (format == 0)
    {
        std::cerr << "Unsupported number of channels in texture '" << path.string() << "'\n";
        stbi_image_free(data);
        return 0;
    }
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return textureId;
}

void bindTexture(const Shader& shader, const char* uniformName, GLint unit, GLuint textureId)
{
    shader.SetInt(uniformName, unit);
    // OpenGL texture unit constants are contiguous, so GL_TEXTURE0 + unit is safe.
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
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

    stbi_set_flip_vertically_on_load(true);

    for (const auto& entry : std::filesystem::directory_iterator(pathToTextures, error))
    {
        if (error)
        {
            std::cerr << "Failed to read texture directory '" << pathToTextures.string()
                      << "': " << error.message() << '\n';
            break;
        }

        switch (getTextureType(entry.path()))
        {
        case TextureType::Diffuse:
            if (diffuseTextureId == 0)
            {
                diffuseTextureId = loadTexture(entry.path()); 
            }
            break;
        case TextureType::Specular:
            if (specularTextureId == 0)
            {
                specularTextureId = loadTexture(entry.path());
            }
            break;
        case TextureType::Normal:
            if (normalTextureId == 0)
            {
                normalTextureId = loadTexture(entry.path());
            }
            break;
        }
    }
}

Material::~Material()
{
    const GLuint textureIds[] = {
        diffuseTextureId,
        specularTextureId,
        normalTextureId
    };
    glDeleteTextures(3, textureIds);
}

void Material::Bind(const Shader& shader) const
{
    bindTexture(shader, "diffuseTexture", 0, diffuseTextureId);
    bindTexture(shader, "specularTexture", 1, specularTextureId);
    bindTexture(shader, "normalTexture", 2, normalTextureId);
}
