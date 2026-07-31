#pragma once

#include <glad/glad.h>
#include <string>

class Shader {

public:
	Shader(const std::string& pathToVertexShader,const std::string& pathToFragmentShader);
	void Bind()const;

private:

	std::string ProcessPathToShader(const std::string& pathToShader);

	template<GLenum ShaderType>
	GLuint CompileShader(const std::string& code) {
		const char* source = code.c_str();
		GLuint shaderId = glCreateShader(ShaderType);
		glShaderSource(shaderId, 1, &source, nullptr);
		glCompileShader(shaderId);
		return shaderId;
	}

	unsigned int id;
};
