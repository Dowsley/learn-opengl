#version 330 core

out vec4 FragColor;
in vec2 fragCoord;

uniform mat4 invViewProj;
uniform vec3 camPos;
uniform vec3 resolution;
uniform float worldSize;
uniform vec3 sunDir;
uniform sampler3D voxelTex;

// Block palette
vec3 getBlockColor(int blockType)
{
    if (blockType == 1) return vec3(0.30, 0.65, 0.20); // grass
    if (blockType == 2) return vec3(0.55, 0.35, 0.18); // dirt
    if (blockType == 3) return vec3(0.50, 0.50, 0.52); // stone
    if (blockType == 4) return vec3(0.45, 0.30, 0.15); // wood
    if (blockType == 5) return vec3(0.20, 0.55, 0.15); // leaf
    return vec3(1.0, 0.0, 1.0); // unknown = magenta
}

// Ray-AABB intersection for box [0, worldSize]^3
bool rayAABB(vec3 ro, vec3 rd, out float tMin, out float tMax)
{
    vec3 invRd = 1.0 / rd;
    vec3 t0 = (vec3(0.0) - ro) * invRd;
    vec3 t1 = (vec3(worldSize) - ro) * invRd;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    tMin = max(max(tmin.x, tmin.y), tmin.z);
    tMax = min(min(tmax.x, tmax.y), tmax.z);

    return tMax >= max(tMin, 0.0);
}

// Sample the voxel grid. Returns block type (0 = air).
int sampleVoxel(ivec3 pos)
{
    if (pos.x < 0 || pos.y < 0 || pos.z < 0) return 0;
    if (pos.x >= int(worldSize) || pos.y >= int(worldSize) || pos.z >= int(worldSize)) return 0;
    float val = texelFetch(voxelTex, pos, 0).r;
    return int(val * 255.0 + 0.5);
}

// DDA ray cast helper — returns true if ray hits a solid voxel within maxSteps
bool castRay(vec3 origin, vec3 dir, int maxSteps)
{
    vec3 pos = origin;
    ivec3 mapPos = ivec3(floor(pos));
    vec3 deltaDist = abs(vec3(1.0) / dir);
    ivec3 stepDir = ivec3(sign(dir));
    vec3 sideDist = (sign(dir) * (vec3(mapPos) - pos) + sign(dir) * 0.5 + 0.5) * deltaDist;

    for (int i = 0; i < maxSteps; i++)
    {
        if (mapPos.x < 0 || mapPos.y < 0 || mapPos.z < 0 ||
            mapPos.x >= int(worldSize) || mapPos.y >= int(worldSize) || mapPos.z >= int(worldSize))
            return false;

        if (sampleVoxel(mapPos) != 0)
            return true;

        if (sideDist.x < sideDist.y)
        {
            if (sideDist.x < sideDist.z)
            {
                sideDist.x += deltaDist.x;
                mapPos.x += stepDir.x;
            }
            else
            {
                sideDist.z += deltaDist.z;
                mapPos.z += stepDir.z;
            }
        }
        else
        {
            if (sideDist.y < sideDist.z)
            {
                sideDist.y += deltaDist.y;
                mapPos.y += stepDir.y;
            }
            else
            {
                sideDist.z += deltaDist.z;
                mapPos.z += stepDir.z;
            }
        }
    }

    return false;
}

