#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <map>
#include <string>
#include "settings.h"
#include "utils/shaderloader.h"

using std::map;
using std::string;

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
  : QOpenGLWidget(parent), extra_lod(false), extra_meshes(false)
{
  m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  m_keyMap[Qt::Key_W]       = false;
  m_keyMap[Qt::Key_A]       = false;
  m_keyMap[Qt::Key_S]       = false;
  m_keyMap[Qt::Key_D]       = false;
  m_keyMap[Qt::Key_Control] = false;
  m_keyMap[Qt::Key_Space]   = false;

  // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
  killTimer(m_timer);
  this->makeCurrent();

  // Students: anything requiring OpenGL calls when the program exits should be done here
  for (auto &t : textures)
    t.cleanup();

  this->doneCurrent();
}

void Realtime::initializeGL() {
  m_devicePixelRatio = this->devicePixelRatio();

  m_timer = startTimer(1000/60);
  m_elapsedTimer.start();

  // Initializing GL.
  // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) {
      std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

  // Allows OpenGL to draw objects appropriately on top of one another
  glEnable(GL_DEPTH_TEST);
  // Tells OpenGL how big the screen is
  glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

  // Students: anything requiring OpenGL calls when the program starts should be done here
  glEnable(GL_CULL_FACE);

  // Load shaders
  phong_shader_id   = ShaderLoader::createShaderProgram("resources/shaders/phong.vert",
                                                        "resources/shaders/phong.frag");
  texture_shader_id = ShaderLoader::createShaderProgram("resources/shaders/texture.vert",
                                                        "resources/shaders/texture.frag");

  glUseProgram(phong_shader_id);
  // Pass shader to camera
  cam.initialize(phong_shader_id);
  // Pass shader to scene lighting
  scene_lighting.initialize(phong_shader_id);
  // Pass shader to scene geometry
  scene_objects.initialize(phong_shader_id);

  // Set texture uniform for our phong shader
  GLint tex_u = glGetUniformLocation(phong_shader_id, "tex");
  glUniform1i(tex_u, 0);

  // Initialize uniforms for the fullscreen quad
  glUseProgram(texture_shader_id);
  full_quad.initialize(texture_shader_id);

  glUseProgram(0);
}

void Realtime::paintGL() {
  // Render to fullscreen quad's texture
  full_quad.set_as_canvas();

  ///////////////////////////
  // Render scene geometry //
  ///////////////////////////
  glUseProgram(phong_shader_id);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Send light and camera data if needed
  if (cam.should_update())
    cam.send_uniforms();
  if (scene_lighting.should_update())
    scene_lighting.send_uniforms();

  // Draw meshes in our master set
  scene_objects.draw();

  ////////////////////////////
  // Render fullscreen quad //
  ////////////////////////////
  glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
  glUseProgram(texture_shader_id);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  full_quad.render();

  glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
  // Tells OpenGL how big the screen is
  glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

  // Update camera settings
  cam.update_size(size().width(), size().height());

  full_quad.make_fbo();
}

