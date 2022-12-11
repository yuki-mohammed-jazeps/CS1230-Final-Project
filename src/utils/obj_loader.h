#pragma once

#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class obj_loader
{
private:
  bool success;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> faces;

  friend class mesh;

public:
  obj_loader(std::string fp);
};
