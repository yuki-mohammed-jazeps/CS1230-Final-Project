#pragma once

#include <vector>
#include <glm/glm.hpp>

class triangle
{
public:
  void update_params();
  std::vector<float> vertex_data;
  std::vector<float> normal_data;
  void set_vertex_data();

private:
  void insert_vec3(std::vector<float> &data, glm::vec3 v);
};
