#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 uv;

// Parallax mapping
layout(location = 3) in vec3 tan;
out mat3 tangent_matrix;
uniform bool parallax_vert;

out vec3 vec_pos;
out vec3 vec_nor;
out vec2 vec_uv;

uniform mat4 model_matrix;
uniform mat4 inv_model_matrix;
uniform mat4 pv_matrix;

// Texture repeats
uniform float u_repeat;
uniform float v_repeat;

void main() {
  vec_pos = vec3(model_matrix * vec4(pos, 1));
  vec_nor = normalize(transpose(mat3(inv_model_matrix)) * nor);
  vec_uv  = uv * vec2(u_repeat, v_repeat);

  // Parallax mapping
  if (parallax_vert) {
    vec3 w_tan     = normalize(transpose(mat3(inv_model_matrix)) * tan);
    w_tan          = normalize(w_tan - vec_nor * dot(w_tan, vec_nor));
    vec3 w_bit     = cross(vec_nor, w_tan);
    tangent_matrix = mat3(w_tan, w_bit, vec_nor);
  }

  gl_Position = pv_matrix * model_matrix * vec4(pos, 1);
}
