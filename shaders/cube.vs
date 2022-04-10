#version 330 core
layout (location = 0) in vec3 aPos;

//World position
out vec3 Pos;
uniform mat4 mvp;
uniform mat4 model;
void main()
{
    Pos = vec3(model * vec4(aPos, 1.0));
    gl_Position = mvp * vec4(aPos, 1.0);
}