void Realtime::sceneChanged() {
  bool success = SceneParser::parse(settings.sceneFilePath, meta_data);

  if (!success) {
    std::cerr << "Error parsing scene: \"" << settings.sceneFilePath << "\"" << std::endl;
    return;
  }

  // Fill texture vector
  for (auto &t : textures)
    t.cleanup();
  textures.clear();
  map<string, size_t> tex_map;
  std::cout << "Filling textures" << std::endl;
  size_t tex_id = 0;
  for (auto &s : meta_data.shapes) {
    auto curr_scene_file_map = &s.primitive.material.textureMap;

    // If this shape is using textures
    if (curr_scene_file_map->isUsed) {
      // If we already created a texture object for this filename
      if (tex_map.contains(curr_scene_file_map->filename)) {
        curr_scene_file_map->key = tex_map[curr_scene_file_map->filename];
      // Else, create a new texture
      } else {
        textures.push_back(texture(*curr_scene_file_map, tex_id));
        curr_scene_file_map->key = tex_id;
        tex_map[curr_scene_file_map->filename] = tex_id;
        ++tex_id;
      }
    }
  }
  std::cout << "Done filling textures" << std::endl;

  // Update camera with new settings
  cam.update_scene(meta_data.cameraData, size().width(), size().height());

  // Set lighting data
  scene_lighting.set_data(meta_data.lights,
    meta_data.globalData.ka, meta_data.globalData.kd,
    meta_data.globalData.ks);

  // Set mesh data
  scene_objects.set_data(meta_data.shapes, cam.get_pos(), textures);

  update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
  // Clipping planes
  if (settings.nearPlane != prev_near ||
      settings.farPlane != prev_far) {
    // Update camera with new settings
    cam.update_settings();
    prev_near = settings.nearPlane;
    prev_far  = settings.farPlane;
  }

  // Mesh tessellation
  if (settings.shapeParameter1 != prev_tess_1 ||
      settings.shapeParameter2 != prev_tess_2) {
    // Update camera with new settings
    scene_objects.update_tessellation(
      settings.shapeParameter1, settings.shapeParameter2);
    prev_tess_1 = settings.shapeParameter1;
    prev_tess_2 = settings.shapeParameter2;
  }

  // Extra credits: LOD
  if (settings.extraCredit1 != extra_lod) {
    scene_objects.update_lod(settings.extraCredit1);
    extra_lod = settings.extraCredit1;
  }

  // Extra credits: meshes
  if (settings.extraCredit2 != extra_meshes) {
    scene_objects.update_meshes_display(settings.extraCredit2);
    extra_meshes = settings.extraCredit2;
  }

  // Extra credits: texturing
  if (settings.extraCredit3 != extra_texturing) {
    scene_objects.update_texturing(settings.extraCredit3);
    extra_texturing = settings.extraCredit3;
  }

  // Customizable default FBO and postprocessing filters
  default_fbo = settings.defaultFBO;

  // Only change filter booleans if necessary since
  // they have to be updated through uniforms
  if (full_quad.get_invert() != settings.perPixelFilter)
    full_quad.set_invert(settings.perPixelFilter, texture_shader_id);
  if (full_quad.get_blur() != settings.kernelBasedFilter)
    full_quad.set_blur(settings.kernelBasedFilter, texture_shader_id);
  if (full_quad.get_grayscale() != settings.extraCredit4)
    full_quad.set_grayscale(settings.extraCredit4, texture_shader_id);
  if (full_quad.get_sharpen() != settings.extraCredit5)
    full_quad.set_sharpen(settings.extraCredit5, texture_shader_id);

  update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
  m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
  m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
  if (event->buttons().testFlag(Qt::LeftButton)) {
      m_mouseDown = true;
      m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
  }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
  if (!event->buttons().testFlag(Qt::LeftButton)) {
      m_mouseDown = false;
  }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
  if (m_mouseDown) {
      int posX = event->position().x();
      int posY = event->position().y();
      int deltaX = posX - m_prev_mouse_pos.x;
      int deltaY = posY - m_prev_mouse_pos.y;
      m_prev_mouse_pos = glm::vec2(posX, posY);

      // Use deltaX and deltaY here to rotate
      float x_angle = deltaX * 0.1;
      float y_angle = deltaY * 0.1;

      // Update camera
      cam.rotate_side(x_angle);
      cam.rotate_up(y_angle);

      update(); // asks for a PaintGL() call to occur
  }
}

void Realtime::timerEvent(QTimerEvent *event) {
  int elapsedms   = m_elapsedTimer.elapsed();
  float deltaTime = elapsedms * 0.001f;
  m_elapsedTimer.restart();

  // Use deltaTime and m_keyMap here to move around
  cam.move(m_keyMap[Qt::Key_W], m_keyMap[Qt::Key_A],
    m_keyMap[Qt::Key_S], m_keyMap[Qt::Key_D],
    m_keyMap[Qt::Key_Control], m_keyMap[Qt::Key_Space], deltaTime);

  update(); // asks for a PaintGL() call to occur
}
