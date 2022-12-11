#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 spotLightSpaceMat;

void main()
{
    gl_Position = spotLightSpaceMat * model * vec4(pos, 1.0);
}
