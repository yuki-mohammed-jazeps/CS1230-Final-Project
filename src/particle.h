#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "settings.h"
#include "camera.h"
#include <QOpenGLWidget>


class particle
{
public:
    struct Particle{
        glm::vec3 position;
        float life;
    };

    void particleInit();
    void particleDraw();
    void particleUpdate();
    void particleRevive(int i);
    void updatePositions(int i);
    void particleFinish();

private:
    camera cam;
    float spawnRadius = 1.0;
    int particleSize = 50000;
    int particlelDataSize = 4;
    std::vector<Particle> particles;
    std::vector<float> positions;
//    float positions[particleSize];
    GLuint m_particle_shader;
    GLuint m_particle_vbo;
    GLuint m_particle_vao;
    GLuint m_partPos_vbo;
    GLuint m_particle_texture;
    QImage m_image;

    glm::vec2 center = glm::vec2(0,0);
    glm::mat4 m_part_model = glm::mat4(1.0);

    float mag = 5.f;
    int timer = 0;
};

