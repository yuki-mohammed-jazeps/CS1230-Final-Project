#include "mesh.h"

using std::string;
using glm::vec3;
using glm::vec2;
using std::vector;

// Auxiliary to insert into vertex or normal vectors
void insert_vec3(vector<float> &data, vec3 v) {
  data.push_back(v.x);
  data.push_back(v.y);
  data.push_back(v.z);
}

void insert_vec2(vector<float> &data, vec2 v) {
  data.push_back(v.x);
  data.push_back(v.y);
}

mesh::mesh(string fp) : loader(obj_loader(fp))
{
  valid = loader.success;
}

void mesh::set_vertex_data() {
  for (size_t i = 0; i != loader.faces.size(); i += 3) {
    auto vertex_indices = loader.faces[i];
    auto uv_indices     = loader.faces[i + 1];
    auto normal_indices = loader.faces[i + 2];

    insert_vec3(vertex_data, loader.vertices[vertex_indices[0]]);
    insert_vec3(vertex_data, loader.vertices[vertex_indices[1]]);
    insert_vec3(vertex_data, loader.vertices[vertex_indices[2]]);
    insert_vec2(uv_data, loader.uvs[uv_indices[0]]);
    insert_vec2(uv_data, loader.uvs[uv_indices[1]]);
    insert_vec2(uv_data, loader.uvs[uv_indices[2]]);
    insert_vec3(normal_data, normalize(loader.normals[normal_indices[0]]));
    insert_vec3(normal_data, normalize(loader.normals[normal_indices[1]]));
    insert_vec3(normal_data, normalize(loader.normals[normal_indices[2]]));
  }
}

void mesh::make_mesh() {
  if (!valid)
    return;
  set_vertex_data();
}
