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
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<std::uint32_t>& indices);
    ~Mesh();
    void Bind() const;

    inline std::size_t getIndexCount() {
        return indexCount;
    }
private:

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    std::size_t indexCount = 0;
};
