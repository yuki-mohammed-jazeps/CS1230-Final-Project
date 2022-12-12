#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "camera.h"
#include "fullscreen.h"
#include "lighting.h"
#include "shapes/geometry.h"
#include "utils/sceneparser.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    // Scene data
    RenderData meta_data;
    geometry_set scene_objects;
    lighting scene_lighting;
    fullscreen full_quad;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    // Shaders
    GLuint phong_shader_id;
    GLuint texture_shader_id;

    // To avoid unnecessary updates
    float prev_near, prev_far;
    int prev_tess_1, prev_tess_2;

    // Extra credits: distance LOD, meshes and texture mapping
    bool extra_lod;
    bool extra_meshes;
    bool extra_texturing;

    // Used to change the default FBO since it might be machine dependant
    int default_fbo;

    // Extra credit: texture objects, one for each texture string
    std::vector<texture> textures;

    // Camera
    camera cam;

    // Shadowmapping related
    int MAX_SPOTLIGHTS = 4;  // note: to update this need to update shadowFBO, shadowMap, and uniforms in phong
    GLuint shadow_shader_id;
    std::array<GLuint, 4> shadowFBO;  // support for shadow maps for 4 spot lights
    std::array<GLuint, 4> shadowMap;
    bool spotLightsInScene = false;  // true if there are any spot lights in the scene
    std::vector<glm::mat4> spotLightSpaceMats;
    void updateSpotLightSpaceMat(); // rotates the direction vec by 10eg every call
};
