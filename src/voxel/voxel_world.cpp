#include "voxel_world.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Hash-based value noise (no external dependencies)
static float hash(int x, int z)
{
    int n = x * 73856093 ^ z * 19349663;
    n = (n << 13) ^ n;
    return 1.0f - static_cast<float>((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
}

static float smoothNoise(float x, float z)
{
    int ix = static_cast<int>(std::floor(x));
    int iz = static_cast<int>(std::floor(z));
    float fx = x - std::floor(x);
    float fz = z - std::floor(z);

    // Quintic interpolation (Perlin's improved curve)
    fx = fx * fx * fx * (fx * (fx * 6.0f - 15.0f) + 10.0f);
    fz = fz * fz * fz * (fz * (fz * 6.0f - 15.0f) + 10.0f);

    float v00 = hash(ix, iz);
    float v10 = hash(ix + 1, iz);
    float v01 = hash(ix, iz + 1);
    float v11 = hash(ix + 1, iz + 1);

    float i0 = v00 + (v10 - v00) * fx;
    float i1 = v01 + (v11 - v01) * fx;
    return i0 + (i1 - i0) * fz;
}

static float fbm(float x, float z, int octaves)
{
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++)
    {
        value += smoothNoise(x * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return value / maxValue; // normalized to [-1, 1]
}

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
    for (int x = 0; x < SIZE; x++)
    {
        for (int z = 0; z < SIZE; z++)
        {
            float fx = static_cast<float>(x) / SIZE;
            float fz = static_cast<float>(z) / SIZE;

            // Continental: large rolling terrain features
            float continental = fbm(fx * 3.0f, fz * 3.0f, 6);

            // Detail: small bumps and texture
            float detail = fbm(fx * 12.0f + 100.0f, fz * 12.0f + 100.0f, 4);

            // Ridged: sharp peaks at noise zero-crossings
            float ridgeBase = fbm(fx * 5.0f + 200.0f, fz * 5.0f + 200.0f, 5);
            float ridged = 1.0f - std::abs(ridgeBase);
            ridged = ridged * ridged; // sharpen ridges

            float height = 60.0f + continental * 32.0f + detail * 8.0f + ridged * 18.0f;

            int h = std::max(1, std::min(static_cast<int>(height), SIZE - 1));

            for (int y = 0; y < h; y++)
            {
                if (y == h - 1 && h > WATER_LEVEL)
                    at(x, y, z) = GRASS;
                else if (y >= h - 4)
                    at(x, y, z) = DIRT;
                else
                    at(x, y, z) = STONE;
            }

            // Fill water in low areas
            if (h < WATER_LEVEL)
            {
                for (int y = h; y < WATER_LEVEL; y++)
                    at(x, y, z) = WATER;
            }
        }
    }

    // Scatter more trees (80 instead of 30)
    std::srand(42);
    for (int i = 0; i < 80; i++)
    {
        int tx = std::rand() % (SIZE - 24) + 12;
        int tz = std::rand() % (SIZE - 24) + 12;
        placeTree(tx, tz);
    }
}

void VoxelWorld::placeTree(int tx, int tz)
{
    // Find ground height (skip air and water)
    int ground = 0;
    for (int y = SIZE - 1; y >= 0; y--)
    {
        uint8_t block = at(tx, y, tz);
        if (block != AIR && block != WATER)
        {
            ground = y + 1;
            break;
        }
    }

    // Only place on land well above water, with room for full tree
    if (ground <= WATER_LEVEL + 2 || ground + 44 >= SIZE) return;

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
