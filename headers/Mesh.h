#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec4 tangent;
};

class RenderMesh {
public:
    RenderMesh(const std::vector<Vertex>& vertices,
         const std::vector<std::uint32_t>& indices);

    RenderMesh(const RenderMesh&) = delete;
    RenderMesh& operator=(const RenderMesh&) = delete;
    RenderMesh(RenderMesh&&) = delete;
    RenderMesh& operator=(RenderMesh&&) = delete;
    ~RenderMesh();

    void Draw() const;
private:

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    std::size_t indexCount = 0;
};
