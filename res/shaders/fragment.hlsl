#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 FragPos;

out vec4 FragColor;
uniform vec4 color;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRadius;
uniform float lightIntensity;
uniform int castShadows;
uniform int hasTexture;

uniform sampler2D textureSampler;

void main()
{
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float distance = length(lightPosition - FragPos);
    float attenuation = 1.0 / (1.0 + (0.09 / lightRadius) * distance + (0.032 / (lightRadius * lightRadius)) * distance * distance);

    // Ambient
    vec3 ambient = 0.1 * lightColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * lightColor;
    
    // Combine results
    vec3 lighting = (ambient + diffuse + specular) * lightIntensity * attenuation;
    vec4 textureColor = (hasTexture == 1) ? texture(textureSampler, vTexCoord) : color;
    FragColor = vec4(lighting, 1.0) * textureColor;
}