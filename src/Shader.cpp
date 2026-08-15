#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Shader::Shader(const char* vertexSource, const char* fragmentSource)
{
	GLuint vertex   = CompileShader<GL_VERTEX_SHADER>(vertexSource);
	GLuint fragment = CompileShader<GL_FRAGMENT_SHADER>(fragmentSource);
 
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    CheckLinkErrors(id);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
	if (id != 0)
		glDeleteProgram(id);
}

void Shader::Bind() const
{
	glUseProgram(id);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::CheckCompileErrors(GLuint shader) const
{
	GLint success = GL_FALSE;
	char infoLog[1024] = {};

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "ERROR::SHADER_COMPILATION_ERROR" << '\n' << infoLog << std::endl;
	}
}

void Shader::CheckLinkErrors(GLuint program) const
{
	GLint success = GL_FALSE;
	char infoLog[1024] = {};

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "ERROR::PROGRAM_LINKING_ERROR" << '\n' << infoLog << std::endl;
	}
}
