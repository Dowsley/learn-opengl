#include "voxel_renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VoxelRenderer::VoxelRenderer()
    : shader("shaders/voxelVertex.glsl", "shaders/voxelFragment.glsl")
{
    setupQuad();
}

VoxelRenderer::~VoxelRenderer()
{
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
}

void VoxelRenderer::setupQuad()
{
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void VoxelRenderer::render(const Camera& cam, int fbWidth, int fbHeight, const VoxelWorld& world)
{
    auto view = cam.getViewMatrix();
    auto proj = glm::perspective(
        glm::radians(cam.fov),
        static_cast<float>(fbWidth) / static_cast<float>(fbHeight),
        0.1f, 1000.0f);

    glm::mat4 invViewProj = glm::inverse(proj * view);

    shader.use();
    shader.setMat4("invViewProj", invViewProj, 1, GL_FALSE);
    shader.setVec3("camPos", cam.pos);
    shader.setVec3("resolution", glm::vec3(static_cast<float>(fbWidth), static_cast<float>(fbHeight), 0.0f));
    shader.setFloat("worldSize", static_cast<float>(VoxelWorld::SIZE));
    shader.setVec3("sunDir", glm::normalize(glm::vec3(0.4f, 0.8f, 0.3f)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, world.getTexture());
    shader.setInt("voxelTex", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
