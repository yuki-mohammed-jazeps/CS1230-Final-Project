#include "lighting.h"

using std::vector;
using glm::vec3;

lighting::lighting() : num_lights(8), valid(false), update(true) {}

void lighting::initialize(GLuint program_id) {
  ka_u = glGetUniformLocation(program_id, "ka");
  kd_u = glGetUniformLocation(program_id, "kd");
  ks_u = glGetUniformLocation(program_id, "ks");

  for (int i = 0; i != num_lights; ++i) {
    auto idx_str = "[" + std::to_string(i) + "]";

    light_dir_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_directions" + idx_str).c_str()));
    light_pos_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_positions" + idx_str).c_str()));
    light_col_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_colors" + idx_str).c_str()));
    light_typ_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_types" + idx_str).c_str()));
    light_ang_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_angles" + idx_str).c_str()));
    light_pen_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_penumbras" + idx_str).c_str()));
    light_fun_uniforms.push_back(
      glGetUniformLocation(program_id,
                           ("light_functions" + idx_str).c_str()));
  }

  update = true;
}

void lighting::set_data(const vector<SceneLightData> &master_data,
  float ka_p, float kd_p, float ks_p) {
  valid  = true;
  lights = &master_data;
  ka = ka_p;
  kd = kd_p;
  ks = ks_p;

  update = true;
}

void lighting::send_uniforms() {
  if (!valid)
    return;

  // Send global parameters
  glUniform1f(ka_u, ka);
  glUniform1f(kd_u, kd);
  glUniform1f(ks_u, ks);

  // Send Light data
  size_t i = 0;
  for (i = 0; i != num_lights && i != lights->size(); ++i) {
    auto curr_light = (*lights)[i];
    glUniform3fv(light_dir_uniforms[i], 1, &curr_light.dir[0]);
    glUniform3fv(light_pos_uniforms[i], 1, &curr_light.pos[0]);
    glUniform3fv(light_col_uniforms[i], 1, &curr_light.color[0]);
    glUniform1i(light_typ_uniforms[i], static_cast<int>(curr_light.type));
    glUniform1f(light_ang_uniforms[i], curr_light.angle);
    glUniform1f(light_pen_uniforms[i], curr_light.penumbra);
    glUniform3fv(light_fun_uniforms[i], 1, &curr_light.function[0]);
  }

  // Fill empty spaces with 0 directions
  auto zero_vec = vec3(0.f);
  while (i != num_lights) {
    glUniform3fv(light_dir_uniforms[i], 1, &zero_vec[0]);
    glUniform3fv(light_pos_uniforms[i], 1, &zero_vec[0]);
    glUniform3fv(light_col_uniforms[i], 1, &zero_vec[0]);
    glUniform1i(light_typ_uniforms[i], -1);
    glUniform1f(light_ang_uniforms[i], 0.f);
    glUniform1f(light_pen_uniforms[i], 0.f);
    glUniform3fv(light_fun_uniforms[i], 1, &zero_vec[0]);
    ++i;
  }

  update = false;
}
