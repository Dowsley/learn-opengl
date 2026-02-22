#version 330 core

out vec4 FragColor;
in vec2 fragCoord;

uniform mat4 invViewProj;
uniform vec3 camPos;
uniform vec3 resolution;
uniform float worldSize;
uniform vec3 sunDir;
uniform sampler3D voxelTex;

// ============================================================
// Phase 1: Color space helpers
// ============================================================
vec3 sRGBToLinear(vec3 c)
{
    return pow(c, vec3(2.2));
}

// ============================================================
// Hash functions (used by Phase 3 soft shadows + Phase 7 variation)
// ============================================================
float hash31(vec3 p)
{
    p = fract(p * vec3(443.897, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.x + p.y) * p.z);
}

float hash21(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * vec3(443.897, 441.423, 437.195));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

// ============================================================
// Phase 7: Material system
// ============================================================
struct Material
{
    vec3 albedo;
    float roughness;
    float metallic;
    float translucency;
};

Material getBlockMaterial(int blockType, vec3 pos)
{
    Material m;
    m.metallic = 0.0;
    m.translucency = 0.0;

    // Per-voxel color noise: +/-8% variation
    float variation = hash31(floor(pos)) * 0.16 - 0.08;

    if (blockType == 1) {        // grass
        m.albedo = sRGBToLinear(vec3(0.30, 0.65, 0.20));
        m.roughness = 0.95;
    } else if (blockType == 2) { // dirt
        m.albedo = sRGBToLinear(vec3(0.55, 0.35, 0.18));
        m.roughness = 1.0;
    } else if (blockType == 3) { // stone
        m.albedo = sRGBToLinear(vec3(0.50, 0.50, 0.52));
        m.roughness = 0.55;
    } else if (blockType == 4) { // wood
        m.albedo = sRGBToLinear(vec3(0.45, 0.30, 0.15));
        m.roughness = 0.8;
    } else if (blockType == 5) { // leaf
        m.albedo = sRGBToLinear(vec3(0.20, 0.55, 0.15));
        m.roughness = 0.9;
        m.translucency = 0.4;
    } else if (blockType == 6) { // water
        m.albedo = sRGBToLinear(vec3(0.15, 0.30, 0.55));
        m.roughness = 0.05;
        m.metallic = 0.02;
    } else {
        m.albedo = vec3(1.0, 0.0, 1.0); // unknown = magenta
        m.roughness = 1.0;
    }

    m.albedo *= (1.0 + variation);
    return m;
}

// ============================================================
// Phase 7: GGX Specular BRDF
// ============================================================
float distributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (3.14159265 * denom * denom);
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ============================================================
// Ray-AABB intersection for box [0, worldSize]^3
// ============================================================
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

