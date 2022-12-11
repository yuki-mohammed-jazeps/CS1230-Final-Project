#include "sceneparser.h"
#include "scenefilereader.h"
#include "transforms.h"

#include <chrono>
#include <memory>

using std::vector;	using glm::inverse;
using glm::mat4;	using std::string;

long recursive_traversal(const mat4 &ctm, const SceneNode *root,
                         vector<RenderShapeData> &primitives, long id) {
    // Update the CTM with this node's transforms
    auto new_ctm = transforms::apply_transforms(ctm, root->transformations);
    auto inv_ctm = inverse(new_ctm);

    // Add primitives that might be in this node to the final vector
    for(auto &p : root->primitives)
        primitives.push_back(RenderShapeData{id++, *p, new_ctm, inv_ctm});

    // Recurse for children
    for(const auto &c : root->children)
        id = recursive_traversal(new_ctm, c, primitives, id);

    return id;
}

bool SceneParser::parse(string filepath, RenderData &renderData) {
  ScenefileReader fileReader = ScenefileReader(filepath);
  bool success = fileReader.readXML();
  if (!success)
    return false;

  renderData.globalData = fileReader.getGlobalData();
  renderData.cameraData = fileReader.getCameraData();
  renderData.lights     = fileReader.getLights();

  renderData.shapes.clear();
  recursive_traversal(mat4(1.0f), fileReader.getRootNode(), renderData.shapes, 0);

  return true;
}
