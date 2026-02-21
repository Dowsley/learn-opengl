#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D emissionMap;
    float shininess;
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
#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec3 CalcAmbient(vec3 ambientColor);
vec3 CalcDiffuse(vec3 diffuseColor, vec3 lightDir, vec3 normal);
vec3 CalcSpecular(vec3 specularColor, vec3 reflectionDir, vec3 viewDir);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir);
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], normal, FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0);
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
    return ambientColor * vec3(texture(material.diffuseMap, TexCoords));
}

vec3 CalcDiffuse(vec3 diffuseColor, vec3 lightDir, vec3 normal)
{
    float alignmentWithLightSource = max(dot(normal, lightDir), 0.0);
    return diffuseColor * alignmentWithLightSource * vec3(texture(material.diffuseMap, TexCoords));
}

vec3 CalcSpecular(vec3 specularColor, vec3 reflectionDir, vec3 viewDir)
{
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    return specularColor * spec * vec3(texture(material.specularMap, TexCoords));
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 reflectionDir = reflect(-lightDir, normal);

    vec3 ambient = CalcAmbient(light.ambientColor);
    vec3 diffuse = CalcDiffuse(light.diffuseColor, lightDir, normal);
    vec3 specular = CalcSpecular(light.specularColor, reflectionDir, viewDir);

    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constantAttTerm + light.linearAttTerm*d + light.quadraticAttTerm*d*d);

    return (ambient + diffuse + specular) * attenuation;
}
