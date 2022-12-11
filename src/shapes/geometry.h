#pragma once

#include "texture.h"
#include "utils/sceneparser.h"
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <map>

class geometry_set
{
private:
  // Can be drawn
  bool valid;

  // OpenGL buffer ids
  GLuint vbo_id; // Vertices
  GLuint ubo_id; // UVs
  GLuint nbo_id; // Normals
  GLuint tbo_id; // Tangents
  GLuint mvo_id; // Mesh vertices
  GLuint muo_id; // Mesh UVs
  GLuint mno_id; // Mesh normals
  GLuint mto_id; // Mesh tangents

  // OpenGL VAO
  GLuint vao_id;

  // Uniform for model matrix
  GLuint model_u;
  GLuint inv_model_u;

  // Uniforms for object colors
  GLuint amb_u;
  GLuint dif_u;
  GLuint spe_u;
  GLuint shi_u;
  GLuint tex_u; // Texture mapping
  GLuint bld_u; // Texture mapping
  GLuint pav_u; // Parallax mapping
  GLuint paf_u; // Parallax mapping

  // Metadata for each shape
  struct shape_description;
  std::vector<shape_description> shape_descriptions;
  std::vector<shape_description> mesh_shape_descriptions;

  // Used for LOD extra credit
  struct shape_id;
  std::map<shape_id, std::tuple<size_t, size_t>> unique_shape_starts; // Given a shape_id, tells
                                                                      // you offset in vertex buffer
                                                                      // and number of tris
  glm::vec3 cam_pos;
  float min_dist;
  float max_dist;
  float dist_range;
  std::vector<float> distances;

  // Data buffers
  std::vector<float> vertex_buffer_data;
  std::vector<float> uv_buffer_data;
  std::vector<float> normal_buffer_data;
  std::vector<float> tangent_buffer_data;

  // Extra credit, data buffers for meshes
  std::vector<float> mesh_vertex_buffer_data;
  std::vector<float> mesh_uv_buffer_data;
  std::vector<float> mesh_normal_buffer_data;
  std::vector<float> mesh_tangent_buffer_data;

  // Count of all elements (points, lines, polys) to render
  size_t elements = 0;

  // Current tessellation parameters
  int t_0;
  int t_1;

  // Extra credits: distance LOD, meshes, and texture mapping
  bool lod;
  bool meshes;
  bool texturing;
  const std::vector<texture> *textures;

  // Points, lines, polys, etc.
  GLenum mode;

  // Shapes this geometry comprises
  const std::vector<RenderShapeData> *shapes;

  // Set VAO as current
  void set_vao_tessellated();
  void set_vao_meshes();

  // Update buffers with new data
  void update_buffers(bool update_meshes);

  // Add a specific shape's data
  void add_mesh_data(const RenderShapeData &s);
  void add_shape_data(const RenderShapeData &s, size_t idx, bool update_meshes);

  // Set vertex and normal data
  void update_data(bool update_meshes);

  // Auxiliary to render shapes in any VAO
  void draw_shapes(const std::vector<shape_description> &vec);

  // Auxiliary to make tangents from triangles
  std::vector<float> make_tangents(const std::vector<float> &vertices,
    const std::vector<float> &uvs);

public:
   geometry_set();
  ~geometry_set();

  // Initialize uniforms and pointer to scene data
  void initialize(GLuint program_id);

  // Set shape data
  void set_data(const std::vector<RenderShapeData> &master_data,
    glm::vec3 camera_pos, const std::vector<texture> &tex);

  // Update buffers with new tessellation parameters
  void update_tessellation(int tess_0, int tess_1);

  // Extra credits: distance LOD, meshes and texture mapping
  void update_lod(bool new_lod);
  void update_meshes_display(bool new_meshes);
  void update_texturing(bool new_texturing);

  // Draw everything
  void draw();

  // Bind VAO and VBOs
  void bind();

  // Unbind VAO and VBOs
  void unbind();

  // Simple getters
  int get_elements() { return this->elements; }
  GLenum get_mode() { return this->mode; }

  // Utility
  static void add_to_vec(std::vector<float> &vec, glm::vec3 p);
  static void add_to_vec(std::vector<float> &vec, glm::vec4 p);
};
