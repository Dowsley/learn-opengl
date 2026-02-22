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

    void setupQuad();
};
