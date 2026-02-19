#version 330 core

in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 normal = normalize(Normal);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 lightDirection = normalize(lightPos - FragPos);
    float alignmentWithLightSource = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = alignmentWithLightSource * lightColor;

    float specularStrength = 0.5;
    vec3 viewDirection =  normalize(viewPos - FragPos);
    vec3 reflectionDir = reflect(-lightDirection, normal);

    float shininess = 32;
    float spec = pow(max(dot(viewDirection, reflectionDir), 0.0), shininess);

    vec3 result = (ambient + diffuse + spec) * objectColor;
    FragColor = vec4(result, 1.0);
}
