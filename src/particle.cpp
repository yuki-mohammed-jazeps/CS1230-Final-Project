#include "particle.h"

#include "utils/shaderloader.h"
#include <math.h>
#include <random>

#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"

float randam() {
    return (float)rand() / (float)RAND_MAX;
}

void particle::particleInit(){
    particles.resize(particleSize);
    positions.resize(particleSize * particlelDataSize);
//    m_part_model = glm::translate(m_part_model, glm::vec3(5.0, 5.0, 5.0));

    QString particle_filepath = QString(":/resources/scene/Particle.png");

    m_image = QImage(particle_filepath);

    // Task 2: Format image to fit OpenGL
    m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();

    m_particle_shader = ShaderLoader::createShaderProgram(":/resources/shaders/particle.vert", ":/resources/shaders/particle.frag");
    glUseProgram(m_particle_shader);

    glGenBuffers(1, &m_partPos_vbo);
    glGenBuffers(1, &m_particle_vbo);
    glGenVertexArrays(1, &m_particle_vao);


    glGenTextures(1, &m_particle_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_particle_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLint location = glGetUniformLocation(m_particle_shader, "u_part_texture");
    glUniform1i(location, 0);

    glUseProgram(0);
}

float uniform(){
    return (float)rand() / RAND_MAX;
}

glm::vec2 randCirc(){
    float theta = 2 * M_PI * uniform();
    double len = sqrt(uniform()) * 1.0f;
    return glm::vec2(len * cos(theta), len * sin(theta));
}

float lerp(float x, float y, float t) {
  return x * (1.f - t) + y * t;
}

void particle::particleRevive(int i){
    glm::vec2 randPoint = randCirc();
    particles[i].position = glm::vec3(randPoint.x, 0,randPoint.y);
    float dist = glm::distance(center, glm::vec2(particles[i].position.x, particles[i].position.z));
    particles[i].life = uniform() * lerp(5, 0, pow(dist, 0.1)) * mag;
}

void particle::updatePositions(int i){
    positions[i*particlelDataSize+0] = particles[i].position.x;
    positions[i*particlelDataSize+1] = particles[i].position.y;
    positions[i*particlelDataSize+2] = particles[i].position.z;
    positions[i*particlelDataSize+3] = particles[i].life;
}

void particle::particleUpdate(){
        for (int i=0; i<particles.size(); i++){
            particles[i].position.x += (rand() % 3 - 1) * 0.01; // Move randomly in x direction
            particles[i].position.y += (rand() % 10) * 0.01; // Move upwards in y direction
            particles[i].position.z += (rand() % 3 - 1) * 0.01; // Move randomly in z direction
            particles[i].life -= 0.1;

            if (particles[i].life <= 0) {
//               particles[i].life = 1;
//               particles[i].position =  glm::vec3((float)rand() / RAND_MAX -0.5, (float)rand() / RAND_MAX-0.5, (float)rand() / RAND_MAX-0.5);
                particleRevive(i);
            }

            updatePositions(i);

        }

//        glm::vec3((float)rand() / RAND_MAX -0.5, (float)rand() / RAND_MAX-0.5, (float)rand() / RAND_MAX-0.5);
}


void particle::particleDraw(){
    mag = uniform() * 5;
    timer += 1;

    if (timer % 10 == 0) center = glm::vec2((uniform()-0.5) * 0.3, (uniform()-0.5) * 0.3);
    particleUpdate();

    glUseProgram(m_particle_shader);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_particle_texture);

    std::vector<float> quadVertices = {
        // positions         // uv
        -0.05f, -0.05f, 0.0,  0.0f, 0.0f,
         0.05f, -0.05f, 0.0,  1.0f, 0.0f,

        -0.05f,  0.05f, 0.0,  0.0f, 1.0f,

        -0.05f,  0.05f,  0.0, 0.0f, 1.0f,
         0.05f, -0.05f,  0.0, 1.0f, 0.0f,
         0.05f,  0.05f,  0.0, 1.0f, 1.0f,
    };

//    uniform vec3 cameraRight;
//    uniform vec3 cameraUp;
//    glm::mat4 m_view = cam.getView();
//    glm::mat4 m_proj = cam.getProjection();
//    std::cout << glm::to_string(cam.view_matrix) << std::endl;
//    glm::mat4 m_view = glm::mat4(1.0);
//    glm::mat4 m_proj = glm::mat4(1.0);
//    glm::mat4 particle_mvp = m_proj * m_view * m_part_model;
//    GLuint location = glGetUniformLocation(m_particle_shader, "mvp");
//    glUniformMatrix4fv(location, 1, false, &particle_mvp[0][0]);

//    glm::vec3 CameraRight_worldspace = {m_view[0][0], m_view[1][0], m_view[2][0]};
//    glm::vec3 CameraUp_worldspace = {m_view[0][1], m_view[1][1], m_view[2][1]};
//    location = glGetUniformLocation(m_particle_shader, "cameraRight");
//    glUniform3fv(location, 1, &CameraRight_worldspace[0]);
//    location = glGetUniformLocation(m_particle_shader, "cameraUp");
//    glUniform3fv(location, 1, &CameraUp_worldspace[0]);
    cam.send_particleUiforms(m_particle_shader, m_part_model);
    GLuint location = glGetUniformLocation(m_particle_shader, "quadSize");
    glUniform1f(location, 0.5);
    location = glGetUniformLocation(m_particle_shader, "u_part_texture");
    glUniform1i(location, 0);

    glBindVertexArray(m_particle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_vbo);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(GLfloat), quadVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5* sizeof(GLfloat), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3*sizeof(GLfloat)));
    // also set instance data
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_partPos_vbo);
    glBufferData(GL_ARRAY_BUFFER, particles.size()*sizeof(GLfloat),positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, particlelDataSize, GL_FLOAT, GL_FALSE, particlelDataSize * sizeof(GLfloat), reinterpret_cast<void*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);

//    std::cout << "hello" << std::endl;

//    glBindVertexArray(m_particle_vao);
//    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particleSize); // 100 triangles of 6 vertices each
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void particle::particleFinish(){
    glDeleteBuffers(1, &m_particle_vbo);
    glDeleteBuffers(1, &m_partPos_vbo);
    glDeleteVertexArrays(1, &m_particle_vao);
    glDeleteProgram(m_particle_shader);
    glDeleteTextures(1, &m_particle_texture);
}
