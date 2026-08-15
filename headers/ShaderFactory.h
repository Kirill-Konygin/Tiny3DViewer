#pragma once

#include "Shader.h"

class ShaderFactory
{
public:
    static Shader MakeModelShader();
    static Shader MakeGizmoShader();
};
