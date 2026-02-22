#include "voxel_renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VoxelRenderer::VoxelRenderer()
    : shader("shaders/voxelVertex.glsl", "shaders/voxelFragment.glsl")
    , postShader("shaders/postVertex.glsl", "shaders/postFragment.glsl")
{
    setupQuad();
}

VoxelRenderer::~VoxelRenderer()
{
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (fboColor) glDeleteTextures(1, &fboColor);
    for (int i = 0; i < 2; i++)
    {
        if (bloomFBO[i]) glDeleteFramebuffers(1, &bloomFBO[i]);
        if (bloomColor[i]) glDeleteTextures(1, &bloomColor[i]);
    }
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fboWidth, fboHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VoxelRenderer::setupBloomFBOs(int width, int height)
{
    for (int i = 0; i < 2; i++)
    {
        if (bloomFBO[i]) glDeleteFramebuffers(1, &bloomFBO[i]);
        if (bloomColor[i]) glDeleteTextures(1, &bloomColor[i]);

        glGenTextures(1, &bloomColor[i]);
        glBindTexture(GL_TEXTURE_2D, bloomColor[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &bloomFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomColor[i], 0);
    }

    bloomWidth = width;
    bloomHeight = height;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void VoxelRenderer::render(const Camera& cam, int fbWidth, int fbHeight, const VoxelWorld& world)
{
    // Ensure half-res HDR FBO matches current window size
    int halfW = fbWidth / 2;
    int halfH = fbHeight / 2;
    if (halfW != fboWidth || halfH != fboHeight)
        setupFBO(halfW, halfH);

    // Ensure quarter-res bloom FBOs match
    int qW = halfW / 2;
    int qH = halfH / 2;
    if (qW != bloomWidth || qH != bloomHeight)
        setupBloomFBOs(qW, qH);

    // === Pass 1: Raymarch scene at half resolution into HDR FBO ===
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

    // === Post-processing pipeline ===
    postShader.use();
    postShader.setInt("sceneTex", 0);
    postShader.setInt("bloomTex", 1);

    // Bind a valid texture to unit 1 to suppress macOS driver warning
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomColor[0]);
    glActiveTexture(GL_TEXTURE0);

    // --- Pass 2: Bright extraction -> bloomFBO[0] at quarter res ---
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[0]);
    glViewport(0, 0, bloomWidth, bloomHeight);
    postShader.setInt("passMode", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboColor);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // --- Pass 3: Horizontal blur bloomFBO[0] -> bloomFBO[1] ---
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[1]);
    postShader.setInt("passMode", 1);
    postShader.setVec3("texelSize", glm::vec3(1.0f / bloomWidth, 1.0f / bloomHeight, 0.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bloomColor[0]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // --- Pass 4: Vertical blur bloomFBO[1] -> bloomFBO[0] ---
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[0]);
    postShader.setInt("passMode", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bloomColor[1]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // --- Pass 5: Final composite -> screen ---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fbWidth, fbHeight);
    postShader.setInt("passMode", 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboColor);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomColor[0]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}
