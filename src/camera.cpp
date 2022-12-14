#include <stdexcept>
#include <cmath>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "camera.h"
#include "settings.h"
#include "utils/transforms.h"
#include "glm/gtx/string_cast.hpp"

using glm::mat4;    	using glm::inverse;
using glm::vec4;        using glm::normalize;
using glm::vec3;        using glm::cross;
using glm::translate;   using glm::dot;
using glm::radians;     using glm::lookAt;

// Auxiliary for projection matrix
mat4 get_projection(float theta_w, float theta_h, float near, float far) {
  float c = (-near) / far;

  auto scaling  = mat4(1.f / (far * tan(theta_w / 2.f)), 0.f, 0.f, 0.f,
                       0.f, 1.f / (far * tan(theta_h / 2.f)), 0.f, 0.f,
                       0.f, 0.f, 1.f / far, 0.f,
                       0.f, 0.f, 0.f,       1.f);
  auto parallel = mat4(1.f, 0.f, 0.f, 0.f,
                       0.f, 1.f, 0.f, 0.f,
                       0.f, 0.f, 1.f   / (1.f + c), -1.f,
                       0.f, 0.f, ((-c) / (1.f + c)), 0.f);
  auto mapping  = mat4(1.f, 0.f,  0.f, 0.f,
                       0.f, 1.f,  0.f, 0.f,
                       0.f, 0.f, -2.f, 0.f,
                       0.f, 0.f, -1.f, 1.f);

  return mapping * parallel * scaling;
}

camera::camera() : initialized(false), width(800), height(800),
  position(vec4(5, 0, 0, 1)), look(vec4(-1, 0, 0, 0)),
  up(vec4(0, 1, 0, 0)), height_angle(0), aspect(1.f),
  horizontal_angle(radians(45.f)), near(.1f), far(40.f),
  update(true) {};

void camera::initialize(GLuint program_id) {
  initialized     = true;
  update          = true;
  view_matrix     = lookAt(vec3(position), vec3(0, 0, 0), vec3(up));
  inv_view_matrix = inverse(view_matrix);
  proj_matrix     = get_projection(horizontal_angle, height_angle, near, far);

  pv_u  = glGetUniformLocation(program_id, "pv_matrix");
  pos_u = glGetUniformLocation(program_id, "camera_pos");
}

void camera::update_scene(const SceneCameraData &s, int wi, int he) {
  if (!initialized)
    return;

  width  = wi;
  height = he;
  position = s.pos;
  look = normalize(s.look);
  up = normalize(s.up);
  right = vec4(normalize(glm::cross(vec3(look), vec3(up))), 0.f);
  height_angle = s.heightAngle;
  aspect = width / static_cast<float>(height);
  horizontal_angle = 2.f * atan(tan(height_angle * .5f) * aspect);

  // View matrix
  view_matrix     = lookAt(vec3(position),
    vec3(position) + vec3(look), vec3(up));
  inv_view_matrix = inverse(view_matrix);
  // Projection matrix
  proj_matrix     = get_projection(horizontal_angle, height_angle, near, far);

  pv_matrix = proj_matrix * view_matrix;

  update = true;
}

// Called when camera moves
void camera::update_position() {
  if (!initialized)
    return;

  right = vec4(normalize(glm::cross(vec3(look), vec3(up))), 0.f);
  view_matrix     = lookAt(vec3(position),
    vec3(position) + vec3(look), vec3(up));
  inv_view_matrix = inverse(view_matrix);

  pv_matrix = proj_matrix * view_matrix;

  update = true;
}

void camera::update_settings() {
  if (!initialized)
    return;

  near = settings.nearPlane;
  far  = settings.farPlane;

  // Projection matrix
  proj_matrix = get_projection(horizontal_angle, height_angle, near, far);

  pv_matrix = proj_matrix * view_matrix;

  update = true;
}

void camera::update_size(int wi, int he) {
  if (!initialized)
    return;

  width  = wi;
  height = he;
  aspect = width / static_cast<float>(height);
  horizontal_angle = 2.f * atan(tan(height_angle * .5f) * aspect);

  // Projection matrix
  proj_matrix = get_projection(horizontal_angle, height_angle, near, far);

  pv_matrix = proj_matrix * view_matrix;

  update = true;
}

void camera::send_uniforms() {
  if (!initialized)
    return;

  glUniform3fv(pos_u, 1, &position[0]);
  glUniformMatrix4fv(pv_u, 1, 0, &pv_matrix[0][0]);

  update = false;
}

// Movement

// Key movement, WASD self explanatory, c = control, u = space, t = delta time
void camera::move(bool w, bool a, bool s, bool d,
  bool c, bool u, float t) {
  float distance = 5.f * t; // Scale movement speed by delta time
  vec4  direction = vec4(0.f);

  // Translate each key independently
  if (w)
    direction += look;
  if (a)
    direction -= right;
  if (s)
    direction -= look;
  if (d)
    direction += right;
  if (c)
    direction -= vec4(0.f, 1.f, 0.f, 0.f);
  if (u)
    direction += vec4(0.f, 1.f, 0.f, 0.f);

  // If we actually moved, normalize direction and scale it
  // so that movement speed is accurate, and we don't move
  // faster diagonally
  if (direction != vec4(0.f))
  {
    direction = normalize(direction) * distance;
    position += direction;
    update_position();
  }
}

// Rotate horizontally
void camera::rotate_side(float angle) {
  look = normalize(look * transforms::manual_rotate_y(angle));
  update_position();
}

void camera::rotate_up(float angle) {
  look = vec4(normalize(vec4(look) *
                        transforms::manual_rotate_arbitrary(angle, right)),
              0.f);
  update_position();
}

void camera::send_particleUiforms(GLuint m_particle_shader, glm::mat4 part_model_matrix){
    glm::mat4 particle_mvp = proj_matrix * view_matrix * part_model_matrix;
//    std::cout << glm::to_string(particle_mvp) << std::endl;
    GLuint location = glGetUniformLocation(m_particle_shader, "mvp");
    glUniformMatrix4fv(location, 1, false, &particle_mvp[0][0]);

    glm::vec3 CameraRight_worldspace = {view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]};
    glm::vec3 CameraUp_worldspace = {view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]};
    location = glGetUniformLocation(m_particle_shader, "cameraRight");
    glUniform3fv(location, 1, &CameraRight_worldspace[0]);
    location = glGetUniformLocation(m_particle_shader, "cameraUp");
    glUniform3fv(location, 1, &CameraUp_worldspace[0]);
}
