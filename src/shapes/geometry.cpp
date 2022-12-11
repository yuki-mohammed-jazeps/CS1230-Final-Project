#include "geometry.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/cone.h"
#include "shapes/mesh.h"
#include "shapes/sphere.h"
#include <iostream>
#include <tuple>

using std::vector;    using glm::vec3;
using glm::vec4;      using glm::mat4;
using std::max;       using glm::vec2;

// Parallax mapping, given a bunch of verts / uvs,
// calculate tangents for them
vector<float> geometry_set::make_tangents(const vector<float> &vertices,
  const vector<float> &uvs) {
  vector<float> ret;
  ret.reserve(vertices.size());

  for (size_t i = 0; i < vertices.size() / 9; ++i) {
    size_t p_idx = i * 9;
    size_t t_idx = i * 6;

    auto p0 = vec3(vertices[p_idx],     vertices[p_idx + 1], vertices[p_idx + 2]);
    auto p1 = vec3(vertices[p_idx + 3], vertices[p_idx + 4], vertices[p_idx + 5]);
    auto p2 = vec3(vertices[p_idx + 6], vertices[p_idx + 7], vertices[p_idx + 8]);
    auto t0 = vec2(uvs[t_idx],     uvs[t_idx + 1]);
    auto t1 = vec2(uvs[t_idx + 2], uvs[t_idx + 3]);
    auto t2 = vec2(uvs[t_idx + 4], uvs[t_idx + 5]);

    // Get edges and uv diffs
    auto e0 = p1 - p0;
    auto e1 = p2 - p0;
    auto d0 = t1 - t0;
    auto d1 = t2 - t0;

    // Calculate tangent
    float inv_factor = 1.f / (d0.x * d1.y - d0.y * d1.x);
    auto  tan = vec3(inv_factor * (d1.y * e0.x - d0.y * e1.x),
                     inv_factor * (d1.y * e0.y - d0.y * e1.y),
                     inv_factor * (d1.y * e0.z - d0.y * e1.z));

    add_to_vec(ret, tan);
    add_to_vec(ret, tan);
    add_to_vec(ret, tan);
  }

  return ret;
}

// Uniquely identifies shape data in a scene by combining primitive
// type, tessellation parameter 1, and tessellation parameter 2
struct geometry_set::shape_id {
protected:
  PrimitiveType type;
  int t_0;
  int t_1;

  friend class geometry_set;

public:
  // Since we'll be using these as keys into a map
  bool operator< (const shape_id& rhs) const {
    return std::tie(this->type, this->t_0, this->t_1) < std::tie(rhs.type, rhs.t_0, rhs.t_1);
  }

  shape_id(PrimitiveType t, int tess_0, int tess_1) :
    type(t), t_0(tess_0), t_1(tess_1) {};
};

// Description of how a shape is laid out in master data buffers
struct geometry_set::shape_description {
protected:
  mat4 const *model_matrix; // Model matrix for this shape
  mat4 const *inv_model_matrix;
  size_t      offset;       // Where to start rendering on master buffer
  size_t      points;       // How many points to render
  vec3        ambient;      // Object colors
  vec3        diffuse;
  vec3        specular;
  float       shininess;
  bool        has_tex;      // Has an associated texture
  size_t      tex_id;       // Position of texture in textures array
  float       tex_blend;    // How much texture to blend into diffuse
  bool        has_par;      // Has parallax mapping

  friend class geometry_set;

public:
  shape_description(const mat4& mat, const mat4 &inv_mat,
    size_t offset, size_t points, vec3 amb, vec3 dif,
    vec3 spe, float shi, bool has_tex, size_t tex,
    float blend, bool has_par) :
    model_matrix(&mat), inv_model_matrix(&inv_mat),
    offset(offset), points(points), ambient(amb), diffuse(dif),
    specular(spe), shininess(shi), has_tex(has_tex),
    tex_id(tex), tex_blend(blend), has_par(has_par) {};
};

