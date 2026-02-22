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
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (fboColor) glDeleteTextures(1, &fboColor);
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

void VoxelRenderer::setupFBO(int width, int height)
{
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (fboColor) glDeleteTextures(1, &fboColor);

    fboWidth = width;
    fboHeight = height;

    glGenTextures(1, &fboColor);
    glBindTexture(GL_TEXTURE_2D, fboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, fboWidth, fboHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VoxelRenderer::render(const Camera& cam, int fbWidth, int fbHeight, const VoxelWorld& world)
{
    // Ensure half-res FBO matches current window size
    int halfW = fbWidth / 2;
    int halfH = fbHeight / 2;
    if (halfW != fboWidth || halfH != fboHeight)
        setupFBO(halfW, halfH);

    // --- Pass 1: Raymarch at half resolution ---
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fboWidth, fboHeight);

    auto view = cam.getViewMatrix();
    auto proj = glm::perspective(
        glm::radians(cam.fov),
        static_cast<float>(fbWidth) / static_cast<float>(fbHeight),
        0.1f, 1000.0f);

    glm::mat4 invViewProj = glm::inverse(proj * view);

    shader.use();
    shader.setMat4("invViewProj", invViewProj, 1, GL_FALSE);
    shader.setVec3("camPos", cam.pos);
    shader.setVec3("resolution", glm::vec3(static_cast<float>(fboWidth), static_cast<float>(fboHeight), 0.0f));
    shader.setFloat("worldSize", static_cast<float>(VoxelWorld::SIZE));
    shader.setVec3("sunDir", glm::normalize(glm::vec3(0.4f, 0.8f, 0.3f)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, world.getTexture());
    shader.setInt("voxelTex", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // --- Pass 2: Blit half-res to full screen with bilinear upscale ---
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, fboWidth, fboHeight, 0, 0, fbWidth, fbHeight,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fbWidth, fbHeight);
}