// DDA ray cast — returns true if ray hits a solid voxel within maxSteps.
bool castRay(vec3 origin, vec3 dir, int maxSteps)
{
    ivec3 mapPos = ivec3(floor(origin));
    vec3 deltaDist = abs(vec3(1.0) / dir);
    ivec3 stepDir = ivec3(sign(dir));
    vec3 sideDist = (sign(dir) * (vec3(mapPos) - origin) + sign(dir) * 0.5 + 0.5) * deltaDist;
    int iWorldSize = int(worldSize);

    for (int i = 0; i < maxSteps; i++)
    {
        if (mapPos.x < 0 || mapPos.y < 0 || mapPos.z < 0 ||
            mapPos.x >= iWorldSize || mapPos.y >= iWorldSize || mapPos.z >= iWorldSize)
            return false;

        if (texelFetch(voxelTex, mapPos, 0).r > 0.0)
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

// ============================================================
// Phase 4: Atmospheric sky with sun disk
// ============================================================
vec3 computeSky(vec3 rd)
{
    // Base sky gradient (power curve for more natural falloff)
    vec3 skyTop = sRGBToLinear(vec3(0.25, 0.45, 0.85));
    vec3 skyBot = sRGBToLinear(vec3(0.65, 0.78, 0.95));
    float skyT = pow(max(rd.y, 0.0), 0.4);
    vec3 sky = mix(skyBot, skyTop, skyT);

    // Horizon warmth tint
    float horizonFactor = pow(1.0 - abs(rd.y), 8.0);
    sky = mix(sky, sRGBToLinear(vec3(0.9, 0.7, 0.5)), horizonFactor * 0.3);

    // Below-horizon darkening
    if (rd.y < 0.0)
        sky *= 1.0 + rd.y * 0.5;

    // Sun disk (HDR bright)
    float sunDot = dot(rd, sunDir);
    float sunDisk = smoothstep(0.9997, 0.9999, sunDot);
    sky += vec3(10.0, 8.0, 6.0) * sunDisk;

    // Tight sun glow
    float sunGlow = pow(max(sunDot, 0.0), 128.0);
    sky += vec3(1.0, 0.6, 0.3) * sunGlow * 2.0;

    // Broad warm halo
    float sunHalo = pow(max(sunDot, 0.0), 8.0);
    sky += vec3(0.8, 0.5, 0.2) * sunHalo * 0.15;

    return sky;
}

// ============================================================
// Main
// ============================================================
void main()
{
    // Reconstruct ray from screen coordinate
    vec2 ndc = fragCoord * 2.0 - 1.0;
    vec4 nearClip = invViewProj * vec4(ndc, -1.0, 1.0);
    vec4 farClip  = invViewProj * vec4(ndc,  1.0, 1.0);
    vec3 near = nearClip.xyz / nearClip.w;
    vec3 far  = farClip.xyz / farClip.w;
    vec3 rd = normalize(far - near);
    vec3 ro = camPos;

    // Phase 4: Atmospheric sky for this ray direction
    vec3 skyColor = computeSky(rd);

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
    vec3 sideDist = (sign(rd) * (vec3(mapPos) - pos) + sign(rd) * 0.5 + 0.5) * deltaDist;
    ivec3 mask = ivec3(0);

    int iWorldSize = int(worldSize);
    int maxSteps = iWorldSize * 3;
    int blockType = 0;

    for (int i = 0; i < maxSteps; i++)
    {
        blockType = sampleVoxel(mapPos);
        if (blockType != 0) break;

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

        if (mapPos.x < 0 || mapPos.y < 0 || mapPos.z < 0 ||
            mapPos.x >= iWorldSize || mapPos.y >= iWorldSize || mapPos.z >= iWorldSize)
            break;
    }

    if (blockType == 0)
    {
        FragColor = vec4(skyColor, 1.0);
        return;
    }

    // Face normal from last DDA step
    vec3 normal = -vec3(mask) * vec3(step);

    // Phase 7: Get material properties
    Material mat = getBlockMaterial(blockType, vec3(mapPos));

    // ==========================================================
    // Phase 2: Enhanced AO (Minecraft smooth lighting)
    // ==========================================================

    // Compute exact hit point on voxel face for sub-voxel UV
    float tHit;
    if (mask.x == 1) {
        float faceX = float(mapPos.x) + (step.x < 0 ? 1.0 : 0.0);
        tHit = (faceX - ro.x) / rd.x;
    } else if (mask.y == 1) {
        float faceY = float(mapPos.y) + (step.y < 0 ? 1.0 : 0.0);
        tHit = (faceY - ro.y) / rd.y;
    } else {
        float faceZ = float(mapPos.z) + (step.z < 0 ? 1.0 : 0.0);
        tHit = (faceZ - ro.z) / rd.z;
    }
    vec3 hitPos = ro + rd * tHit;
    vec3 localPos = hitPos - vec3(mapPos);

    // Build tangent frame on the face
    vec3 absNormal = abs(normal);
    ivec3 iNormal = ivec3(absNormal);
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

    // Sub-voxel UV on the face [0,1]
    vec2 faceUV;
    if (iNormal.y == 1)      faceUV = clamp(localPos.xz, 0.0, 0.999);
    else if (iNormal.x == 1) faceUV = clamp(localPos.yz, 0.0, 0.999);
    else                     faceUV = clamp(localPos.xy, 0.0, 0.999);

    ivec3 neighborBase = mapPos + ivec3(normal);

    // Sample 4 edges and 4 corners
    int e1 = sampleVoxel(neighborBase + tangent1) != 0 ? 1 : 0; // +tangent1
    int e2 = sampleVoxel(neighborBase - tangent1) != 0 ? 1 : 0; // -tangent1
    int e3 = sampleVoxel(neighborBase + tangent2) != 0 ? 1 : 0; // +tangent2
    int e4 = sampleVoxel(neighborBase - tangent2) != 0 ? 1 : 0; // -tangent2
    int c1 = sampleVoxel(neighborBase + tangent1 + tangent2) != 0 ? 1 : 0;
    int c2 = sampleVoxel(neighborBase + tangent1 - tangent2) != 0 ? 1 : 0;
    int c3 = sampleVoxel(neighborBase - tangent1 + tangent2) != 0 ? 1 : 0;
    int c4 = sampleVoxel(neighborBase - tangent1 - tangent2) != 0 ? 1 : 0;

    // Minecraft corner occlusion rule: corner only counts if adjacent edge is occluded
    // AO at each face corner
    float ao00 = 1.0 - 0.25 * float(e2 + e4 + ((e2 == 1 || e4 == 1) ? c4 : 0));
    float ao10 = 1.0 - 0.25 * float(e1 + e4 + ((e1 == 1 || e4 == 1) ? c2 : 0));
    float ao01 = 1.0 - 0.25 * float(e2 + e3 + ((e2 == 1 || e3 == 1) ? c3 : 0));
    float ao11 = 1.0 - 0.25 * float(e1 + e3 + ((e1 == 1 || e3 == 1) ? c1 : 0));

    // Bilinear interpolation across the face
    float aoFactor = mix(mix(ao00, ao10, faceUV.x), mix(ao01, ao11, faceUV.x), faceUV.y);

    // ==========================================================
    // Phase 3: Soft shadows (fixed-pattern multi-sample)
    // ==========================================================
    float dist = length(vec3(mapPos) - ro);
    float shadowFactor = 1.0;
    float NdotL = max(dot(normal, sunDir), 0.0);

    if (NdotL > 0.0 && dist < worldSize * 0.75)
    {
        vec3 shadowOrigin = vec3(mapPos) + 0.5 + normal * 0.51;

        // Build tangent frame around sun direction
        vec3 sunTangent = normalize(cross(sunDir, abs(sunDir.y) < 0.9 ? vec3(0,1,0) : vec3(1,0,0)));
        vec3 sunBitangent = cross(sunDir, sunTangent);

        // 3 fixed directions: center + 2 opposing offsets at 120-degree spacing
        // No per-pixel randomness = spatially coherent shadow edges
        float sunRadius = 0.035;
        int shadowHits = 0;

        // Center ray
        if (castRay(shadowOrigin, sunDir, 48))
            shadowHits++;

        // Two offset rays (opposing, perpendicular to each other)
        vec3 sDir1 = normalize(sunDir + sunTangent * sunRadius * 0.866 + sunBitangent * sunRadius * 0.5);
        vec3 sDir2 = normalize(sunDir - sunTangent * sunRadius * 0.866 - sunBitangent * sunRadius * 0.5);

        if (castRay(shadowOrigin, sDir1, 48))
            shadowHits++;
        if (castRay(shadowOrigin, sDir2, 48))
            shadowHits++;

        shadowFactor = 1.0 - float(shadowHits) / 3.0;
        shadowFactor = smoothstep(0.0, 1.0, shadowFactor);
    }

    // ==========================================================
    // Phase 1: Linear-space PBR lighting
    // ==========================================================
    vec3 sunColor = vec3(1.0, 0.95, 0.85) * 1.8; // warm sun, high intensity

    // Hemisphere ambient (sky blue above, warm brown below)
    vec3 skyAmbient = sRGBToLinear(vec3(0.30, 0.35, 0.50));
    vec3 groundAmbient = sRGBToLinear(vec3(0.15, 0.10, 0.08));
    float hemiBlend = normal.y * 0.5 + 0.5;
    vec3 hemiAmbient = mix(groundAmbient, skyAmbient, hemiBlend);

    // ==========================================================
    // Phase 7: PBR lighting (GGX specular + diffuse)
    // ==========================================================
    vec3 viewDir = normalize(ro - (vec3(mapPos) + 0.5));
    vec3 halfVec = normalize(sunDir + viewDir);
    float NdotV = max(dot(normal, viewDir), 0.001);
    float NdotH = max(dot(normal, halfVec), 0.0);

    vec3 F0 = mix(vec3(0.04), mat.albedo, mat.metallic);

    // Diffuse component
    vec3 diffuse = mat.albedo * (1.0 - mat.metallic);

    // Specular: GGX (Trowbridge-Reitz NDF + Schlick Fresnel + Smith geometry)
    float D = distributionGGX(NdotH, mat.roughness);
    float G = geometrySmith(NdotV, NdotL, mat.roughness);
    vec3 F = fresnelSchlick(max(dot(halfVec, viewDir), 0.0), F0);
    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);

    // Energy conservation
    vec3 kD = (vec3(1.0) - F) * (1.0 - mat.metallic);
    vec3 directLight = (kD * diffuse / 3.14159265 + specular) * sunColor * NdotL * shadowFactor;
    vec3 ambient = hemiAmbient * mat.albedo * aoFactor;

    // Phase 7: Leaf translucency (wrap lighting for backlit glow)
    if (mat.translucency > 0.0)
    {
        float wrapLight = max(dot(-normal, sunDir), 0.0) * 0.5;
        vec3 transColor = mat.albedo * sunColor * wrapLight * mat.translucency * shadowFactor;
        directLight += transColor;
    }

    vec3 color = ambient + directLight;

    // Fog blends toward per-ray sky color
    float fogFactor = clamp(dist / (worldSize * 2.0), 0.0, 1.0);
    fogFactor = fogFactor * fogFactor;
    color = mix(color, skyColor, fogFactor);

    // Output raw linear HDR — tone mapping + gamma in post-process shader
    FragColor = vec4(color, 1.0);
}
