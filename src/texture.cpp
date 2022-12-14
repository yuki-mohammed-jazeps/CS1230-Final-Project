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
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(),
    0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

  // Bind parallax mapping files if enabled
  if (filemap.parallax) {
    string norm_fp = filemap.normal_fn;
    string disp_fp = filemap.disp_fn;
    QImage nor_img, dis_img;

    if (!nor_img.load(QString::fromStdString(norm_fp)))
      cout << "Failed to load in texture: " << norm_fp << endl;
    else
      cout << "Done loading texture: " << norm_fp << endl;
    if (!dis_img.load(QString::fromStdString(disp_fp)))
      cout << "Failed to load in texture: " << disp_fp << endl;
    else
      cout << "Done loading texture: " << disp_fp << endl;

    nor_img = nor_img.convertToFormat(QImage::Format_RGBA8888).mirrored();
    dis_img = dis_img.convertToFormat(QImage::Format_RGBA8888).mirrored();

    // Generate textures through OpenGL
    glGenTextures(1, &nor_id);
    glGenTextures(1, &dis_id);

    // Bind textures to set data and parameters
    glBindTexture(GL_TEXTURE_2D, nor_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nor_img.width(), nor_img.height(),
      0, GL_RGBA, GL_UNSIGNED_BYTE, nor_img.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, dis_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dis_img.width(), dis_img.height(),
      0, GL_RGBA, GL_UNSIGNED_BYTE, dis_img.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void texture::cleanup() {
  glDeleteTextures(1, &tex_id);
  glDeleteTextures(1, &nor_id);
  glDeleteTextures(1, &dis_id);
}

void texture::bind() const {
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, nor_id);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, dis_id);
}

void texture::unbind() const {
  glBindTexture(GL_TEXTURE_2D, 0);
}
