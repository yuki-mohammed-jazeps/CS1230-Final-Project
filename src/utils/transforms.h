#pragma once

#include "scenedata.h"
#include <glm/gtx/transform.hpp>
#include <vector>

class transforms
{
public:
  static glm::mat4 apply_transforms(const glm::mat4 &m, const std::vector<SceneTransformation*> &ts);
  static glm::mat4 manual_rotate_y(float angle);
  static glm::mat3 manual_rotate_arbitrary(float angle, glm::vec4 axis);
};
