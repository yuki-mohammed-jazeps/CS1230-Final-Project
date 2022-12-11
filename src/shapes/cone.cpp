#include "shapes/cone.h"

using glm::vec3;
using glm::vec2;
using std::vector;

constexpr float pi = 3.1415926535f;

void cone::update_params(int param1, int param2) {
  vertex_data = vector<float>();
  normal_data = vector<float>();
  m_param1 = param1 < 1 ? 1 : param1;
  m_param2 = param2 < 3 ? 3 : param2;
  set_vertex_data();
}

// Auxiliary functions for cap and body UVs
static inline vec2 cap_uv(const vec3 &p) {
  return vec2(p.x + 0.5, -p.z + 0.5);
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

  return vec2(u, 0.5 - p.y);
}

// Data building funtions
void cone::make_cap_tile(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right) {
  vec3 normal = vec3(0, -1, 0);

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

void cone::make_body_tile(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right, bool last) {
  glm::vec3 tl_normal = glm::vec3(top_left.x, 0, top_left.z);
  glm::vec3 tr_normal = glm::vec3(top_right.x, 0, top_right.z);
  glm::vec3 bl_normal = glm::vec3(bottom_left.x, 0, bottom_left.z);
  glm::vec3 br_normal = glm::vec3(bottom_right.x, 0, bottom_right.z);

  bl_normal.y = glm::length(bl_normal) * 0.5;
  br_normal.y = glm::length(br_normal) * 0.5;
  // Tip special case
  if (top_left.x  == 0.f && top_left.z  == 0.f &&
      top_right.x == 0.f && top_right.z == 0.f) {
    auto midpoint = (bl_normal + br_normal) / 2.f;
    tl_normal     = midpoint;
    tr_normal     = midpoint;
    tl_normal.y   = bl_normal.y;
    tr_normal.y   = br_normal.y;
  } else {
    tl_normal.y = glm::length(tl_normal) * 0.5;
    tr_normal.y = glm::length(tr_normal) * 0.5;
  }

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

void cone::make_wedge(float currentTheta, float nextTheta, bool last) {
  // Body
  float y_step   = 1.f / m_param1;
  float curr_top = 0.5f;

  float curr_radius = 0.f;
  float radius_step = 0.5 / m_param1;

  for (size_t i = 0; i != m_param1; ++i) {
    float curr_bot = curr_top - y_step;
    float next_radius = curr_radius + radius_step;

    auto top_left  = vec3(curr_radius * cos(nextTheta),
      curr_top, curr_radius * sin(nextTheta));
    auto bot_left  = vec3(next_radius * cos(nextTheta),
      curr_bot, next_radius * sin(nextTheta));
    auto top_right = vec3(curr_radius * cos(currentTheta),
      curr_top, curr_radius * sin(currentTheta));
    auto bot_right = vec3(next_radius * cos(currentTheta),
      curr_bot, next_radius * sin(currentTheta));

    make_body_tile(top_left, top_right, bot_left, bot_right, last);
    curr_top = curr_bot;
    curr_radius = next_radius;
  }

  // Bottom cap
  curr_radius = 0.f;
  for (size_t i = 0; i != m_param1; ++i) {
    float next_radius = curr_radius + radius_step;

    auto top_left  = vec3(curr_radius * cos(nextTheta),
      -0.5, curr_radius * sin(nextTheta));
    auto bot_left  = vec3(next_radius * cos(nextTheta),
      -0.5, next_radius * sin(nextTheta));
    auto top_right = vec3(curr_radius * cos(currentTheta),
      -0.5, curr_radius * sin(currentTheta));
    auto bot_right = vec3(next_radius * cos(currentTheta),
      -0.5, next_radius * sin(currentTheta));
    make_cap_tile(bot_left, bot_right, top_left, top_right);

    curr_radius = next_radius;
  }
}

void cone::make_cone() {
  float theta_step = glm::radians(360.f / m_param2);
  float curr_theta = 0 * theta_step;

  for (size_t i = 0; i != m_param2; ++i, curr_theta += theta_step)
    make_wedge(curr_theta, curr_theta + theta_step, i == m_param2 - 1);
}

void cone::set_vertex_data() {
  make_cone();
}

void cone::insert_vec3(vector<float> &data, vec3 v) {
  data.push_back(v.x);
  data.push_back(v.y);
  data.push_back(v.z);
}

void cone::insert_vec2(vector<float> &data, vec2 v) {
  data.push_back(v.x);
  data.push_back(v.y);
}
