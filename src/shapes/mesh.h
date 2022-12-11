#pragma once

#include <string>
#include "utils/obj_loader.h"
#include <shapes/triangle.h>

class mesh
{
private:
  obj_loader loader;
  void set_vertex_data();

  // All good with obj laoder
  bool valid;

public:
  std::vector<float> vertex_data;
  std::vector<float> normal_data;
  std::vector<float> uv_data;

  mesh(std::string fp);
  void make_mesh();
};
