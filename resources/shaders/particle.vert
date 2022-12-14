#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 aOffset;

out vec3 fColor;

uniform mat4 mvp;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float quadSize;

out vec2 frag_uv;
out float life;
out float coordY;

void main()
{
    vec3 obj_pos = vec3(aOffset) + cameraRight * aPos.x + cameraUp * aPos.y;
    frag_uv = uv;
    life = aOffset[3];
    coordY = aOffset[1];
    gl_Position = mvp * vec4(obj_pos, 1.0f);
}
