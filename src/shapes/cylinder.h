#pragma once

#include <vector>
#include <glm/glm.hpp>

class cylinder
{
public:
  void update_params(int param1, int param2);
  std::vector<float> vertex_data;
  std::vector<float> normal_data;
  std::vector<float> uv_data;
  void set_vertex_data();

private:
  void insert_vec3(std::vector<float> &data, glm::vec3 v);
  void insert_vec2(std::vector<float> &data, glm::vec2 v);

  void make_cap_tile(glm::vec3 top_left, glm::vec3 top_right,
    glm::vec3 bottom_left, glm::vec3 bottom_right);
  void make_body_tile(glm::vec3 top_left, glm::vec3 top_right,
    glm::vec3 bottom_left, glm::vec3 bottom_right, bool last);
  void make_wedge(float curr_theta, float next_theta, bool last);
  void make_cylinder();

  int m_param1;
  int m_param2;
};
