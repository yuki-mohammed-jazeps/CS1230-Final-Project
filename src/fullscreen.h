#pragma once

#include <GL/glew.h>
#include <vector>

class fullscreen {
private:
  std::vector<GLfloat> quad_data;
  bool invert_filter;
  bool blur_filter;
  bool grayscale_filter;
  bool sharpen_filter;
  bool valid; // Can be rendered

  // Uniforms for postprocessing booleans and sampler
  GLuint inv_u;
  GLuint blr_u;
  GLuint gry_u;
  GLuint shr_u;
  GLuint tex_u;

  // VAO and VBO for fullscreen quad
  GLuint vao_id;
  GLuint vbo_id;

  // Framebuffer data
  size_t fbo_width, fbo_height;
  GLuint fbo_id;
  GLuint fbo_tex_id;
  GLuint fbo_buf_id;

  // For rendering
  void bind();
  void unbind();

public:
  fullscreen();
 ~fullscreen();
  void initialize(GLuint program_id);

  // Simple setters
  void set_invert(bool new_filter, GLuint program_id);
  void set_blur(bool new_filter, GLuint program_id);
  void set_grayscale(bool new_filter, GLuint program_id);
  void set_sharpen(bool new_filter, GLuint program_id);

  // Make a new FBO with current window size
  void make_fbo();
  // Set as current framebuffer
  void set_as_canvas();
  // Render the fullscren quad with data in FBO
  void render();

  // Simple getters for filter booleans
  bool get_invert()    { return invert_filter; }
  bool get_blur()      { return blur_filter; }
  bool get_grayscale() { return grayscale_filter; }
  bool get_sharpen()   { return sharpen_filter; }
};
