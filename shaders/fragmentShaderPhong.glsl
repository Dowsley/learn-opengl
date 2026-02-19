#version 330 core

struct Material {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
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
out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    vec3 ambient = material.ambientColor * light.ambientColor;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(light.position - FragPos);
    float alignmentWithLightSource = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = light.diffuseColor * (alignmentWithLightSource * material.diffuseColor);

    vec3 viewDirection =  normalize(viewPos - FragPos);
    vec3 reflectionDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectionDir), 0.0), material.shininess);
    vec3 specular = light.specularColor * (spec * material.specularColor);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
