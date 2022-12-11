#include "fullscreen.h"
#include <math.h>

fullscreen::fullscreen() : valid(false),
  invert_filter(false), blur_filter(false),
  grayscale_filter(false), sharpen_filter(false)
{
  quad_data = {
  //     POSITIONS    //
  -1.f,  1.f, 0.0f,
  -1.f, -1.f, 0.0f,
   1.f, -1.f, 0.0f,
   1.f,  1.f, 0.0f,
  -1.f,  1.f, 0.0f,
   1.f, -1.f, 0.0f,
  //        UVS       //
   0.f, 1.f,
   0.f, 0.f,
   1.f, 0.f,
   1.f, 1.f,
   0.f, 1.f,
   1.f, 0.f
  };
}

void fullscreen::make_fbo(){
  if (!valid)
    return;

  // Clean slate
  glDeleteTextures(1, &fbo_tex_id);
  glDeleteRenderbuffers(1, &fbo_buf_id);
  glDeleteFramebuffers(1, &fbo_id);

  GLint borders[4];
  glGetIntegerv(GL_VIEWPORT, borders);
  fbo_width  = borders[2];
  fbo_height = borders[3];

  // Generate and bind an empty texture, set its min/mag filter
  // interpolation, then unbind
  glGenTextures(1, &fbo_tex_id);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fbo_tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbo_width, fbo_height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Generate and bind a renderbuffer of the right size,
  // set its format, then unbind
  glGenRenderbuffers(1, &fbo_buf_id);
  glBindRenderbuffer(GL_RENDERBUFFER, fbo_buf_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                        fbo_width, fbo_height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Generate and bind an FBO
  glGenFramebuffers(1, &fbo_id);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

  // Add our texture as a color attachment,
  // and our renderbuffer as a depth+stencil attachment, to our FBO
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, fbo_tex_id, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, fbo_buf_id);

  // Unbind the FBO
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fullscreen::set_as_canvas() {
  if (!valid)
    return;

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

void fullscreen::render() {
  if (!valid)
    return;

  glBindVertexArray(vao_id);
  // Bind "texture" to slot 0
  glUniform1i(tex_u, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fbo_tex_id);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

void fullscreen::bind() {
  if (!valid)
    return;

  glBindVertexArray(vao_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
}

void fullscreen::unbind() {
  if (!valid)
    return;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

fullscreen::~fullscreen() {
  if (!valid)
    return;

  glDeleteVertexArrays(1, &vao_id);
  glDeleteBuffers(1, &vbo_id);
  glDeleteTextures(1, &fbo_tex_id);
  glDeleteRenderbuffers(1, &fbo_buf_id);
  glDeleteFramebuffers(1, &fbo_id);
}

void fullscreen::initialize(GLuint program_id) {
  valid = true;

  glUseProgram(program_id);
  // Set uniforms to 0 by default
  inv_u = glGetUniformLocation(program_id, "should_invert");
  blr_u = glGetUniformLocation(program_id, "should_blur");
  gry_u = glGetUniformLocation(program_id, "should_grayscale");
  shr_u = glGetUniformLocation(program_id, "should_sharpen");
  tex_u = glGetUniformLocation(program_id, "tex");
  glUniform1i(inv_u, false);
  glUniform1i(blr_u, false);
  glUniform1i(gry_u, false);
  glUniform1i(shr_u, false);

  glGenVertexArrays(1, &vao_id);
  glGenBuffers(1, &vbo_id);

  // Pass data to GPU
  bind();
  glBufferData(GL_ARRAY_BUFFER, quad_data.size() * sizeof(GLfloat),
               quad_data.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0,
                        reinterpret_cast<void*>(sizeof(float) * 18));
  unbind();

  // Set FBO data, its own function since it might get called
  // from resizeGL()
  make_fbo();

  glUseProgram(0);
}

// Update uniforms when booleans change
void fullscreen::set_invert(bool new_filter, GLuint program_id)  {
  invert_filter = new_filter;
  glUseProgram(program_id);
  glUniform1i(inv_u, invert_filter);
  glUseProgram(0);
}

void fullscreen::set_blur(bool new_filter, GLuint program_id) {
  blur_filter = new_filter;
  glUseProgram(program_id);
  glUniform1i(blr_u, blur_filter);
  glUseProgram(0);
}

void fullscreen::set_grayscale(bool new_filter, GLuint program_id) {
  grayscale_filter = new_filter;
  glUseProgram(program_id);
  glUniform1i(gry_u, grayscale_filter);
  glUseProgram(0);
}

void fullscreen::set_sharpen(bool new_filter, GLuint program_id) {
  sharpen_filter = new_filter;
  glUseProgram(program_id);
  glUniform1i(shr_u, sharpen_filter);
  glUseProgram(0);
}
