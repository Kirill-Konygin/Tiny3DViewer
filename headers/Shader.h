#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {

public:
	Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(Shader&&) = delete;
	~Shader();

	void Bind()const;
	void SetInt(const std::string& name, int value) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
	void CheckCompileErrors(GLuint shader) const;
	void CheckLinkErrors(GLuint program) const;

	template<GLenum ShaderType>
	GLuint CompileShader(const char* source) {
		GLuint shaderId = glCreateShader(ShaderType);
		glShaderSource(shaderId, 1, &source, nullptr);
		glCompileShader(shaderId);
		CheckCompileErrors(shaderId);

		return shaderId;
	}

	static constexpr const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec4 aTangent;

uniform mat4 transformMatrix;
uniform mat4 modelMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out float TangentBasisValidity;

void main()
{
	gl_Position = transformMatrix * vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	mat3 modelTransform = mat3(modelMatrix);
	mat3 normalTransform = transpose(inverse(modelTransform));
	Normal = normalize(normalTransform * aNormal);
	TangentBasisValidity = abs(aTangent.w);

	if (TangentBasisValidity > 0.5)
	{
		vec3 transformedTangent = modelTransform * aTangent.xyz;
		Tangent = normalize(transformedTangent - dot(transformedTangent, Normal) * Normal);
		Bitangent = normalize(cross(Normal, Tangent)) * aTangent.w;
	}
	else
	{
		Tangent = vec3(0.0);
		Bitangent = vec3(0.0);
	}
}
)";

	static constexpr const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in float TangentBasisValidity;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform vec4 baseColor;
uniform bool hasDiffuseTexture;
uniform bool hasNormalTexture;

void main()
{
	vec4 surfaceColor = baseColor;
	if (hasDiffuseTexture)
		surfaceColor *= texture(diffuseTexture, TexCoord);

	vec3 surfaceNormal = normalize(Normal);
	if (hasNormalTexture && TangentBasisValidity > 0.5)
	{
		vec3 tangentNormal = texture(normalTexture, TexCoord).rgb * 2.0 - 1.0;
		vec3 surfaceTangent = normalize(Tangent - dot(Tangent, surfaceNormal) * surfaceNormal);
		vec3 surfaceBitangent = normalize(
			Bitangent
			- dot(Bitangent, surfaceNormal) * surfaceNormal
			- dot(Bitangent, surfaceTangent) * surfaceTangent);
		mat3 tangentToWorld = mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
		surfaceNormal = normalize(tangentToWorld * tangentNormal);
	}

	const vec3 lightDirection = vec3(0.4364, 0.8729, 0.2182);
	const float ambientStrength = 0.2;
	float diffuseStrength = max(dot(surfaceNormal, lightDirection), 0.0);
	float lighting = ambientStrength + (1.0 - ambientStrength) * diffuseStrength;

	FragColor = vec4(surfaceColor.rgb * lighting, surfaceColor.a);
}
)";

	GLuint id;
};
