#include "ShaderFactory.h"

Shader ShaderFactory::MakeModelShader()
{
    constexpr const char* modelVertexShaderSource = R"(#version 330 core
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

    constexpr const char* modelFragmentShaderSource = R"(#version 330 core
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
    return Shader{modelVertexShaderSource, modelFragmentShaderSource};
}

Shader ShaderFactory::MakeGizmoShader()
{
    constexpr const char* gizmoVertexShaderSource = R"(#version 330 core
    layout (location = 0) in vec3 aPosition;

    uniform mat4 transformMatrix;

    void main()
    {
        gl_Position = transformMatrix * vec4(aPosition, 1.0);
    }
    )";

    constexpr const char* gizmoFragmentShaderSource = R"(#version 330 core
    out vec4 FragColor;

    uniform vec4 color;

    void main()
    {
        FragColor = color;
    }
    )";
    return Shader{gizmoVertexShaderSource, gizmoFragmentShaderSource};
}
