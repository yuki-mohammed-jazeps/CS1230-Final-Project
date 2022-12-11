#include "shapes/triangle.h"

using glm::vec3;
using std::vector;

void triangle::update_params() {
  vertex_data = vector<float>();
  normal_data = vector<float>();
  set_vertex_data();
}

void triangle::set_vertex_data() {
  insert_vec3(vertex_data, vec3(-0.5,  -0.5, 0.0));
  insert_vec3(normal_data, vec3( 0.0,  0.0, 1.0));
  insert_vec3(vertex_data, vec3(0.5, -0.5, 0.0));
  insert_vec3(normal_data, vec3( 0.0,  0.0, 1.0));
  insert_vec3(vertex_data, vec3( 0.0, 0.5, 0.0));
  insert_vec3(normal_data, vec3( 0.0,  0.0, 1.0));
}

void triangle::insert_vec3(vector<float> &data, vec3 v) {
  data.push_back(v.x);
  data.push_back(v.y);
  data.push_back(v.z);
}
