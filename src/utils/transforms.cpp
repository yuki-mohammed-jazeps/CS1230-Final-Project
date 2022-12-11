#include "transforms.h"

using glm::vec4;                using glm::mat3;
using glm::vec3;		using glm::mat4;
using glm::normalize;	        using glm::dot;
using glm::cross;		using glm::rotate;
using glm::translate; 	        using glm::scale;
using glm::inverse;		using std::vector;

// Returns the result of applying a sequence of transformations to a matrix
mat4 transforms::apply_transforms(const mat4 &m, const vector<SceneTransformation*> &ts) {
  auto ret = m;

  for(const auto &t : ts) {
    switch (t->type) {
      case TransformationType::TRANSFORMATION_TRANSLATE:
        ret *= translate(t->translate);
        break;
      case TransformationType::TRANSFORMATION_SCALE:
        ret *= scale(t->scale);
        break;
      case TransformationType::TRANSFORMATION_ROTATE:
        ret *= rotate(t->angle, t->rotate);
        break;
      case TransformationType::TRANSFORMATION_MATRIX:
        ret *= t->matrix;
        break;
      }
  }

  return ret;
}

mat4 transforms::manual_rotate_y(float angle) {
  auto rad = glm::radians(angle);
  return mat4(cos(rad), 0.0f, -sin(rad), 0.0f,
              0.0f,     1.0f,  0.0f, 	   0.0f,
              sin(rad), 0.0f,  cos(rad), 0.0f,
              0.0f,     0.0f,  0.0f,     1.0f);
}

mat3 transforms::manual_rotate_arbitrary(float angle, vec4 axis) {
  auto rad = glm::radians(angle);

  // Precompute some values
  float ux  = axis.x;
  float uy  = axis.y;
  float uz  = axis.z;
  float uxx = axis.x * axis.x;
  float uyy = axis.y * axis.y;
  float uzz = axis.z * axis.z;
  float uxy = axis.x * axis.y;
  float uxz = axis.x * axis.z;
  float uyz = axis.y * axis.z;

  float c   = cos(rad);
  float s   = sin(rad);
  float com = 1 - c;

  // Ugly equation but handout says to use matrices, got this from
  // https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
  return mat3(c + uxx * com, uxy * com + uz * s, uxz * com - uy * s,
              uxy * com - uz * s, c + uyy * com, uyz * com + ux * s,
              uxz * com + uy * s, uyz * com - ux * s, c + uzz * com);
}
