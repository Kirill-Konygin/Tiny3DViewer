#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <span>
struct Vertex {
    glm::vec3 position;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<std::uint32_t>& indices);
    ~Mesh();
    GLuint VAO = 0;
private:

    GLuint VBO = 0;
    GLuint EBO = 0;
};