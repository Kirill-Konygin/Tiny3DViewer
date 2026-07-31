#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

Shader::Shader(const std::string& pathToVertexShader, const std::string& pathToFragmentShader)
{
    const std::string vertexCode = ProcessPathToShader(pathToVertexShader);
    const std::string fragmentCode = ProcessPathToShader(pathToFragmentShader);


	GLuint vertex   = CompileShader<GL_VERTEX_SHADER>(vertexCode);
	GLuint fragment = CompileShader<GL_FRAGMENT_SHADER>(fragmentCode);
 
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::Bind() const
{
	glUseProgram(id);
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
