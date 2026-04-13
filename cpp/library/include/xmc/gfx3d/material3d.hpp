#ifndef XMC_GFX_MATERIAL3D_HPP
#define XMC_GFX_MATERIAL3D_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx2d/colorf.hpp"
#include "xmc/gfx2d/sprite.hpp"
#include "xmc/gfx3d/gfx3d_common.hpp"

namespace xmc {

enum class MaterialFlags3D : uint32_t {
  NONE = 0,
  HAS_BASE_COLOR = 1 << 0,
  DOUBLE_SIDED = 1 << 1,
  ENVIRONMENT_MAPPED = 1 << 2,
};
XMC_ENUM_FLAGS(MaterialFlags3D, uint32_t);

class VertexShader;

class Material3DClass {
 public:
  MaterialFlags3D flags = MaterialFlags3D::NONE;
  colorf baseColor = {1.0f, 1.0f, 1.0f, 1.0f};
  // colorf emissiveColor = {0.0f, 0.0f, 0.0f, 1.0f};
  // float metalness = 0.0f;
  // float roughness = 0.5f;
  Sprite colorTexture = nullptr;
  VertexShader *vertexShader = nullptr;
};

using Material3D = std::shared_ptr<Material3DClass>;

static inline Material3D createMaterial3D() {
  return std::make_shared<Material3DClass>();
}

}  // namespace xmc

#endif
