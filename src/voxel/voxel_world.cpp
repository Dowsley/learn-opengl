#include "voxel_world.h"
#include <cmath>
#include <cstdlib>

VoxelWorld::VoxelWorld()
    : voxels(SIZE * SIZE * SIZE, AIR)
{
    generateTerrain();
    uploadToGPU();
}

VoxelWorld::~VoxelWorld()
{
    if (texture)
        glDeleteTextures(1, &texture);
}

void VoxelWorld::generateTerrain()
{
    // Sine-wave heightmap — scaled for 256³ grid
    for (int x = 0; x < SIZE; x++)
    {
        for (int z = 0; z < SIZE; z++)
        {
            float fx = static_cast<float>(x) / SIZE;
            float fz = static_cast<float>(z) / SIZE;

            float height = 64.0f
                + 24.0f * std::sin(fx * 6.28f * 2.0f)
                + 16.0f * std::sin(fz * 6.28f * 3.0f)
                + 12.0f * std::sin((fx + fz) * 6.28f * 1.5f);

            int h = static_cast<int>(height);
            if (h < 1) h = 1;
            if (h >= SIZE) h = SIZE - 1;

            for (int y = 0; y < h; y++)
            {
                if (y == h - 1)
                    at(x, y, z) = GRASS;
                else if (y >= h - 16)
                    at(x, y, z) = DIRT;
                else
                    at(x, y, z) = STONE;
            }
        }
    }

    // Scatter trees
    std::srand(42);
    for (int i = 0; i < 30; i++)
    {
        int tx = std::rand() % (SIZE - 24) + 12;
        int tz = std::rand() % (SIZE - 24) + 12;
        placeTree(tx, tz);
    }
}

void VoxelWorld::placeTree(int tx, int tz)
{
    // Find ground height
    int ground = 0;
    for (int y = SIZE - 1; y >= 0; y--)
    {
        if (at(tx, y, tz) != AIR)
        {
            ground = y + 1;
            break;
        }
    }

    if (ground <= 0 || ground + 28 >= SIZE) return;

    int trunkHeight = 16 + (std::rand() % 12);

    // Trunk
    for (int y = ground; y < ground + trunkHeight; y++)
        at(tx, y, tz) = WOOD;

    // Leaves (canopy)
    int leafBase = ground + trunkHeight - 4;
    for (int dy = 0; dy < 16; dy++)
    {
        int radius = (dy < 12) ? 8 : 4;
        for (int dx = -radius; dx <= radius; dx++)
        {
            for (int dz = -radius; dz <= radius; dz++)
            {
                int lx = tx + dx, ly = leafBase + dy, lz = tz + dz;
                if (lx < 0 || lx >= SIZE || ly >= SIZE || lz < 0 || lz >= SIZE)
                    continue;
                if (dx == 0 && dz == 0 && dy < trunkHeight)
                    continue; // trunk occupies this
                // Spherical-ish shape: skip corners beyond radius
                if (dx * dx + dz * dz > radius * radius)
                    continue;
                if (at(lx, ly, lz) == AIR)
                    at(lx, ly, lz) = LEAF;
            }
        }
    }
}

void VoxelWorld::uploadToGPU()
{
    if (texture)
        glDeleteTextures(1, &texture);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, SIZE, SIZE, SIZE, 0,
                 GL_RED, GL_UNSIGNED_BYTE, voxels.data());

    glBindTexture(GL_TEXTURE_3D, 0);
}
