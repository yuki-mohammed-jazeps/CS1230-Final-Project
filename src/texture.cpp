#include <iostream>
#include "texture.h"

using std::string;
using std::cout;    using std::endl;

texture::texture(const SceneFileMap &filemap, size_t tex_unit) : tex_unit(tex_unit) {
  string filepath = filemap.filename;
  QImage img;
  if (!img.load(QString::fromStdString(filepath)))
    cout << "Failed to load in texture: " << filepath << endl;
  else
    cout << "Done loading texture: " << filepath << endl;

  img = img.convertToFormat(QImage::Format_RGBA8888).mirrored();

  // Generate texture through OpenGL
  glGenTextures(1, &tex_id);

  // Bind texture to set data and parameters
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(),
    0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void texture::cleanup() {
  glDeleteTextures(1, &tex_id);
}

void texture::bind() const {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
}

void texture::unbind() const {
  glBindTexture(GL_TEXTURE_2D, 0);
}
