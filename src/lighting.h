#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "utils/scenedata.h"

// A class representative of the lighting setup in a scene

class lighting {
private:
  bool valid;
  bool update;

  // Array of lights in scene
  size_t num_lights;
  std::vector<SceneLightData> const *lights;

  // Global lighting values
  float ka;
  float kd;
  float ks;

  // Uniforms for lighting array
  std::vector<GLuint> light_dir_uniforms;
  std::vector<GLuint> light_pos_uniforms;
  std::vector<GLuint> light_col_uniforms;
  std::vector<GLuint> light_typ_uniforms;
  std::vector<GLuint> light_ang_uniforms;
  std::vector<GLuint> light_pen_uniforms;
  std::vector<GLuint> light_fun_uniforms;

  // Uniforms for global lighting variables
  GLuint ka_u;
  GLuint kd_u;
  GLuint ks_u;

public:
  lighting();

  void initialize(GLuint program_id);
  void set_data(const std::vector<SceneLightData> &master_data,
    float ka, float kd, float ks);
  void send_uniforms();

  bool should_update() { return update; };
};
