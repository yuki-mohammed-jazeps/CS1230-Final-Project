#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
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
    void particleDraw(camera &cam);
    void particleUpdate();
    void particleRevive(int i);
    void updatePositions(int i);
    void particleFinish();

private:
    glm::vec3 firePos = glm::vec3(-5, -0.4, -3);

    float spawnRadius = 0.7;
    int particleSize = 50000;
    int particlelDataSize = 4;
    std::vector<Particle> particles;
    std::vector<float> positions;
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

    // Spawn randomly in circle
    glm::vec2 randCirc();
};

