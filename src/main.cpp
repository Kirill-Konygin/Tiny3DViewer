#include "Window.h"
#include "Mesh.h"
#include "Material.h"
#include "Shader.h"
#include <iostream>


int main()
{
    Window window("Tiny3DViewer");
    Shader shader("../shaders/VertexShader", "../shaders/FragmentShader");
    Material material("../textures");
    
    RenderMesh mesh(
        {
            Vertex{glm::vec3{  0.5f,  0.5f, 0.0f}, glm::vec2{1.0f, 1.0f}},
            Vertex{glm::vec3{  0.5f, -0.5f, 0.0f}, glm::vec2{1.0f, 0.0f}},
            Vertex{glm::vec3{ -0.5f, -0.5f, 0.0f}, glm::vec2{0.0f, 0.0f}},
            Vertex{glm::vec3{ -0.5f,  0.5f, 0.0f}, glm::vec2{0.0f, 1.0f}}
        },
        {
            0, 1, 3,
            1, 2, 3
        }
    );

    while (!window.shouldClose())
    {
        window.clear();
        shader.Bind();
        material.Bind(shader);
        mesh.Draw();
        window.update();
    }
    
    return 0;
}
