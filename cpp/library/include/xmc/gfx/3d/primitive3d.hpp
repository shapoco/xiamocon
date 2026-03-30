#ifndef XMC_GFX_PRIMITIVE3D_HPP
#define XMC_GFX_PRIMITIVE3D_HPP

#include "xmc/gfx/attribute_buffer.hpp"
#include "xmc/gfx/3d/material3d.hpp"

#include <memory>

namespace xmc {

class Primitive3DClass {
 public:
  PrimitiveMode mode;
  Vec3Buffer position;
  Vec3Buffer normal;
  ColorBuffer color;
  Vec2Buffer uv;
  IndexBuffer indexes;
  Material3D material;
  Primitive3DClass(PrimitiveMode mode, Vec3Buffer pos, Vec3Buffer norm,
                 ColorBuffer col, Vec2Buffer uv, IndexBuffer idx = nullptr,
                 Material3D mat = nullptr)
      : mode(mode),
        position(pos),
        normal(norm),
        color(col),
        uv(uv),
        indexes(idx),
        material(mat) {}

  inline int numVertices() {
    if (indexes) {
      return indexes->size;
    } else if (position) {
      return position->size;
    }
    return 0;
  }

  inline int numElements() {
    int n = numVertices();
    switch (mode) {
      case PrimitiveMode::POINTS: return n;
      case PrimitiveMode::LINES: return n / 2;
      case PrimitiveMode::LINE_LOOP: return n < 2 ? 0 : n;
      case PrimitiveMode::LINE_STRIP: return n < 2 ? 0 : n - 1;
      case PrimitiveMode::TRIANGLES: return n / 3;
      case PrimitiveMode::TRIANGLE_STRIP: return n < 3 ? 0 : n - 2;
      case PrimitiveMode::TRIANGLE_FAN: return n < 3 ? 0 : n - 2;
      default: return 0;
    }
  }
};

using Primitive3D = std::shared_ptr<Primitive3DClass>;

static inline Primitive3D createPrimitive3D(PrimitiveMode mode, Vec3Buffer pos,
                                        Vec3Buffer norm, ColorBuffer col,
                                        Vec2Buffer uv = nullptr,
                                        IndexBuffer idx = nullptr,
                                        Material3D mat = nullptr) {
  return std::make_shared<Primitive3DClass>(mode, pos, norm, col, uv, idx, mat);
}

}  // namespace xmc

#endif
