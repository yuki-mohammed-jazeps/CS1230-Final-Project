#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 uv;

out vec3 vec_pos;
out vec3 vec_nor;
out vec2 vec_uv;

uniform mat4 model_matrix;
uniform mat4 inv_model_matrix;
uniform mat4 pv_matrix;

void main() {
  vec_pos = vec3(model_matrix * vec4(pos, 1));
  vec_nor = normalize(transpose(mat3(inv_model_matrix)) * nor);
  vec_uv  = uv;

  gl_Position = pv_matrix * model_matrix * vec4(pos, 1);
}
