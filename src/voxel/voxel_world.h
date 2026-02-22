#pragma once

#include <cstdint>
#include <glad/glad.h>

class VoxelWorld
{
public:
    static constexpr int SIZE = 64;

    // Block types
    static constexpr uint8_t AIR   = 0;
    static constexpr uint8_t GRASS = 1;
    static constexpr uint8_t DIRT  = 2;
    static constexpr uint8_t STONE = 3;
    static constexpr uint8_t WOOD  = 4;
    static constexpr uint8_t LEAF  = 5;

    VoxelWorld();
    ~VoxelWorld();

    void uploadToGPU();
    GLuint getTexture() const { return texture; }

private:
    uint8_t voxels[SIZE * SIZE * SIZE]{};
    GLuint texture = 0;

    void generateTerrain();
    void placeTree(int tx, int tz);

    uint8_t& at(int x, int y, int z) { return voxels[x + SIZE * (y + SIZE * z)]; }
};
