#include "obj_loader.h"
#include <fstream>
#include <iterator>
#include <sstream>
#include <iostream>

using std::string;    using std::ifstream;
using std::vector;    using std::istringstream;
using glm::vec3;      using std::getline;
using glm::vec2;      using std::stof;
using std::cerr;      using std::endl;
using std::cout;

// Auxiliary for trimming strings
inline int is_not_space(int ch) {
  return !std::isspace(ch);
}

inline int is_comment(int ch) {
  return ch == '#';
}

// Trim whitespace from a string
void trim(string &s) {
  // Strip comments
  auto left_comment = std::find_if(s.begin(), s.end(), is_comment);
  s.erase(left_comment, s.end());

  // Strip trailing whitespace
  auto from_left    = std::find_if(s.begin(), s.end(), is_not_space);
  auto from_right   = std::find_if(s.rbegin(), s.rend(), is_not_space).base();

  s.erase(s.begin(), from_left);
  s.erase(from_right, s.end());
}

// Split into vector of strings
vector<string> split(const string &s) {
  auto stream = std::stringstream(s);
  auto begin  = std::istream_iterator<string>(stream);
  std::istream_iterator<string> end;

  return vector<string>(begin, end);
}

// Split based on / char
vector<size_t> face_split(const string &s) {
  auto stream = std::stringstream(s);
  string temp;
  vector<size_t> ret;

  while (getline(stream, temp, '/')) {
    if (temp == "")
      temp = "0";

    ret.push_back(stoi(temp));
  }

  return ret;
}

// Auxiliary for indexing using OBJ rules (relative indices)
size_t obj_at(size_t idx, size_t last=0) {
  if (idx > 0)
    return idx - 1;
  else
    return obj_at(last - (idx + 1));
}

obj_loader::obj_loader(string fp) : success(false)
{
  // File stream
  auto in_f = ifstream(fp);

  // Couldn't open file
  if (!in_f) {
    cerr << "Could not open file " << fp << endl;
    return;
  }

  // Read line by line
  string line;
  size_t last_vertex = 0;
  size_t last_uv     = 0;
  size_t last_normal = 0;
  while (getline(in_f, line)) {
    trim(line);
    if (line == "")
      continue;

    auto toks = split(line);

    // Vertex
    if (toks[0] == "v") {
      if (toks.size() != 4) {
        cerr << "Malformed obj file: " << line << endl;
        return;
      }

      vertices.push_back(vec3(stof(toks[1]), stof(toks[2]), stof(toks[3])));
      ++last_vertex;
    // UV
    } else if (toks[0] == "vt") {
      if (toks.size() != 3) {
        cerr << "Malformed obj file: " << line << endl;
        return;
      }

      uvs.push_back(vec2(stof(toks[1]), stof(toks[2])));
      ++last_uv;
    // Normal
    } else if (toks[0] == "vn") {
      if (toks.size() != 4) {
        cerr << "Malformed obj file: " << line << endl;
        return;
      }

      normals.push_back(vec3(stof(toks[1]), stof(toks[2]), stof(toks[3])));
      ++last_normal;
    // Face
    } else if (toks[0] == "f") {
      if (toks.size() < 4) {
        cerr << "Malformed obj file: " << line << endl;
        return;
      }

      // It's a simple tri
      if (toks.size() == 4) {
        auto face_toks_0 = face_split(toks[1]);
        auto face_toks_1 = face_split(toks[2]);
        auto face_toks_2 = face_split(toks[3]);

        // Vertex indices
        faces.push_back(vec3(obj_at(face_toks_0[0]),
                             obj_at(face_toks_1[0]),
                             obj_at(face_toks_2[0])));
        // UV indices
        faces.push_back(vec3(obj_at(face_toks_0[1]),
                             obj_at(face_toks_1[1]),
                             obj_at(face_toks_2[1])));
        // Normal indices
        faces.push_back(vec3(obj_at(face_toks_0[2]),
                             obj_at(face_toks_1[2]),
                             obj_at(face_toks_2[2])));
      // It's an n-gon, screw you monsterdance.obj
      } else {
        auto face_toks_0 = face_split(toks[1]);

        for (size_t i = 1; i != toks.size() - 1; ++i) {
          auto face_toks_1 = face_split(toks[i]);
          auto face_toks_2 = face_split(toks[i + 1]);

          // Vertex indices
          faces.push_back(vec3(obj_at(face_toks_0[0]),
                               obj_at(face_toks_1[0]),
                               obj_at(face_toks_2[0])));
          // UV indices
          faces.push_back(vec3(obj_at(face_toks_0[1]),
                               obj_at(face_toks_1[1]),
                               obj_at(face_toks_2[1])));
          // Normal indices
          faces.push_back(vec3(obj_at(face_toks_0[2]),
                               obj_at(face_toks_1[2]),
                               obj_at(face_toks_2[2])));
        }
      }
    }
  }

  if (uvs.empty())
    uvs.push_back(vec2(0.f, 0.f));

  success = true;
}
