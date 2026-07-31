#include "Shader.h"
#include <glad/glad.h>
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
