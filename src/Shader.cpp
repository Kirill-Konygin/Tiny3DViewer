#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Shader::Shader(const std::string& pathToVertexShader, const std::string& pathToFragmentShader)
{
    const std::string vertexCode = ProcessPathToShader(pathToVertexShader);
    const std::string fragmentCode = ProcessPathToShader(pathToFragmentShader);


	GLuint vertex   = CompileShader<GL_VERTEX_SHADER>(vertexCode);
	CheckCompileErrors(vertex);
	GLuint fragment = CompileShader<GL_FRAGMENT_SHADER>(fragmentCode);
	CheckCompileErrors(fragment);
 
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    CheckLinkErrors(id);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
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

std::string Shader::ProcessPathToShader(const std::string& pathToShader)
{
	std::ifstream shaderFile(pathToShader);
	if (!shaderFile.is_open())
	{
		throw std::runtime_error("Failed to open shader file: " + pathToShader);
	}

	std::ostringstream shaderCode;
	shaderCode << shaderFile.rdbuf();

	if (shaderFile.bad())
	{
		throw std::runtime_error("Failed to read shader file: " + pathToShader);
	}
	return shaderCode.str();
}
