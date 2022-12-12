#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GL/glew.h"
#include "qimage.h"
#include "utils/scenedata.h"

class texture
{
private:
  size_t tex_unit;
public:
  GLuint tex_id;
  GLuint nor_id;
  GLuint dis_id;
  texture(const SceneFileMap &filemap, size_t tex_unit);

  void bind() const;
  void unbind() const;
  void cleanup();
};