geometry_set::geometry_set() : valid(false), lod(false), meshes(false), texturing(false), mode(GL_TRIANGLES) {};

void geometry_set::initialize(GLuint program_id) {
  // Location of model matrix
  model_u     = glGetUniformLocation(program_id, "model_matrix");
  inv_model_u = glGetUniformLocation(program_id, "inv_model_matrix");

  // Locations of color uniforms
  amb_u = glGetUniformLocation(program_id, "ambient");
  dif_u = glGetUniformLocation(program_id, "diffuse");
  spe_u = glGetUniformLocation(program_id, "specular");
  shi_u = glGetUniformLocation(program_id, "shine");
  tex_u = glGetUniformLocation(program_id, "texturing");
  bld_u = glGetUniformLocation(program_id, "tex_blend");
  pav_u = glGetUniformLocation(program_id, "parallax_vert");
  paf_u = glGetUniformLocation(program_id, "parallax_frag");

  // VAO for this set of meshes
  glGenVertexArrays(1, &vao_id);

  // VBOs
  glGenBuffers(1, &vbo_id);
  glGenBuffers(1, &ubo_id);
  glGenBuffers(1, &nbo_id);
  glGenBuffers(1, &tbo_id);
  glGenBuffers(1, &mvo_id);
  glGenBuffers(1, &muo_id);
  glGenBuffers(1, &mno_id);
  glGenBuffers(1, &mto_id);
}

// Auxiliary function to handle adding mesh data as its own case (doesn't use
// unique shape map)
void geometry_set::add_mesh_data(const RenderShapeData &s) {
  auto shape = mesh(s.primitive.meshfile);
  shape.make_mesh();
  auto vertices = std::move(shape.vertex_data);
  auto normals  = std::move(shape.normal_data);
  auto uvs      = std::move(shape.uv_data);

  // Metadata for this shape, i.e. how many tris to render,
  // the shape's material, shape's model matrices, offset in data buffer
  auto metadata = shape_description(s.ctm, s.inv_ctm,
    mesh_vertex_buffer_data.size() / 3, vertices.size() / 3,
    vec3(s.primitive.material.cAmbient),
    vec3(s.primitive.material.cDiffuse),
    vec3(s.primitive.material.cSpecular),
    s.primitive.material.shininess,
    s.primitive.material.textureMap.isUsed,
    s.primitive.material.textureMap.key,
    s.primitive.material.blend,
    s.primitive.material.textureMap.parallax);

  // Add data to buffers
  mesh_vertex_buffer_data.insert(mesh_vertex_buffer_data.end(),
    vertices.begin(), vertices.end());
  mesh_uv_buffer_data.insert(mesh_uv_buffer_data.end(),
    uvs.begin(), uvs.end());
  mesh_normal_buffer_data.insert(mesh_normal_buffer_data.end(),
    normals.begin(), normals.end());
  mesh_shape_descriptions.push_back(metadata);
}

