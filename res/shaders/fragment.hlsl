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
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Calculate distance from fragment to light position
    float distance = length(lightPosition - FragPos);
    
    float attenuation = 1.0 / (1.0 + (0.09 / lightRadius) * distance + (0.032 / (lightRadius * lightRadius)) * distance * distance);
    vec3 result = (ambient + diffuse + specular) * lightIntensity * attenuation;
    
    // Sample the texture color if texture is present, otherwise use the color uniform
    vec4 textureColor = (hasTexture == 1) ? texture(textureSampler, vTexCoord) : color;
    
    // Multiply the texture color by the diffuse and specular components, and add the ambient component
    FragColor = vec4(ambient, 1.0) + textureColor * vec4(diffuse + specular, 1.0);
}