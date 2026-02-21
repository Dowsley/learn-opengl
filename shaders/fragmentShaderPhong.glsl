#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D emissionMap;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    vec3 ambient = light.ambientColor * vec3(texture(material.diffuseMap, TexCoords));

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(light.position - FragPos);
    float alignmentWithLightSource = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.diffuseColor * alignmentWithLightSource * vec3(texture(material.diffuseMap, TexCoords));

    vec3 viewDirection =  normalize(viewPos - FragPos);
    vec3 reflectionDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectionDir), 0.0), material.shininess);
    vec3 specular = light.specularColor * spec * vec3(texture(material.specularMap, TexCoords));

    vec3 emission = texture(material.emissionMap, TexCoords).rgb;

    vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1.0);
}
