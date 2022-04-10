#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 WorldPos;

uniform mat4 mvp;
uniform mat4 model;

void main()
{
    Normal = transpose(inverse(mat3(model))) * aNormal;
    WorldPos = vec3(model * vec4(aPos, 1.0)); 
    gl_Position = mvp * vec4(aPos, 1.0);
}          