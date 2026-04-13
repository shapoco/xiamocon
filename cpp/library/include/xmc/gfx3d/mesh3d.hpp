#ifndef XMC_GFX_MESH3D_HPP
#define XMC_GFX_MESH3D_HPP

#include "xmc/gfx3d/primitive3d.hpp"

#include <memory>
#include <vector>

namespace xmc {

class Mesh3DClass {
 public:
  std::vector<Primitive3D> primitives;

  Mesh3DClass(std::vector<Primitive3D> &&prims)
      : primitives(std::move(prims)) {}

  void addPrimitive(Primitive3D prim) { primitives.push_back(prim); }

  void setMaterial(Material3D mat) {
    for (Primitive3D &prim : primitives) {
      prim->material = mat;
    }
  }
};

using Mesh3D = std::shared_ptr<Mesh3DClass>;

static inline Mesh3D createMesh3D(std::vector<Primitive3D> &&prims) {
  return std::make_shared<Mesh3DClass>(std::move(prims));
}

/**
 * @brief Create a cube mesh centered at the origin with side length s
 * @param s The side length of the cube
 * @param uv Whether to include UV coordinates
 * @return A Mesh3D object representing the cube
 */
Mesh3D createCube(float s = 1.0f, bool uv = true);

/**
 * @brief Create a colored cube mesh centered at the origin with side length s
 * @param s The side length of the cube
 * @return A Mesh3D object representing the cube
 */
Mesh3D createColoredCube(float s = 1.0f);

/**
 * @brief Create a colored sphere mesh centered at the origin with given radius
 * and tessellation
 * @param radius The radius of the sphere
 * @param segments The number of segments around the equator (longitude)
 * @param rings The number of rings from pole to pole (latitude)
 * @param col The color of the sphere
 * @return A Mesh3D object representing the sphere
 */
Mesh3D createSphere(float radius = 1.0f, int segments = 12, int rings = 6,
                    colorf col = {1.0f, 1.0f, 1.0f, 1.0f});

}  // namespace xmc

#endif
