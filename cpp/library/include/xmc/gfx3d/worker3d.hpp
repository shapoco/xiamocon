#ifndef XMC_GFX_WORKER3D_HPP
#define XMC_GFX_WORKER3D_HPP

#include "xmc/gfx2d/raster_scan.hpp"
#include "xmc/gfx2d/sprite.hpp"
#include "xmc/gfx3d/gfx3d_common.hpp"

namespace xmc {

enum class MultiCoreMode3D {
  NONE,
  INTERLACE,
  PIPELINE,
};

struct WorkerArgs3D {
  RenderFlags3D renderFlags = RenderFlags3D::DEFAULT;
  BlendMode blendMode = BlendMode::OVERWRITE;
  Sprite target = nullptr;
  Depth3D *depthBuff = nullptr;
  uint16_t depthStride = 0;
  int32_t zTestOffset = 0;
  PixelFormat textureFormat = PixelFormat::RGB565;
  const uint16_t *textureData = nullptr;
  uint16_t textureStride = 0;
  uint16_t textureWidth = 0;
  uint16_t textureHeight = 0;
  uint16_t uMask = 0;
  uint16_t vMask = 0;
  uint8_t vShift = 0;
  int16_t xMin = 0;
  int16_t xMax = 240;
};

struct EdgeScanVars {
  fixed16p16 x = 0;
  fixed20p12 z = 0;
  color8p24 c = {0, 0, 0, 0};
  fixed12p20 u = 0;
  fixed12p20 v = 0;

  static XMC_INLINE EdgeScanVars subDiv(const EdgeScanVars &start,
                                        const EdgeScanVars &end, int32_t span,
                                        bool gouraudShading = true,
                                        bool textureMapping = true) {
    if (span <= 0) span = 1;
    return EdgeScanVars{
        (end.x - start.x) / span,
        (end.z - start.z) / span,
        gouraudShading ? (end.c - start.c) / span : color8p24(0, 0, 0, 0),
        textureMapping ? (end.u - start.u) / span : fixed12p20(0),
        textureMapping ? (end.v - start.v) / span : fixed12p20(0),
    };
  }

  XMC_INLINE void step(const EdgeScanVars &step, int n = 1,
                       bool gouraudShading = true, bool textureMapping = true) {
    if (n == 1) {
      x += step.x;
      z += step.z;
      if (gouraudShading) {
        c += step.c;
      }
      if (textureMapping) {
        u += step.u;
        v += step.v;
      }
    } else {
      x += step.x * n;
      z += step.z * n;
      if (gouraudShading) {
        c += step.c * n;
      }
      if (textureMapping) {
        u += step.u * n;
        v += step.v * n;
      }
    }
  }
};

struct Trapezoid3D {
  EdgeScanVars topLeft;
  EdgeScanVars topRight;
  EdgeScanVars stepLeft;
  EdgeScanVars stepRight;
  int yTop;
  int yBottom;
  int yStep;
};

class Worker3D {
 public:
  static constexpr int FIFO_DEPTH = 4;

 private:
  WorkerArgs3D args;
  Trapezoid3D fifo[FIFO_DEPTH];
  volatile int fifoWrPtr = 0;
  volatile int fifoRdPtr = 0;
  volatile bool full = false;

 public:
  void beginPrimitive(const WorkerArgs3D &args);
  void push(const Trapezoid3D &trap);
  void endPrimitive();
  inline bool isFull() const { return full; }
  void service();
};

XMC_INLINE uint32_t getTrapezoidRenderMode(const WorkerArgs3D &args) {
  uint32_t mode = 0;
  if (args.target->format == PixelFormat::RGB444) {
    mode |= (1 << 0);
  }
  if (hasFlag(args.renderFlags, RenderFlags3D::GOURAUD_SHADING)) {
    mode |= (1 << 1);
  }
  if (hasFlag(args.renderFlags, RenderFlags3D::COLOR_TEXTURE)) {
    if (args.textureFormat == PixelFormat::RGB565) {
      mode |= (1 << 2);
    } else if (args.textureFormat == PixelFormat::ARGB4444) {
      mode |= (1 << 2) | (1 << 3);
    }
  }
  if (args.blendMode != BlendMode::OVERWRITE) {
    mode |= (1 << 4);
  }
  return mode;
}

void renderTrapezoid3D(WorkerArgs3D &args, const Trapezoid3D &trap,
                       uint32_t mode);

}  // namespace xmc

#endif  // XMC_GFX_WORKER3D_HPP