// Auxiliary function to get data for any type of primitive
void geometry_set::add_shape_data(const RenderShapeData &s, size_t idx,
  bool update_meshes) {
  // Special case for meshes
  if (s.primitive.type == PrimitiveType::PRIMITIVE_MESH && update_meshes) {
    add_mesh_data(s);
    return;
  }

  int curr_t_0 = t_0;
  int curr_t_1 = t_1;

  // Extra credit: distance LOD:
  if (lod) {
    curr_t_0 = max(static_cast<int>(round(curr_t_0 * distances[idx])), t_0 / 3);
    curr_t_1 = max(static_cast<int>(round(curr_t_1 * distances[idx])), t_1 / 3);
  }

  // If this is a cube, second param is always 0
  if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)
    curr_t_1 = 0;

  // Metadata for this shape, i.e. how many tris to render,
  // the shape's material, shape's model matrices, offset and
  // number of tris are zero atm, since we might have them in
  // the map or we might have to recalculate them
  auto metadata = shape_description(s.ctm, s.inv_ctm, 0, 0,
    vec3(s.primitive.material.cAmbient),
    vec3(s.primitive.material.cDiffuse),
    vec3(s.primitive.material.cSpecular),
    s.primitive.material.shininess,
    s.primitive.material.textureMap.isUsed,
    s.primitive.material.textureMap.key,
    s.primitive.material.blend,
    s.primitive.material.textureMap.parallax);

  // Check if this shape's data already exists in vertex buffer
  auto curr_id = shape_id(s.primitive.type, curr_t_0, curr_t_1);
  bool exists  = unique_shape_starts.count(curr_id);

  // If it already exists, add metadata with existing offset and size into
  // metadata container
  if (exists) {
    auto tri_data = unique_shape_starts[curr_id];
    metadata.offset = std::get<0>(tri_data);
    metadata.points = std::get<1>(tri_data);
  // If it doesn't exist, we have to create it
  } else {
    // Vertex and normal data
    vector<float> vertices;
    vector<float> normals;
    vector<float> uvs;

    // Get vertex data at origin, yes, shape should just be a virtual class
    // to make this cleaner
    if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE) {
      auto shape = cube();
      shape.update_params(curr_t_0);
      vertices = std::move(shape.vertex_data);
      uvs      = std::move(shape.uv_data);
      normals  = std::move(shape.normal_data);
    } else if (s.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
      auto shape = cone();
      shape.update_params(curr_t_0, curr_t_1);
      vertices = std::move(shape.vertex_data);
      uvs      = std::move(shape.uv_data);
      normals  = std::move(shape.normal_data);
    } else if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
      auto shape = cylinder();
      shape.update_params(curr_t_0, curr_t_1);
      vertices = std::move(shape.vertex_data);
      uvs      = std::move(shape.uv_data);
      normals  = std::move(shape.normal_data);
    } else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
      auto shape = sphere();
      shape.update_params(curr_t_0, curr_t_1);
      vertices = std::move(shape.vertex_data);
      uvs      = std::move(shape.uv_data);
      normals  = std::move(shape.normal_data);
    }

    metadata.offset = vertex_buffer_data.size() / 3;
    metadata.points = vertices.size() / 3;
    unique_shape_starts[curr_id] = std::make_tuple(metadata.offset, metadata.points);

    // Add data to buffers
    vertex_buffer_data.insert(vertex_buffer_data.end(),
      vertices.begin(), vertices.end());
    uv_buffer_data.insert(uv_buffer_data.end(),
      uvs.begin(), uvs.end());
    normal_buffer_data.insert(normal_buffer_data.end(),
      normals.begin(), normals.end());
  }

  shape_descriptions.push_back(metadata);
}

// Takes scene data and updates relevant meta data like camera position,
// number of elements in scene, pointer to objects in scene, then
// uses update() to set the vertex and normal data
void geometry_set::set_data(const vector<RenderShapeData> &master_data,
  vec3 camera_pos, const vector<texture> &tex) {
  shapes   = &master_data;
  textures = &tex;
  elements = shapes->size();
  cam_pos  = camera_pos;
  valid    = true;

  // Distances will be used for LOD extra credit
  // First pass: get distances to all centers, update
  // distance rangge
  distances.clear();
  distances.reserve(elements);
  min_dist = std::numeric_limits<float>::infinity();
  max_dist = 0.f;
  for (const auto &p : *shapes){
    auto  world_space_center = vec3(p.ctm * vec4(0, 0, 0, 1));
    float distance = glm::distance(cam_pos, world_space_center);

    // Update min and max distances
    if (distance < min_dist)
      min_dist = distance;
    if (distance > max_dist)
      max_dist = distance;

    distances.push_back(distance);
  }
  dist_range = max_dist - min_dist;

  // Second pass: normalize distances
  for (size_t i = 0; i != elements; ++i)
    distances[i] = 1.f - ((distances[i] - min_dist) / dist_range);

  // Clear vertex and mesh data since it's a completely new scene
  vertex_buffer_data.clear();
  uv_buffer_data.clear();
  normal_buffer_data.clear();
  unique_shape_starts.clear();
  mesh_vertex_buffer_data.clear();
  mesh_uv_buffer_data.clear();
  mesh_normal_buffer_data.clear();
  mesh_shape_descriptions.clear();

  // Create vertex and normal data
  update_data(true);
}

