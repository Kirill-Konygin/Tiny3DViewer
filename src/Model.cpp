#include "Model.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <utility>

Model::Model(std::vector<RenderObject>&& renderObjects) noexcept
    : renderObjects(std::move(renderObjects))
{
}

void Model::Draw(const Shader& shader, const glm::mat4& viewProjection) const
{
    const glm::mat4 modelTransform = getTransformMatrix();

    for (const auto& renderObject : renderObjects)
    {
        if (!renderObject.mesh)
            continue;

        if (renderObject.material)
            renderObject.material->Bind(shader);

        shader.SetMat4("transformMatrix", viewProjection * modelTransform * renderObject.localToModel);
        renderObject.mesh->Draw();
    }
}

void Model::setPosition(const glm::vec3& newPosition)
{
    position = newPosition;
}

void Model::setRotation(const glm::vec3& newRotation)
{
    rotation = newRotation;
}

void Model::setScale(const glm::vec3& newScale)
{
    scale = newScale;
}

glm::vec3 Model::getPosition() const
{
    return position;
}

glm::vec3 Model::getRotation() const
{
    return rotation;
}

glm::vec3 Model::getScale() const
{
    return scale;
}

glm::mat4 Model::getTransformMatrix() const
{
    glm::mat4 transform{1.0f};
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, rotation.x, glm::vec3{1.0f, 0.0f, 0.0f});
    transform = glm::rotate(transform, rotation.y, glm::vec3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, rotation.z, glm::vec3{0.0f, 0.0f, 1.0f});
    transform = glm::scale(transform, scale);
    return transform;
}
