#pragma once

#include <glad/glad.h>
#include "render/shader.h"
#include "render/camera.h"
#include "voxel/voxel_world.h"

class VoxelRenderer
{
public:
    VoxelRenderer();
    ~VoxelRenderer();

    void render(const Camera& cam, int fbWidth, int fbHeight, const VoxelWorld& world);

private:
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    Shader shader;

    // Half-res HDR FBO for raymarching
    GLuint fbo = 0;
    GLuint fboColor = 0;
    int fboWidth = 0;
    int fboHeight = 0;

    // Post-processing (bloom)
    Shader postShader;
    GLuint bloomFBO[2] = {0, 0};
    GLuint bloomColor[2] = {0, 0};
    int bloomWidth = 0;
    int bloomHeight = 0;

    void setupQuad();
    void setupFBO(int width, int height);
    void setupBloomFBOs(int width, int height);
};
