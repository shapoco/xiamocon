#ifndef XMC_GFX_NODE3D_HPP
#define XMC_GFX_NODE3D_HPP

#include "xmc/gfx3d/mesh3d.hpp"

namespace xmc {

class Node3DClass {
 public:
  Mesh3D mesh;
  mat4 transform;
  std::vector<std::shared_ptr<Node3DClass>> children;

  Node3DClass(Mesh3D mesh, mat4 transform = mat4::identity())
      : mesh(mesh), transform(transform) {}
};

using Node3D = std::shared_ptr<Node3DClass>;

static inline Node3D createNode3D(Mesh3D mesh, mat4 transform = mat4::identity()) {
  return std::make_shared<Node3DClass>(mesh, transform);
}

}  // namespace xmc

#endif
