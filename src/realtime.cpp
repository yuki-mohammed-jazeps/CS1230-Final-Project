#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <map>
#include <string>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "settings.h"
#include "utils/shaderloader.h"

using std::map;
using std::string;

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
  : QOpenGLWidget(parent), extra_lod(false), extra_meshes(false),
    initialized(false)
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

  glDeleteProgram(phong_shader_id);
  glDeleteProgram(texture_shader_id);

  // SHADOW MAPPING RELATED
  glDeleteFramebuffers(MAX_SPOTLIGHTS, &shadowFBO[0]);
  glDeleteTextures(MAX_SPOTLIGHTS, &shadowMap[0]);
  glDeleteProgram(shadow_shader_id);

  part.particleFinish();

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

  // Tells OpenGL how big the screen is
  glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
  // Allows OpenGL to draw objects appropriately on top of one another
  glEnable(GL_DEPTH_TEST);
  // Students: anything requiring OpenGL calls when the program starts should be done here
  glEnable(GL_CULL_FACE);
  // Gamma correction
  glEnable(GL_FRAMEBUFFER_SRGB);

  // Load shaders
  phong_shader_id   = ShaderLoader::createShaderProgram(":resources/shaders/parallax.vert",
                                                        ":resources/shaders/parallax.frag");
  texture_shader_id = ShaderLoader::createShaderProgram(":resources/shaders/texture.vert",
                                                        ":resources/shaders/texture.frag");
  shadow_shader_id = ShaderLoader::createShaderProgram(":resources/shaders/shadow.vert",
                                                       ":resources/shaders/shadow.frag");

  glUseProgram(phong_shader_id);
  // Pass shader to camera
  cam.initialize(phong_shader_id);
  // Pass shader to scene lighting
  scene_lighting.initialize(phong_shader_id);
  // Pass shader to scene geometry
  scene_objects.initialize(phong_shader_id);

  // Set texture uniform for our phong shader
  GLint p_tex_u = glGetUniformLocation(phong_shader_id, "tex");
  glUniform1i(p_tex_u, 0);
  GLint p_nor_u = glGetUniformLocation(phong_shader_id, "normal_map");
  glUniform1i(p_nor_u, 1);
  GLint p_dis_u = glGetUniformLocation(phong_shader_id, "disp_map");
  glUniform1i(p_dis_u, 2);

  // Store uniform for shadow enabling / disabling
  shadow_bool_u = glGetUniformLocation(phong_shader_id, "do_shadows");

  // Initialize uniforms for the fullscreen quad
  glUseProgram(texture_shader_id);
  full_quad.initialize(texture_shader_id);

  // --------  SHADOW MAPPING RELATED (set up FBO and depth texture for shadowmap) -------- //
  for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
      // The framebuffer
      glGenFramebuffers(1, &shadowFBO[i]);
      glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO[i]);

      // Depth texture
      glGenTextures(1, &shadowMap[i]);
      glBindTexture(GL_TEXTURE_2D, shadowMap[i]);
      glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT,  size().width()*m_devicePixelRatio, size().height()*m_devicePixelRatio, 0,GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap[i], 0);
      glDrawBuffer(GL_NONE); // No color buffer is drawn to
      glReadBuffer(GL_NONE);
      glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
      glBindTexture(GL_TEXTURE_2D, 0);
  }
  // ------------------------------------------------------------------------- //
  part.particleInit();

  glUseProgram(0);

  initialized = true;
}

// input is degrees and axis of rotation
glm::mat4 rotationMat(float theta, glm::vec3 axis) {
    axis = glm::normalize(axis);
    auto ux = axis[0], uy = axis[1], uz = axis[2];
    theta = glm::radians(theta);
    float _cos = cos(theta);
    float _1mcos = 1-_cos;
    float _sin = sin(theta);
    return glm::mat4(_cos + ux*ux*_1mcos, ux*uy*_1mcos + uz*_sin, ux*uz*_1mcos - uy*_sin, 0,
                     ux*uy*_1mcos - uz*_sin, _cos + uy*uy*_1mcos, uy*uz*_1mcos + ux*_sin, 0,
                     ux*uz*_1mcos + uy*_sin, uy*uz*_1mcos - ux*_sin, _cos + uz*uz*_1mcos, 0,
                     0, 0, 0, 1);
}

