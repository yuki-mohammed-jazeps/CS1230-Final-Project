#include "shapes/sphere.h"

using glm::vec2;
using glm::vec3;
using std::vector;

constexpr float pi = 3.141592653f;

void sphere::update_params(int param1, int param2) {
  vertex_data = vector<float>();
  normal_data = vector<float>();
  m_param1 = param1 < 2 ? 2 : param1;
  m_param2 = param2 < 3 ? 3 : param2;
  set_vertex_data();
}

// Auxiliary function for sphere UVs
static inline vec2 sphere_uv(const vec3 &p, bool last) {
  float u = 0.f;
  float t = atan2(p.z, p.x);

  if (last)
    u = 0.f;
  else if (t < 0.f)
    u = (-t) / (2 * pi);
  else
    u = 1 - (t / (2 * pi));

  return vec2(u, 0.5 + asin(-p.y / 0.5) / pi);
}

void sphere::make_tile(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right,
  vec3 uv_top_left, vec3 uv_top_right, vec3 uv_bot_left,
  vec3 uv_bot_right, bool last) {
  insert_vec3(vertex_data, top_left);
  insert_vec2(uv_data, sphere_uv(uv_top_left, last));
  insert_vec3(normal_data, normalize(top_left));
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, sphere_uv(uv_bot_left, last));
  insert_vec3(normal_data, normalize(bottom_left));
  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, sphere_uv(uv_top_right, false));
  insert_vec3(normal_data, normalize(top_right));

  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, sphere_uv(uv_top_right, false));
  insert_vec3(normal_data, normalize(top_right));
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, sphere_uv(uv_bot_left, last));
  insert_vec3(normal_data, normalize(bottom_left));
  insert_vec3(vertex_data, bottom_right);
  insert_vec2(uv_data, sphere_uv(uv_bot_right, false));
  insert_vec3(normal_data, normalize(bottom_right));
}

void sphere::make_wedge(float currentTheta, float nextTheta, bool last) {
  float phi_step = pi / static_cast<float>(m_param1);
  float curr_phi = 0.f;
  for (size_t i = 0; i != m_param1; ++i) {
    float next_phi = curr_phi + phi_step;

    auto top_left  = vec3(0.5 * sin(curr_phi) * cos(nextTheta),
      0.5 * cos(curr_phi), 0.5 * sin(curr_phi) * sin(nextTheta));
    auto bot_left  = vec3(0.5 * sin(next_phi) * cos(nextTheta),
      0.5 * cos(next_phi), 0.5 * sin(next_phi) * sin(nextTheta));
    auto top_right = vec3(0.5 * sin(curr_phi) * cos(currentTheta),
      0.5 * cos(curr_phi), 0.5 * sin(curr_phi) * sin(currentTheta));
    auto bot_right = vec3(0.5 * sin(next_phi) * cos(currentTheta),
      0.5 * cos(next_phi), 0.5 * sin(next_phi) * sin(currentTheta));

    // Do a cylindrical projection on the sphere
    auto uv_top_left  = vec3(0.5 * cos(nextTheta),
      0.5 * cos(curr_phi), 0.5 * sin(nextTheta));
    auto uv_bot_left  = vec3(0.5 * cos(nextTheta),
      0.5 * cos(next_phi), 0.5 * sin(nextTheta));
    auto uv_top_right = vec3(0.5 * cos(currentTheta),
      0.5 * cos(curr_phi), 0.5 * sin(currentTheta));
    auto uv_bot_right = vec3(0.5 * cos(currentTheta),
      0.5 * cos(next_phi), 0.5 * sin(currentTheta));

    make_tile(top_left, top_right, bot_left, bot_right,
              uv_top_left, uv_top_right, uv_bot_left, uv_bot_right, last);
    curr_phi = next_phi;
  }
}

void sphere::make_sphere() {
  float theta_step = glm::radians(360.f / m_param2);
  float curr_theta = 0 * theta_step;

  for (size_t i = 0; i != m_param2; ++i, curr_theta += theta_step)
    make_wedge(curr_theta, curr_theta + theta_step, i == m_param2 - 1);
}

void sphere::set_vertex_data() {
  make_sphere();
}

void sphere::insert_vec3(vector<float> &data, vec3 v) {
  data.push_back(v.x);
  data.push_back(v.y);
  data.push_back(v.z);
}

void sphere::insert_vec2(vector<float> &data, vec2 v) {
  data.push_back(v.x);
  data.push_back(v.y);
}
