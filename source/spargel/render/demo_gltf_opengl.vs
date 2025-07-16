#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat3 uR;

out vec3 color;

void main()
{
    gl_Position = uMVP * vec4(aPosition, 1.0);
    color = abs(aNormal);
}
