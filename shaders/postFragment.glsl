#version 330 core

out vec4 FragColor;
in vec2 fragCoord;

uniform sampler2D sceneTex;
uniform sampler2D bloomTex;
uniform int passMode;
uniform vec3 texelSize; // xy = 1.0/resolution of source texture

// ACES filmic tone mapping
vec3 acesToneMap(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// 5-tap Gaussian weights
const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    if (passMode == 0)
    {
        // Pass 0: Bright pixel extraction (scene HDR -> bloom buffer)
        vec3 color = texture(sceneTex, fragCoord).rgb;
        float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
        FragColor = vec4(color * smoothstep(1.0, 2.0, brightness), 1.0);
    }
    else if (passMode == 1)
    {
        // Pass 1: Horizontal Gaussian blur
        vec3 result = texture(sceneTex, fragCoord).rgb * weights[0];
        for (int i = 1; i < 5; i++)
        {
            result += texture(sceneTex, fragCoord + vec2(texelSize.x * float(i), 0.0)).rgb * weights[i];
            result += texture(sceneTex, fragCoord - vec2(texelSize.x * float(i), 0.0)).rgb * weights[i];
        }
        FragColor = vec4(result, 1.0);
    }
    else if (passMode == 2)
    {
        // Pass 2: Vertical Gaussian blur
        vec3 result = texture(sceneTex, fragCoord).rgb * weights[0];
        for (int i = 1; i < 5; i++)
        {
            result += texture(sceneTex, fragCoord + vec2(0.0, texelSize.y * float(i))).rgb * weights[i];
            result += texture(sceneTex, fragCoord - vec2(0.0, texelSize.y * float(i))).rgb * weights[i];
        }
        FragColor = vec4(result, 1.0);
    }
    else
    {
        // Pass 3: Final composite — bloom + tone map + vignette + gamma
        vec3 scene = texture(sceneTex, fragCoord).rgb;
        vec3 bloom = texture(bloomTex, fragCoord).rgb;

        vec3 color = scene + bloom * 0.3;

        // ACES tone mapping
        color = acesToneMap(color);

        // Subtle color grading (warm highlights, cool shadows)
        color = mix(color, color * vec3(1.02, 0.99, 0.96), 0.3);

        // Vignette
        vec2 uv = fragCoord * 2.0 - 1.0;
        float vignette = 1.0 - dot(uv * 0.5, uv * 0.5);
        vignette = smoothstep(0.0, 1.0, vignette);
        color *= vignette;

        // Gamma correction (linear -> sRGB)
        color = pow(color, vec3(1.0 / 2.2));

        FragColor = vec4(color, 1.0);
    }
}