// SHADOW MAPPING RELATED - rotates the direction vec for all spot lights by 1deg
void Realtime::updateSpotLightSpaceMat(float deltaTime) {
    // update the directions for all spot lights in the scene & calculate new light space mats
    spotLightSpaceMats.clear();  // clear old spotlight space mats
    for (SceneLightData &light : meta_data.lights) {
        if (light.type == LightType::LIGHT_SPOT) {
            float FoV = light.angle + light.penumbra; // glm::radians(90.0f);
            glm::mat4 lightProjectionMatrix = glm::perspective(FoV, 1.0f, 0.1f, 100.f);
            glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(light.pos), glm::vec3(light.pos) + glm::vec3(light.dir), glm::vec3(0.0f,0.0f,1.0f));

            spotLightSpaceMats.push_back(lightProjectionMatrix * lightViewMatrix);
        }
    }

    scene_lighting.set_data(meta_data.lights,
      meta_data.globalData.ka, meta_data.globalData.kd,
      meta_data.globalData.ks);
}

void Realtime::paintGL() {
    // --------  SHADOW MAPPING RELATED (render the shadow map into shadowMap texture) -------- //
    if (spotLightsInScene) {  // render shadow map only if there is at least one spot light in the scene
        glUseProgram(shadow_shader_id);

        // For each spotlight, bind shadow buffer, paint shadow map into FBO buffer texture shadowMap
        for (int i = 0; i < spotLightSpaceMats.size(); ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            glUniformMatrix4fv(glGetUniformLocation(shadow_shader_id, "spotLightSpaceMat"), 1, GL_FALSE, &spotLightSpaceMats[i][0][0]);
            scene_objects.draw_shapes_shadows(shadow_shader_id);
        }

        glUseProgram(0);

    }
    // ------------------------------------------------------------------------- //


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

  // --------  SHADOW MAPPING RELATED ------------- //
  glUniform1i(shadow_bool_u, settings.shadows);
  if (spotLightsInScene) {  // send uniforms required to render shadows for spot lights in the scene
      for (int i = 0; i < spotLightSpaceMats.size(); ++i) {
          glActiveTexture(GL_TEXTURE7+i);
          glBindTexture(GL_TEXTURE_2D, shadowMap[i]);  // bind shadowmap depth texture

          glUniform1i(glGetUniformLocation(phong_shader_id, ("shadowMap[" + std::to_string(i) + "]").c_str()), 7+i);
          glUniformMatrix4fv(glGetUniformLocation(phong_shader_id, ("spotLightSpaceMat[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, &spotLightSpaceMats[i][0][0]);
      }
  }
  // -------------------------------------------- //

  // Draw meshes in our master set
  scene_objects.draw();

  if (settings.fire)
    part.particleDraw(cam);

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

  // update the shadow map textures to match the view port
  for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
      glBindTexture(GL_TEXTURE_2D, shadowMap[i]);
      glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT,  size().width()*m_devicePixelRatio, size().height()*m_devicePixelRatio, 0,GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);
  }
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

  // --------  SHADOW MAPPING RELATED ------------- //
  spotLightsInScene = false;
  spotLightSpaceMats.clear();
  // loops over lights and saves the lightSpaceMats for all spot lights
  for (SceneLightData &light : meta_data.lights) {
      if (light.type == LightType::LIGHT_SPOT) {
          spotLightsInScene = true;

          float FoV = light.angle; // glm::radians(90.0f);
          glm::mat4 lightProjectionMatrix = glm::perspective(FoV, 1.0f, 0.1f, 100.f);
          glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(light.pos), glm::vec3(light.dir), glm::vec3(0.0f,0.0f,1.0f));

          spotLightSpaceMats.push_back(lightProjectionMatrix * lightViewMatrix);
      }
  }
  // --------------------------------------------- //


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

  // Extra credits: parallax
  if (settings.extra_parallax != extra_parallax) {
    scene_objects.update_parallax(settings.extra_parallax);
    extra_parallax = settings.extra_parallax;
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
      float x_angle = deltaX * 0.06;
      float y_angle = deltaY * 0.06;

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


  // updates spotLightSpaceMats - the light space mats after rotating all spot lights by 1deg
  updateSpotLightSpaceMat(deltaTime);  // NOTE: comment this line out to stop rotating the spotlights

  update(); // asks for a PaintGL() call to occur
}