void main()
{
    // Reconstruct ray from screen coordinate using invViewProj
    vec2 ndc = fragCoord * 2.0 - 1.0;
    vec4 nearClip = invViewProj * vec4(ndc, -1.0, 1.0);
    vec4 farClip  = invViewProj * vec4(ndc,  1.0, 1.0);
    vec3 near = nearClip.xyz / nearClip.w;
    vec3 far  = farClip.xyz / farClip.w;
    vec3 rd = normalize(far - near);
    vec3 ro = camPos;

    // Sky gradient for misses
    vec3 skyTop = vec3(0.35, 0.55, 0.90);
    vec3 skyBot = vec3(0.75, 0.85, 0.95);
    float skyT = rd.y * 0.5 + 0.5;
    vec3 skyColor = mix(skyBot, skyTop, skyT);

    float tMin, tMax;
    if (!rayAABB(ro, rd, tMin, tMax))
    {
        FragColor = vec4(skyColor, 1.0);
        return;
    }

    // Clamp max traversal distance
    float maxDist = worldSize * 1.5;
    tMax = min(tMax, max(tMin, 0.0) + maxDist);

    // Advance ray to AABB entry (or start at camera if inside)
    float t = max(tMin, 0.0) + 0.001;
    vec3 pos = ro + rd * t;

    // DDA setup (Amanatides & Woo)
    ivec3 mapPos = ivec3(floor(pos));
    vec3 deltaDist = abs(vec3(1.0) / rd);
    ivec3 step = ivec3(sign(rd));

    // Distance to next voxel boundary along each axis
    vec3 sideDist = (sign(rd) * (vec3(mapPos) - pos) + sign(rd) * 0.5 + 0.5) * deltaDist;

    // Track which face was hit for normal calculation
    ivec3 mask = ivec3(0);

    int maxSteps = int(worldSize) * 3;
    int blockType = 0;

    for (int i = 0; i < maxSteps; i++)
    {
        blockType = sampleVoxel(mapPos);
        if (blockType != 0) break;

        // Step along the axis with smallest sideDist
        if (sideDist.x < sideDist.y)
        {
            if (sideDist.x < sideDist.z)
            {
                if (sideDist.x > maxDist) break;
                sideDist.x += deltaDist.x;
                mapPos.x += step.x;
                mask = ivec3(1, 0, 0);
            }
            else
            {
                if (sideDist.z > maxDist) break;
                sideDist.z += deltaDist.z;
                mapPos.z += step.z;
                mask = ivec3(0, 0, 1);
            }
        }
        else
        {
            if (sideDist.y < sideDist.z)
            {
                if (sideDist.y > maxDist) break;
                sideDist.y += deltaDist.y;
                mapPos.y += step.y;
                mask = ivec3(0, 1, 0);
            }
            else
            {
                if (sideDist.z > maxDist) break;
                sideDist.z += deltaDist.z;
                mapPos.z += step.z;
                mask = ivec3(0, 0, 1);
            }
        }

        // Check if we left the grid
        if (mapPos.x < 0 || mapPos.y < 0 || mapPos.z < 0 ||
            mapPos.x >= int(worldSize) || mapPos.y >= int(worldSize) || mapPos.z >= int(worldSize))
            break;
    }

    if (blockType == 0)
    {
        FragColor = vec4(skyColor, 1.0);
        return;
    }

    // Compute face normal from the last step
    vec3 normal = -vec3(mask) * vec3(step);

    // --- 1. Sun shadow ray ---
    float shadowFactor = 1.0;
    float diffuse = max(dot(normal, sunDir), 0.0);
    if (diffuse > 0.0)
    {
        // Start shadow ray from the hit face, offset slightly into air
        vec3 shadowOrigin = vec3(mapPos) + 0.5 + normal * 0.51;
        if (castRay(shadowOrigin, sunDir, 64))
            shadowFactor = 0.0;
    }

    // --- 2. Voxel ambient occlusion ---
    // Sample 4 edge-adjacent voxels on the hit face plane
    vec3 absNormal = abs(normal);
    ivec3 iNormal = ivec3(absNormal);
    // Build two tangent directions on the face
    ivec3 tangent1, tangent2;
    if (iNormal.y == 1) {
        tangent1 = ivec3(1, 0, 0);
        tangent2 = ivec3(0, 0, 1);
    } else if (iNormal.x == 1) {
        tangent1 = ivec3(0, 1, 0);
        tangent2 = ivec3(0, 0, 1);
    } else {
        tangent1 = ivec3(1, 0, 0);
        tangent2 = ivec3(0, 1, 0);
    }

    ivec3 neighborBase = mapPos + ivec3(normal);
    int occluded = 0;
    if (sampleVoxel(neighborBase + tangent1) != 0) occluded++;
    if (sampleVoxel(neighborBase - tangent1) != 0) occluded++;
    if (sampleVoxel(neighborBase + tangent2) != 0) occluded++;
    if (sampleVoxel(neighborBase - tangent2) != 0) occluded++;
    float aoFactor = 1.0 - 0.15 * float(occluded);

    // --- 3. Hemisphere ambient lighting ---
    vec3 skyAmbient = vec3(0.30, 0.35, 0.50);   // blue-ish sky
    vec3 groundAmbient = vec3(0.15, 0.10, 0.08); // warm brown ground
    float hemiBlend = normal.y * 0.5 + 0.5; // 1 = up, 0.5 = side, 0 = down
    vec3 hemiAmbient = mix(groundAmbient, skyAmbient, hemiBlend);

    // --- 4. Combine lighting ---
    vec3 color = getBlockColor(blockType) * (hemiAmbient * aoFactor + diffuse * shadowFactor * 0.75);

    // Slight fog based on distance
    float dist = length(vec3(mapPos) - ro);
    float fogFactor = clamp(dist / (worldSize * 2.0), 0.0, 1.0);
    fogFactor = fogFactor * fogFactor; // quadratic falloff
    color = mix(color, skyColor, fogFactor);

    FragColor = vec4(color, 1.0);
}
