#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {

public:
	Shader(const std::string& pathToVertexShader,const std::string& pathToFragmentShader);
	void Bind()const;
	void SetInt(const std::string& name, int value) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetMat4(const std::string& name, const glm::mat4& value) const;

private:

	std::string ProcessPathToShader(const std::string& pathToShader);
	void CheckCompileErrors(GLuint shader) const;
	void CheckLinkErrors(GLuint program) const;

	template<GLenum ShaderType>
	GLuint CompileShader(const std::string& code) {
		const char* source = code.c_str();
		GLuint shaderId = glCreateShader(ShaderType);
		glShaderSource(shaderId, 1, &source, nullptr);
		glCompileShader(shaderId);
		CheckCompileErrors(shaderId);

		return shaderId;
	}

	unsigned int id;
};
