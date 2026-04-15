#ifndef XMC_GFX3D_COMMON_HPP
#define XMC_GFX3D_COMMON_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx2d/colorf.hpp"

namespace xmc {

enum class RenderFlags3D : uint32_t {
  NONE,
  VERTEX_COLOR = 1 << 0,
  VERTEX_NORMAL = 1 << 1,
  GOURAUD_SHADING = 1 << 2,
  LIGHTING = 1 << 3,
  COLOR_TEXTURE = 1 << 4,
  CUSTOM_VERTEX_SHADER = 1 << 6,
  CUSTOM_PIXEL_SHADER = 1 << 7,
  Z_TEST = 1 << 8,
  Z_UPDATE = 1 << 9,
  DEFAULT = VERTEX_COLOR | VERTEX_NORMAL | GOURAUD_SHADING | LIGHTING |
            COLOR_TEXTURE | Z_TEST | Z_UPDATE,
  ALL = 0xFFFFFFFF,
};
XMC_ENUM_FLAGS(RenderFlags3D, uint32_t);

struct Vertex3D {
  vec3 pos;
  vec3 normal;
  colorf color;
  vec2 uv;
};

}  // namespace xmc

#endif
