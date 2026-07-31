#pragma once

#include <glad/glad.h>

#include <filesystem>

class Shader;

class Material {
public:
    explicit Material(const std::filesystem::path& pathToTextures);
    ~Material();
    void Bind(const Shader& shader) const;

private:
    GLuint diffuseTextureId = 0;
    GLuint specularTextureId = 0;
    GLuint normalTextureId = 0;
};
