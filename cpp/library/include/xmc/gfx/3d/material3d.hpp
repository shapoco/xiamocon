#ifndef XMC_GFX_MATERIAL3D_HPP
#define XMC_GFX_MATERIAL3D_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx/colorf.hpp"
#include "xmc/gfx/sprite.hpp"

namespace xmc {

class Material3DClass {
 public:
  colorf baseColor = {1.0f, 1.0f, 1.0f, 1.0f};
  // colorf emissiveColor = {0.0f, 0.0f, 0.0f, 1.0f};
  // float metalness = 0.0f;
  // float roughness = 0.5f;
  Sprite colorTexture = nullptr;
  bool doubleSided = false;
};

using Material3D = std::shared_ptr<Material3DClass>;

static inline Material3D createMaterial3D() {
  return std::make_shared<Material3DClass>();
}

}  // namespace xmc

#endif