// Sets vertex and normal data based on a vector of
// RenderShapeData
void geometry_set::update_data(bool update_meshes) {
  if (!valid)
    return;

  // Only clear shape metadata, since that's the
  // only container that might get invalidated by a single
  // tessellation parameter change
  shape_descriptions.clear();
  shape_descriptions.reserve(elements);

  // If vertex data is getting too big (4 MB more or less),
  // clear VBOs and map pointing into them
  if (vertex_buffer_data.size() > 1000000) {
    vertex_buffer_data.clear();
    uv_buffer_data.clear();
    normal_buffer_data.clear();
    unique_shape_starts.clear();
  }

  // Create meshes for each of our shapes
  for (size_t i = 0; i != elements; ++i)
    add_shape_data((*shapes)[i], i, update_meshes);

  // Parallax mapping
  tangent_buffer_data = make_tangents(vertex_buffer_data, uv_buffer_data);
  if (update_meshes) {
    mesh_tangent_buffer_data = make_tangents(mesh_vertex_buffer_data,
      mesh_uv_buffer_data);
  }

  update_buffers(update_meshes);
}

// Auxiliary to render shapes in any VAO
void geometry_set::draw_shapes(const vector<shape_description> &vec) {
  for (const auto &d : vec) {
    // Send this object's model matrix
    glUniformMatrix4fv(model_u, 1, 0,
      &(*(d.model_matrix))[0][0]);
    glUniformMatrix4fv(inv_model_u, 1, 0,
      &(*(d.inv_model_matrix))[0][0]);

    // Send this object's color data
    glUniform3fv(amb_u, 1, &d.ambient[0]);
    glUniform3fv(dif_u, 1, &d.diffuse[0]);
    glUniform3fv(spe_u, 1, &d.specular[0]);
    glUniform1f(shi_u, d.shininess);
    glUniform1f(bld_u, d.tex_blend);
    glUniform1f(pav_u, d.has_par);
    glUniform1f(paf_u, d.has_par);

    // If this shape is using a texture, bind the
    // correct texture
    if (texturing && d.has_tex) {
      glUniform1i(tex_u, 1);
      (*textures)[d.tex_id].bind();
    } else {
      glUniform1i(tex_u, 0);
    }

    // Draw this shape
    glDrawArrays(mode, d.offset, d.points);

    // Unbind the texture if we used it
    if (texturing)
      (*textures)[d.tex_id].unbind();
  }
}

void geometry_set::draw() {
  if (!valid)
    return;

  // Draw standard shapes
  set_vao_tessellated();
  draw_shapes(shape_descriptions);

  // Extra credit: draw meshes if option is enabled
  if (meshes) {
    set_vao_meshes();
    draw_shapes(mesh_shape_descriptions);
  }

  unbind();
}

void geometry_set::set_vao_tessellated() {
  if (!valid)
    return;

  bind();

  // Set vertex attribute
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));

  // Send normal attribute
  glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));

  // Send UV attribute
  glBindBuffer(GL_ARRAY_BUFFER, ubo_id);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));

  // Send tangent attribute
  glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));
}

void geometry_set::set_vao_meshes() {
  if (!valid || !meshes)
    return;

  bind();

  // Set vertex attribute
  glBindBuffer(GL_ARRAY_BUFFER, mvo_id);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));

  // Send normal attribute
  glBindBuffer(GL_ARRAY_BUFFER, mno_id);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));

  // Send UV attribute
  glBindBuffer(GL_ARRAY_BUFFER, muo_id);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));

  // Send tangent attribute
  glBindBuffer(GL_ARRAY_BUFFER, mto_id);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<void*>(0));
}

