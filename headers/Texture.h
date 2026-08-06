#pragma once

#include <glad/glad.h>

#include <cstddef>
#include <filesystem>
#include <optional>

class Texture final {
public:
    [[nodiscard]] static std::optional<Texture> Load(const std::filesystem::path& path);
    [[nodiscard]] static std::optional<Texture> LoadEncoded(const unsigned char* data, std::size_t size);
    [[nodiscard]] static std::optional<Texture> LoadRawBGRA(const unsigned char* data, unsigned int width, unsigned int height);

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void Bind(GLuint unit) const;
    static void Unbind(GLuint unit);

private:
    explicit Texture(GLuint textureId) noexcept;

    GLuint mTextureId = 0;
};
