#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"

#include <stb_image.h>

#include <iostream>
#include <limits>
#include <utility>

namespace {

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

GLuint uploadTexture(
    const unsigned char* data,
    int width,
    int height,
    GLenum dataFormat,
    GLenum internalFormat)
{
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    if (textureId == 0)
        return 0;

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint previousUnpackAlignment = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(internalFormat),
        width,
        height,
        0,
        dataFormat,
        GL_UNSIGNED_BYTE,
        data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureId;
}

} // namespace

std::optional<Texture> Texture::Load(const std::filesystem::path& path)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
    if (data == nullptr)
    {
        std::cerr << "Failed to load texture '" << path.string() << "': "
                  << stbi_failure_reason() << '\n';
        return std::nullopt;
    }

    const GLenum format = formatForChannels(channels);
    if (format == 0)
    {
        std::cerr << "Unsupported number of channels in texture '" << path.string() << "'\n";
        stbi_image_free(data);
        return std::nullopt;
    }

    const GLuint textureId = uploadTexture(data, width, height, format, format);
    if (textureId == 0)
    {
        std::cerr << "Failed to create OpenGL texture for '" << path.string() << "'\n";
        stbi_image_free(data);
        return std::nullopt;
    }

    stbi_image_free(data);
    return Texture(textureId);
}

std::optional<Texture> Texture::LoadEncoded(const unsigned char* data, std::size_t size)
{
    if (data == nullptr || size == 0 || size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        std::cerr << "Invalid embedded texture data\n";
        return std::nullopt;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* decodedData = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 0);

    if (decodedData == nullptr)
    {
        std::cerr << "Failed to decode embedded texture: " << stbi_failure_reason() << '\n';
        return std::nullopt;
    }

    const GLenum format = formatForChannels(channels);
    if (format == 0)
    {
        std::cerr << "Unsupported number of channels in embedded texture\n";
        stbi_image_free(decodedData);
        return std::nullopt;
    }

    const GLuint textureId = uploadTexture(decodedData, width, height, format, format);
    stbi_image_free(decodedData);

    if (textureId == 0)
    {
        std::cerr << "Failed to create OpenGL texture from embedded data\n";
        return std::nullopt;
    }

    return Texture(textureId);
}

std::optional<Texture> Texture::LoadRawBGRA(
    const unsigned char* data,
    unsigned int width,
    unsigned int height)
{
    if (data == nullptr || width == 0 || height == 0 ||
        width > static_cast<unsigned int>(std::numeric_limits<int>::max()) ||
        height > static_cast<unsigned int>(std::numeric_limits<int>::max()))
    {
        std::cerr << "Invalid uncompressed embedded texture data\n";
        return std::nullopt;
    }

    const GLuint textureId = uploadTexture(
        data,
        static_cast<int>(width),
        static_cast<int>(height),
        GL_BGRA,
        GL_RGBA);
    if (textureId == 0)
    {
        std::cerr << "Failed to create OpenGL texture from uncompressed embedded data\n";
        return std::nullopt;
    }

    return Texture(textureId);
}

Texture::Texture(GLuint textureId) noexcept
    : mTextureId(textureId)
{
}

Texture::~Texture()
{
    if (mTextureId != 0)
    {
        glDeleteTextures(1, &mTextureId);
    }
}

Texture::Texture(Texture&& other) noexcept
    : mTextureId(std::exchange(other.mTextureId, 0))
{
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (mTextureId != 0)
    {
        glDeleteTextures(1, &mTextureId);
    }

    mTextureId = std::exchange(other.mTextureId, 0);
    return *this;
}

void Texture::Bind(GLuint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
}

void Texture::Unbind(GLuint unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}
