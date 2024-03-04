#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 FragPos;

uniform mat4 viewProjectionMatrix;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = viewProjectionMatrix * modelMatrix * vec4(aPosition, 1.0);
    vTexCoord = aTexCoord;
    vNormal = mat3(transpose(inverse(modelMatrix))) * aNormal;
    FragPos = vec3(modelMatrix * vec4(aPosition, 1.0));
}