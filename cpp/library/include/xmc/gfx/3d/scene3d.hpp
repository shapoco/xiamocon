#ifndef XMC_GFX_SCENE3D_HPP
#define XMC_GFX_SCENE3D_HPP

#include "xmc/gfx/3d/node3d.hpp"

namespace xmc {

class Scene3DClass {
 public:
  std::vector<Node3D> rootNodes;

  void addNode(Node3D node) { rootNodes.push_back(node); }
};

using Scene3D = std::shared_ptr<Scene3DClass>;

static inline Scene3D createScene3D() {
  return std::make_shared<Scene3DClass>();
}

}  // namespace xmc

#endif
