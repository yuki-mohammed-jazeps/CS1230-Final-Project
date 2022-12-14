#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "utils/scenedata.h"

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class camera {
private:
  bool update;

  glm::vec4 position;
  glm::vec4 look;
  glm::vec4 up;
  glm::vec4 right;
  glm::mat4 view_matrix;
  glm::mat4 inv_view_matrix;
  glm::mat4 proj_matrix;
  glm::mat4 pv_matrix;

  float height_angle;
  float horizontal_angle;
  float aspect;

  float near;
  float far;

  GLuint pv_u;  // Projection * View
  GLuint pos_u; // Camera position

  // In pixels
  int width;
  int height;

  bool initialized;

public:
  camera();

  void initialize(GLuint program_id);
  void update_scene(const SceneCameraData &s, int width, int height);
  void update_position();
  void update_settings();
  void update_size(int width, int height);
  void send_uniforms();
  bool should_update() { return update; };

  // Get position in world space
  glm::vec3 get_pos() { return glm::vec3(position); }

  // Movement
  void move(bool w, bool a, bool s, bool d, bool c, bool u, float t);
  void rotate_side(float angle);
  void rotate_up(float angle);

  void send_particleUiforms(GLuint m_particle_shader, glm::mat4 part_model_matrix);

};
