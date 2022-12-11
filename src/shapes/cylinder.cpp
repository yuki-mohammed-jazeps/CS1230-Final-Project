#include "shapes/cylinder.h"

using glm::vec2;
using glm::vec3;
using std::vector;

constexpr float pi = 3.1415926535f;

void cylinder::update_params(int param1, int param2) {
  vertex_data = vector<float>();
  normal_data = vector<float>();
  m_param1 = param1 < 1 ? 1 : param1;
  m_param2 = param2 < 3 ? 3 : param2;
  set_vertex_data();
}

// Auxiliary functions for cap and body UVs
static inline vec2 cap_uv(const vec3 &p) {
  return vec2(p.x + .5f, -p.z + .5f);
}

static inline vec2 body_uv(const vec3 &p, bool last) {
  float u = 0.f;
  float t = atan2(p.z, p.x);

  if (last)
    u = 0.f;
  else if (t < 0.f)
    u = (-t) / (2 * pi);
  else
    u = 1 - (t / ( 2 *pi));

  return vec2(u, p.y - .5f);
}

// Data building funtions
void cylinder::make_cap_tile(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right) {
  vec3 normal = vec3(0, 0, 0);
  normal.y = top_left.y > 0.f ? 1.f : -1.f;

  insert_vec3(vertex_data, top_left);
  insert_vec2(uv_data, cap_uv(top_left));
  insert_vec3(normal_data, normal);
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, cap_uv(bottom_left));
  insert_vec3(normal_data, normal);
  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, cap_uv(top_right));
  insert_vec3(normal_data, normal);

  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, cap_uv(top_right));
  insert_vec3(normal_data, normal);
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, cap_uv(bottom_left));
  insert_vec3(normal_data, normal);
  insert_vec3(vertex_data, bottom_right);
  insert_vec2(uv_data, cap_uv(bottom_right));
  insert_vec3(normal_data, normal);
}

void cylinder::make_body_tile(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right, bool last) {
  vec3 tl_normal = normalize(vec3(top_left.x, 0, top_left.z));
  vec3 tr_normal = normalize(vec3(top_right.x, 0, top_right.z));
  vec3 bl_normal = normalize(vec3(bottom_left.x, 0, bottom_left.z));
  vec3 br_normal = normalize(vec3(bottom_right.x, 0, bottom_right.z));

  insert_vec3(vertex_data, top_left);
  insert_vec2(uv_data, body_uv(top_left, last));
  insert_vec3(normal_data, normalize(tl_normal));
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, body_uv(bottom_left, last));
  insert_vec3(normal_data, normalize(bl_normal));
  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, body_uv(top_right, false));
  insert_vec3(normal_data, normalize(tr_normal));

  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, body_uv(top_right, false));
  insert_vec3(normal_data, normalize(tr_normal));
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, body_uv(bottom_left, last));
  insert_vec3(normal_data, normalize(bl_normal));
  insert_vec3(vertex_data, bottom_right);
  insert_vec2(uv_data, body_uv(bottom_right, false));
  insert_vec3(normal_data, normalize(br_normal));
}

void cylinder::make_wedge(float currentTheta, float nextTheta, bool last) {
  // Body
  float y_step   = 1.f / m_param1;
  float curr_top = 0.5f;
  for (size_t i = 0; i != m_param1; ++i) {
    // If it's the last iter we override the U coordinate to 1.f
    float curr_bot = curr_top - y_step;
    auto top_left  = vec3(0.5 * cos(nextTheta),
      curr_top, 0.5 * sin(nextTheta));
    auto bot_left  = vec3(0.5 * cos(nextTheta),
      curr_bot, 0.5 * sin(nextTheta));
    auto top_right = vec3(0.5 * cos(currentTheta),
      curr_top, 0.5 * sin(currentTheta));
    auto bot_right = vec3(0.5 * cos(currentTheta),
      curr_bot, 0.5 * sin(currentTheta));

    make_body_tile(top_left, top_right, bot_left, bot_right, last);
    curr_top = curr_bot;
  }

  // Caps
  float curr_radius = 0.f;
  float radius_step = 0.5 / m_param1;
  for (size_t i = 0; i != m_param1; ++i) {
    float next_radius = curr_radius + radius_step;

    // Top cap
    auto top_left  = vec3(curr_radius * cos(nextTheta),
      0.5, curr_radius * sin(nextTheta));
    auto bot_left  = vec3(next_radius * cos(nextTheta),
      0.5, next_radius * sin(nextTheta));
    auto top_right = vec3(curr_radius * cos(currentTheta),
      0.5, curr_radius * sin(currentTheta));
    auto bot_right = vec3(next_radius * cos(currentTheta),
      0.5, next_radius * sin(currentTheta));
    make_cap_tile(top_left, top_right, bot_left, bot_right);

    // Bottom cap
    top_left  = vec3(curr_radius * cos(nextTheta),
      -0.5, curr_radius * sin(nextTheta));
    bot_left  = vec3(next_radius * cos(nextTheta),
      -0.5, next_radius * sin(nextTheta));
    top_right = vec3(curr_radius * cos(currentTheta),
      -0.5, curr_radius * sin(currentTheta));
    bot_right = vec3(next_radius * cos(currentTheta),
      -0.5, next_radius * sin(currentTheta));
    make_cap_tile(bot_left, bot_right, top_left, top_right);

    curr_radius = next_radius;
  }
}

void cylinder::make_cylinder() {
  float theta_step = glm::radians(360.f / m_param2);
  float curr_theta = 0 * theta_step;

  for (size_t i = 0; i != m_param2; ++i, curr_theta += theta_step)
    make_wedge(curr_theta, curr_theta + theta_step, i == m_param2 - 1);
}

void cylinder::set_vertex_data() {
  make_cylinder();
}

void cylinder::insert_vec3(vector<float> &data, vec3 v) {
  data.push_back(v.x);
  data.push_back(v.y);
  data.push_back(v.z);
}

void cylinder::insert_vec2(vector<float> &data, vec2 v) {
  data.push_back(v.x);
  data.push_back(v.y);
}
