#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class ShaderFactory;

class Shader
{
    friend class ShaderFactory;

public:
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;
    ~Shader();

    void Bind() const;
    void SetInt(const std::string& name, int value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
    Shader(const char* vertexSource, const char* fragmentSource);

    void CheckCompileErrors(GLuint shader) const;
    void CheckLinkErrors(GLuint program) const;

    template<GLenum ShaderType>
    GLuint CompileShader(const char* source)
    {
        GLuint shaderId = glCreateShader(ShaderType);
        glShaderSource(shaderId, 1, &source, nullptr);
        glCompileShader(shaderId);
        CheckCompileErrors(shaderId);

        return shaderId;
    }

    GLuint id = 0;
};