void geometry_set::update_buffers(bool update_meshes) {
  if (!valid)
    return;

  bind();

  // Send vertex buffer data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER,
    vertex_buffer_data.size() * sizeof(float),
    vertex_buffer_data.data(), GL_STATIC_DRAW);

  // Send UV buffer data
  glBindBuffer(GL_ARRAY_BUFFER, ubo_id);
  glBufferData(GL_ARRAY_BUFFER,
    uv_buffer_data.size() * sizeof(float),
    uv_buffer_data.data(), GL_STATIC_DRAW);

  // Send normal buffer data
  glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
  glBufferData(GL_ARRAY_BUFFER,
    normal_buffer_data.size() * sizeof(float),
    normal_buffer_data.data(), GL_STATIC_DRAW);

  // Send tangent buffer data
  glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
  glBufferData(GL_ARRAY_BUFFER,
    tangent_buffer_data.size() * sizeof(float),
    tangent_buffer_data.data(), GL_STATIC_DRAW);

  // Only if we changed mesh data
  if (update_meshes) {
    // Send vertex buffer data
    glBindBuffer(GL_ARRAY_BUFFER, mvo_id);
    glBufferData(GL_ARRAY_BUFFER,
      mesh_vertex_buffer_data.size() * sizeof(float),
      mesh_vertex_buffer_data.data(), GL_STATIC_DRAW);

    // Send UV buffer data
    glBindBuffer(GL_ARRAY_BUFFER, muo_id);
    glBufferData(GL_ARRAY_BUFFER,
      mesh_uv_buffer_data.size() * sizeof(float),
      mesh_uv_buffer_data.data(), GL_STATIC_DRAW);

    // Send normal buffer data
    glBindBuffer(GL_ARRAY_BUFFER, mno_id);
    glBufferData(GL_ARRAY_BUFFER,
      mesh_normal_buffer_data.size() * sizeof(float),
      mesh_normal_buffer_data.data(), GL_STATIC_DRAW);

    // Send tangent buffer data
    glBindBuffer(GL_ARRAY_BUFFER, mto_id);
    glBufferData(GL_ARRAY_BUFFER,
      mesh_tangent_buffer_data.size() * sizeof(float),
      mesh_tangent_buffer_data.data(), GL_STATIC_DRAW);
  }

  unbind();
}

// Updates stored LOD bool, and updates
// vertex and normal data
void geometry_set::update_lod(bool new_lod) {
  lod = new_lod;
  update_data(false);
}

// Updates stored meshes bool
void geometry_set::update_meshes_display(bool new_meshes) {
  meshes = new_meshes;
}

// Updates stored texturing bool
void geometry_set::update_texturing(bool new_texturing) {
  texturing = new_texturing;
}

// Updates stored tessellation params, and updates
// vertex and normal data
void geometry_set::update_tessellation(int tess_0, int tess_1) {
  t_0 = tess_0;
  t_1 = tess_1;
  update_data(false);
}

geometry_set::~geometry_set() {
  unbind();

  // Cleanup VBOs
  glDeleteBuffers(1, &vbo_id);
  glDeleteBuffers(1, &ubo_id);
  glDeleteBuffers(1, &nbo_id);
  glDeleteBuffers(1, &tbo_id);
  glDeleteBuffers(1, &mvo_id);
  glDeleteBuffers(1, &muo_id);
  glDeleteBuffers(1, &mno_id);
  glDeleteBuffers(1, &mto_id);

  // Cleanup attributes
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(3);

  // Delete VAO
  glDeleteVertexArrays(1, &vao_id);
}

void geometry_set::bind() {
  glBindVertexArray(vao_id);
}

void geometry_set::unbind() {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void geometry_set::add_to_vec(vector<float> &vec, vec3 p) {
  vec.push_back(p.x);
  vec.push_back(p.y);
  vec.push_back(p.z);
}

void geometry_set::add_to_vec(vector<float> &vec, vec4 p) {
  vec.push_back(p.x);
  vec.push_back(p.y);
  vec.push_back(p.z);
}
