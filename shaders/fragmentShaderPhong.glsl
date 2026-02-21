#version 330 core

// material textures
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform float shininess;

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    float constantAttTerm;
    float linearAttTerm;
    float quadraticAttTerm;
};

struct DirLight {
    vec3 direction;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct PointLight {
    vec3 position;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    float constantAttTerm;
    float linearAttTerm;
    float quadraticAttTerm;
};
#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 CalcAmbient(vec3 ambientColor);
vec3 CalcDiffuse(vec3 diffuseColor, vec3 lightDir, vec3 normal);
vec3 CalcSpecular(vec3 specularColor, vec3 reflectionDir, vec3 viewDir);
float CalcAttenuation(vec3 position, vec3 fragPos, float constantAttTerm, float linearAttTerm, float quadraticAttTerm);

void main()
{
    float alpha = texture(texture_diffuse0, TexCoords).a;
    if (alpha < 0.1)
        discard;

    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir);
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], normal, FragPos, viewDir);
    }
    result += CalcSpotLight(spotLight, normal, FragPos, viewDir);

    FragColor = vec4(result, texture(texture_diffuse0, TexCoords).a);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient = CalcAmbient(light.ambientColor);
    vec3 diffuse = CalcDiffuse(light.diffuseColor, lightDir, normal);
    vec3 specular = CalcSpecular(light.specularColor, reflect(-lightDir, normal), viewDir);

    return ambient + diffuse + specular;
}

vec3 CalcAmbient(vec3 ambientColor)
{
    return ambientColor * vec3(texture(texture_diffuse0, TexCoords));
}

vec3 CalcDiffuse(vec3 diffuseColor, vec3 lightDir, vec3 normal)
{
    float alignmentWithLightSource = max(dot(normal, lightDir), 0.0);
    return diffuseColor * alignmentWithLightSource * vec3(texture(texture_diffuse0, TexCoords));
}

vec3 CalcSpecular(vec3 specularColor, vec3 reflectionDir, vec3 viewDir)
{
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), shininess);
    return specularColor * spec * vec3(texture(texture_specular0, TexCoords));
}

float CalcAttenuation(vec3 position, vec3 fragPos, float constantAttTerm, float linearAttTerm, float quadraticAttTerm)
{
    float d = length(position - fragPos);
    return 1.0 / (constantAttTerm + linearAttTerm*d + quadraticAttTerm*d*d);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 reflectionDir = reflect(-lightDir, normal);

    vec3 ambient = CalcAmbient(light.ambientColor);
    vec3 diffuse = CalcDiffuse(light.diffuseColor, lightDir, normal);
    vec3 specular = CalcSpecular(light.specularColor, reflectionDir, viewDir);

    float attenuation = CalcAttenuation(light.position, fragPos, light.constantAttTerm, light.linearAttTerm, light.quadraticAttTerm);
    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 reflectionDir = reflect(-lightDir, normal);

    vec3 ambient = CalcAmbient(light.ambientColor);
    vec3 diffuse = CalcDiffuse(light.diffuseColor, lightDir, normal);
    vec3 specular = CalcSpecular(light.specularColor, reflectionDir, viewDir);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float attenuation = CalcAttenuation(light.position, fragPos, light.constantAttTerm, light.linearAttTerm, light.quadraticAttTerm);

    return (ambient + diffuse + specular) * attenuation * intensity;
}
