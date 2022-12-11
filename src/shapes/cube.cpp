#include "shapes/cube.h"

using glm::vec2;
using glm::vec3;
using std::vector;

void cube::update_params(int param1) {
  vertex_data = vector<float>();
  normal_data = vector<float>();
  m_param1 = param1 < 1 ? 1 : param1;
  set_vertex_data();
}

static inline vec2 cube_uv(const vec3 &p, const vec3 &n) {
  // X+
  if (abs(n.x - 1.f) < 0.01)
    return vec2(-p.z + 0.5, -p.y + 0.5);
  // X-
  if (abs(n.x + 1.f) < 0.01)
    return vec2(p.z + 0.5, -p.y + 0.5);
  // Y+
  if (abs(n.y - 1.f) < 0.01)
    return vec2(p.x + 0.5, p.z + 0.5);
  // Y-
  if (abs(n.y + 1.f) < 0.01)
    return vec2(p.x + 0.5, -p.z + 0.5);
  // Z+
  if (abs(n.z - 1.f) < 0.01)
    return vec2(p.x + 0.5, -p.y + 0.5);
  // Z-
  if (abs(n.z + 1.f) < 0.01)
    return vec2(-p.x + 0.5, -p.y + 0.5);
  return vec2(0.f, 0.f);
}

void cube::make_tile(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right) {
  auto n_0 = normalize((cross(bottom_left - top_left, top_right - top_left)));
  auto n_1 = normalize((cross(bottom_left - top_right, bottom_right - top_right)));

  insert_vec3(vertex_data, top_left);
  insert_vec2(uv_data, cube_uv(top_left, n_0));
  insert_vec3(normal_data, n_0);
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, cube_uv(bottom_left, n_0));
  insert_vec3(normal_data, n_0);
  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, cube_uv(top_right, n_0));
  insert_vec3(normal_data, n_0);

  insert_vec3(vertex_data, top_right);
  insert_vec2(uv_data, cube_uv(top_right, n_1));
  insert_vec3(normal_data, n_1);
  insert_vec3(vertex_data, bottom_left);
  insert_vec2(uv_data, cube_uv(bottom_left, n_1));
  insert_vec3(normal_data, n_1);
  insert_vec3(vertex_data, bottom_right);
  insert_vec2(uv_data, cube_uv(bottom_right, n_1));
  insert_vec3(normal_data, n_1);
}

void cube::make_face(vec3 top_left, vec3 top_right,
  vec3 bottom_left, vec3 bottom_right) {
  auto x_dir  = top_right - top_left;
  auto y_dir  = bottom_left - top_left;
  auto x_step = x_dir / static_cast<float>(m_param1);
  auto y_step = y_dir / static_cast<float>(m_param1);

  auto curr_top_left = top_left;
  for (size_t i = 0; i != m_param1; ++i) {
    auto curr_bot_left = curr_top_left + y_step;

    for (size_t j = 0; j != m_param1; ++j) {
      make_tile(curr_top_left + (static_cast<float>(j)     * x_step),
                curr_top_left + (static_cast<float>(j + 1) * x_step),
                curr_bot_left + (static_cast<float>(j)     * x_step),
                curr_bot_left + (static_cast<float>(j + 1) * x_step));
    }

    curr_top_left = curr_bot_left;
  }
}

void cube::set_vertex_data() {
  make_face(vec3(-0.5f,  0.5f,  0.5f),
            vec3( 0.5f,  0.5f,  0.5f),
            vec3(-0.5f, -0.5f,  0.5f),
            vec3( 0.5f, -0.5f,  0.5f));

  make_face(vec3( 0.5f,  0.5f, -0.5f),
            vec3(-0.5f,  0.5f, -0.5f),
            vec3( 0.5f, -0.5f, -0.5f),
            vec3(-0.5f, -0.5f, -0.5f));

  make_face(vec3(-0.5f,  0.5f, -0.5f),
            vec3( 0.5f,  0.5f, -0.5f),
            vec3(-0.5f,  0.5f,  0.5f),
            vec3( 0.5f,  0.5f,  0.5f));

  make_face(vec3(-0.5f, -0.5f,  0.5f),
            vec3( 0.5f, -0.5f,  0.5f),
            vec3(-0.5f, -0.5f, -0.5f),
            vec3( 0.5f, -0.5f, -0.5f));

  make_face(vec3(-0.5f,  0.5f, -0.5f),
            vec3(-0.5f,  0.5f,  0.5f),
            vec3(-0.5f, -0.5f, -0.5f),
            vec3(-0.5f, -0.5f,  0.5f));

  make_face(vec3( 0.5f,  0.5f,  0.5f),
            vec3( 0.5f,  0.5f, -0.5f),
            vec3( 0.5f, -0.5f,  0.5f),
            vec3( 0.5f, -0.5f, -0.5f));
}

void cube::insert_vec3(vector<float> &data, vec3 v) {
  data.push_back(v.x);
  data.push_back(v.y);
  data.push_back(v.z);
}

void cube::insert_vec2(vector<float> &data, vec2 v) {
  data.push_back(v.x);
  data.push_back(v.y);
}
