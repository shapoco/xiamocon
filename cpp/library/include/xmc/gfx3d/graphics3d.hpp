#ifndef XMC_GFX_RASTERIZER_HPP
#define XMC_GFX_RASTERIZER_HPP

#include "xmc/gfx2d/color8p24.hpp"
#include "xmc/gfx2d/raster_scan.hpp"
#include "xmc/gfx2d/sprite444.hpp"
#include "xmc/gfx3d/gfx3d_common.hpp"
#include "xmc/gfx3d/scene3d.hpp"

namespace xmc {

using depth_t = uint16_t;
static constexpr depth_t MAX_DEPTH = (1 << (sizeof(depth_t) * 8)) - 1;

enum class TrapezoidFlags : uint8_t {
  NONE = 0,
  OUTPUT_444 = 1 << 0,
  GOURAUD_SHADING = 1 << 1,
  COLOR_TEXTURE = 1 << 2,
  TEXTURE_4444 = 1 << 3,
  BLEND = 1 << 4,
};
XMC_ENUM_FLAGS(TrapezoidFlags, uint8_t);

struct TextureArgs {
  const uint16_t *data = nullptr;
  uint32_t stride = 0;
  uint32_t uMask = 0;
  uint32_t vMask = 0;
};

struct ScanLineCounter {
  fixed16p16 x = 0;
  fixed20p12 z = 0;
  color8p24 c = {0, 0, 0, 0};
  fixed12p20 u = 0;
  fixed12p20 v = 0;

  static XMC_INLINE ScanLineCounter subDiv(const ScanLineCounter &start,
                                           const ScanLineCounter &end,
                                           int32_t span) {
    if (span <= 0) span = 1;
    return ScanLineCounter{
        // fixed16p16::subDiv(end.x, start.x, span),
        (end.x - start.x) / span,
        // fixed20p12::subDiv(end.z, start.z, span),
        (end.z - start.z) / span,
        (end.c - start.c) / span,
        (end.u - start.u) / span,
        (end.v - start.v) / span,
    };
  }

  XMC_INLINE void step(const ScanLineCounter &step, int n = 1) {
    if (n == 1) {
      x += step.x;
      z += step.z;
      c += step.c;
      u += step.u;
      v += step.v;
    } else {
      x += step.x * n;
      z += step.z * n;
      c += step.c * n;
      u += step.u * n;
      v += step.v * n;
    }
  }
};

class Graphics3DClass;
using Graphics3D = std::shared_ptr<Graphics3DClass>;

static inline Graphics3D createGraphics3D(int width, int height,
                                          uint32_t stackSize = 16) {
  return std::make_shared<Graphics3DClass>(width, height, stackSize);
}

struct MatrixStackEntry {
  mat4 local = mat4::identity();
  mat4 world = mat4::identity();
  bool dirty = true;
};

class Graphics3DClass {
 private:
  const int width;
  const int height;
  const int stackSize;

  depth_t *depthBuff;

  Sprite target;
  Rect viewport;
  vec3 eyePosition;

  mat4 screenMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  mat4 viewProjectionMatrix;
  bool viewProjectionDirty = true;
  mat4 mvpMatrix;
  bool mvpDirty = true;

  RenderFlags3D renderFlags = RenderFlags3D::DEFAULT;
  BlendMode blendMode = BlendMode::OVERWRITE;

  MatrixStackEntry *modelMatrixStack;
  int modelMatrixStackPtr = 0;

  float zNear = 0.01f;
  float zFar = 100.0f;
  int32_t zTestOffset = 0;

  colorf envLight = {0.1f, 0.1f, 0.1f, 1.0f};

  vec3 parallelLightDir = vec3(0.5f, 0.5f, 1.0f).normalized();
  colorf parallelLightColor = {1, 1, 1, 1};

  Material3D material;

 public:
  Graphics3DClass(int w, int h, uint32_t stackSize)
      : width(w), height(h), stackSize(stackSize) {
    depthBuff = (depth_t *)xmcMalloc(sizeof(depth_t) * width * height,
                                     XMC_RAM_CAP_SPIRAM);
    modelMatrixStack = (MatrixStackEntry *)xmcMalloc(
        sizeof(MatrixStackEntry) * stackSize, XMC_RAM_CAP_DMA);
    screenMatrix = mat4::identity();
    projectionMatrix = mat4::identity();
    viewMatrix = mat4::identity();
    loadIdentity();
  };

  ~Graphics3DClass() {
    if (depthBuff) {
      xmcFree(depthBuff);
      depthBuff = nullptr;
    }
    if (modelMatrixStack) {
      xmcFree(modelMatrixStack);
      modelMatrixStack = nullptr;
    }
  }

  void setTarget(Sprite target, Rect viewport);

  inline void setTarget(Sprite target) {
    setTarget(target, Rect{0, 0, target->width, target->height});
  }

  inline RenderFlags3D getFlags() const { return renderFlags; }
  inline void setFlags(RenderFlags3D flags) { renderFlags = flags; }
  inline void enableFlags(RenderFlags3D flags) { renderFlags |= flags; }
  inline void disableFlags(RenderFlags3D flags) { renderFlags &= ~flags; }

  inline BlendMode getBlendMode() const { return blendMode; }
  inline void setBlendMode(BlendMode mode) { blendMode = mode; }

  inline void setEnvironmentLight(const colorf &color) { envLight = color; }

  inline void setParallelLight(const vec3 &dir, const colorf &color) {
    parallelLightDir = dir.normalized();
    parallelLightColor = color;
  }

  inline void setScreenMatrix(const mat4 &mat) {
    screenMatrix = mat;
    viewProjectionDirty = true;
    mvpDirty = true;
  }

  inline void setProjection(const mat4 &mat) {
    projectionMatrix = mat;
    viewProjectionDirty = true;
    mvpDirty = true;
  }
  inline const mat4 &getProjection() const { return projectionMatrix; }

  inline void setOrthoProjection(float left, float right, float bottom,
                                 float top, float near = 0.01f,
                                 float far = 100.0f) {
    setProjection(mat4::ortho(left, right, bottom, top, near, far));
  }
  inline void setPerspectiveProjection(float fovY, float aspect,
                                       float near = 0.01f, float far = 100.0f) {
    setProjection(mat4::perspective(fovY, aspect, near, far));
  }

  inline void setViewMatrix(const mat4 &mv) {
    viewMatrix = mv;
    viewProjectionDirty = true;
    mvpDirty = true;
  }

  inline const mat4 &getViewMatrix() const { return viewMatrix; }

  inline void lookAt(const vec3 &eye, const vec3 &focus, const vec3 &up) {
    eyePosition = eye;
    setViewMatrix(mat4::lookAt(eye, focus, up));
  }

  inline void loadMatrix(const mat4 &m) { dirtyModelMatrix() = m; }

  inline void loadIdentity() { loadMatrix(mat4::identity()); }

  inline void pushMatrix() {
    if (modelMatrixStackPtr < stackSize - 1) {
      modelMatrixStackPtr++;
      modelMatrixStack[modelMatrixStackPtr].local = mat4::identity();
      modelMatrixStack[modelMatrixStackPtr].dirty = true;
      mvpDirty = true;
    }
  }

  inline void popMatrix() {
    if (modelMatrixStackPtr > 0) {
      modelMatrixStackPtr--;
      mvpDirty = true;
    }
  }

  void getModelMatrix(mat4 &out);
  void getMvpMatrix(mat4 &out);

  inline void transform(const mat4 &t) { dirtyModelMatrix() *= t; }

  inline void translate(const vec3 &t) { dirtyModelMatrix().translate(t); }

  inline void translate(float x, float y, float z) {
    dirtyModelMatrix().translate(x, y, z);
  }

  inline void rotate(const quat &q) { dirtyModelMatrix().rotate(q); }

  inline void rotate(float pitch, float yaw, float roll) {
    dirtyModelMatrix().rotate(pitch, yaw, roll);
  }

  inline void rotate(const vec3 &axis, float angle) {
    dirtyModelMatrix().rotate(axis, angle);
  }

  inline void scale(const vec3 &s) { dirtyModelMatrix().scale(s); }

  inline void scale(float s) { dirtyModelMatrix().scale(s); }

  inline void setDepthRange(float near, float far) {
    zNear = near;
    zFar = far;
  }

  inline int32_t getZTestOffset() const { return zTestOffset; }
  inline void setZTestOffset(int32_t offset) { zTestOffset = offset; }

  inline void setMaterial(const Material3D &mat) { material = mat; }

  void clearDepth(depth_t value = MAX_DEPTH);

  void renderScene(const Scene3D &scene, void *userContext = nullptr);

  void renderNode(const Node3D &node, void *userContext = nullptr);

  void renderMesh(const Mesh3D &mesh, void *userContext = nullptr);

  void renderPrimitive(const Primitive3D &prim, void *userContext = nullptr);

  template <bool BLEND, bool TEXTURE_4444, bool COLOR_TEXTURE,
            bool GOURAUD_SHADING, bool OUTPUT_444>
  XMC_INLINE void renderTrapezoid(ScanLineCounter &accumVL,
                                  ScanLineCounter &accumVR,
                                  ScanLineCounter &stepVL,
                                  ScanLineCounter &stepVR, int yStart, int yEnd,
                                  const TextureArgs &tex) {
    int32_t ztOffset = zTestOffset;
    if (!hasFlag(renderFlags, RenderFlags3D::Z_TEST)) {
      ztOffset = -MAX_DEPTH;
    }
    for (int iy = yStart; iy < yEnd; iy++) {
      int32_t ixMin = accumVL.x.roundToInt();
      int32_t ixMax = accumVR.x.roundToInt();
      int32_t hSpan = ixMax - ixMin;

      if (ixMin < viewport.x) ixMin = viewport.x;
      if (ixMax > viewport.right()) ixMax = viewport.right();

      if (ixMin < ixMax) {
        ScanLineCounter accumH = accumVL;
        ScanLineCounter stepH =
            ScanLineCounter::subDiv(accumVL, accumVR, hSpan);

        int32_t xOffset = ixMin - accumVL.x.roundToInt();
        accumH.step(stepH, xOffset);

        uint16_t *__restrict__ texData = (uint16_t *)tex.data;
        depth_t *__restrict__ zPtr = depthBuff + iy * width;
        RasterScan444 cPtr444((uint8_t *)target->linePtr(iy), ixMin);
        uint16_t *__restrict__ ptr565 = (uint16_t *)target->linePtr(iy) + ixMin;
        for (int x = ixMin; x < ixMax; x++) {
          int32_t z = accumH.z.floorToInt();
          bool written = false;
          if (z + ztOffset < (int32_t)zPtr[x]) {
            if (GOURAUD_SHADING || BLEND) {
              color8p24 c = accumH.c;
              if (COLOR_TEXTURE) {
                uint32_t u = accumH.u.floorToInt() & tex.uMask;
                uint32_t v = accumH.v.floorToInt() & tex.vMask;
                if (TEXTURE_4444) {
                  c.multSelf4444(texData[v * tex.stride + u]);
                } else {
                  c.multSelf565(texData[v * tex.stride + u]);
                }
              }
              if (BLEND) {
                if (c.a.raw > 0) {
                  if (blendMode == BlendMode::ADD) {
                    if (OUTPUT_444) {
                      uint16_t dst444 = cPtr444.peek();
                      cPtr444.push444(add8p24To444(dst444, c));
                    } else {
                      uint16_t dst565 = *ptr565;
                      *(ptr565++) = add8p24To565(dst565, c);
                    }
                  } else {
                    if (OUTPUT_444) {
                      uint16_t dst444 = cPtr444.peek();
                      cPtr444.push444(blend8p24To444(dst444, c));
                    } else {
                      uint16_t dst565 = *ptr565;
                      *(ptr565++) = blend8p24To565(dst565, c);
                    }
                  }
                  zPtr[x] = z;
                  written = true;
                }
              } else {
                if (OUTPUT_444) {
                  cPtr444.push444(c.to444());
                } else {
                  *(ptr565++) = c.to565();
                }
                written = true;
                zPtr[x] = z;
              }
            } else {
              uint16_t c;
              if (COLOR_TEXTURE) {
                uint32_t u = accumH.u.floorToInt() & tex.uMask;
                uint32_t v = accumH.v.floorToInt() & tex.vMask;
                c = texData[v * tex.stride + u];
                if (!TEXTURE_4444 && OUTPUT_444) {
                  c = convert565To444(c);
                } else if (TEXTURE_4444 && !OUTPUT_444) {
                  c = convert444To565(c);
                }
              } else {
                if (OUTPUT_444) {
                  c = accumH.c.to444();
                } else {
                  c = accumH.c.to565();
                }
              }
              if (OUTPUT_444) {
                cPtr444.push444(c);
              } else {
                *(ptr565++) = c;
              }
              written = true;
              zPtr[x] = z;
            }
          }
          if (!written) {
            if (OUTPUT_444) {
              cPtr444.skip();
            } else {
              ptr565++;
            }
          }
          accumH.step(stepH);
        }
      }

      accumVL.step(stepVL);
      accumVR.step(stepVR);
    }
  }

  void renderTriangle(const Vertex3D &v0, const Vertex3D &v1,
                      const Vertex3D &v2, const Material3D &mat) {
    RenderFlags3D flags = renderFlags;
    if (!mat || !mat->colorTexture ||
        !hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
      flags &= ~RenderFlags3D::COLOR_TEXTURE;
    }

    int vpl = viewport.x;
    int vpr = viewport.right();
    if (v0.pos.x < vpl && v1.pos.x < vpl && v2.pos.x < vpl) return;
    if (v0.pos.x >= vpr && v1.pos.x >= vpr && v2.pos.x >= vpr) return;

    if (!mat || !hasFlag(mat->flags, MaterialFlags3D::DOUBLE_SIDED)) {
      // back-face culling
      float ax = v1.pos.x - v0.pos.x;
      float ay = v1.pos.y - v0.pos.y;
      float bx = v2.pos.x - v0.pos.x;
      float by = v2.pos.y - v0.pos.y;
      if (ax * by - ay * bx >= 0) return;
    }

    const Vertex3D *tri[] = {&v0, &v1, &v2};

    int i0 = 0;
    int i1 = 1;
    int i2 = 2;

    // sort vertices by y-coordinate
    if (tri[i0]->pos.y > tri[i1]->pos.y) {
      int t = i0;
      i0 = i1;
      i1 = t;
    }
    if (tri[i1]->pos.y > tri[i2]->pos.y) {
      int t = i1;
      i1 = i2;
      i2 = t;
    }
    if (tri[i0]->pos.y > tri[i1]->pos.y) {
      int t = i0;
      i0 = i1;
      i1 = t;
    }

    float x0 = tri[i0]->pos.x, y0 = tri[i0]->pos.y;
    float x1 = tri[i1]->pos.x, y1 = tri[i1]->pos.y;
    float x2 = tri[i2]->pos.x, y2 = tri[i2]->pos.y;

    float z0 = (float)MAX_DEPTH * (tri[i0]->pos.z - zNear) / (zFar - zNear);
    float z1 = (float)MAX_DEPTH * (tri[i1]->pos.z - zNear) / (zFar - zNear);
    float z2 = (float)MAX_DEPTH * (tri[i2]->pos.z - zNear) / (zFar - zNear);
    if (z0 < 0 || z1 < 0 || z2 < 0 || z0 > MAX_DEPTH || z1 > MAX_DEPTH ||
        z2 > MAX_DEPTH) {
      return;
    }

    int iy0 = (int)floorf(y0);
    int iy1 = (int)roundf(y1);
    int iy2 = (int)ceilf(y2);
    int vSpan = (int)fmaxf(iy2 - iy0, 1);
    int vSpanT = (int)fmaxf(iy1 - iy0, 1);
    int vSpanB = (int)fmaxf(iy2 - iy1, 1);
    int iyMin = iy0;
    int iyMax = iy2;
    if (iyMin < viewport.y) iyMin = viewport.y;
    if (iyMax >= viewport.bottom()) iyMax = viewport.bottom() - 1;
    if (iyMin > iyMax) return;

    float yT = (float)vSpanT / vSpan;
    ScanLineCounter cornerT, cornerL, cornerR, cornerB;

    cornerT.x = fixed16p16::fromFloat(x0);
    cornerL.x = fixed16p16::fromFloat(x1);
    cornerR.x = fixed16p16::fromFloat(x0 + (x2 - x0) * yT);
    cornerB.x = fixed16p16::fromFloat(x2);

    cornerT.z = fixed20p12::fromFloat(z0);
    cornerL.z = fixed20p12::fromFloat(z1);
    cornerR.z = fixed20p12::fromFloat(z0 + (z2 - z0) * yT);
    cornerB.z = fixed20p12::fromFloat(z2);

    if (hasFlag(flags, RenderFlags3D::GOURAUD_SHADING)) {
      const colorf &c0 = tri[i0]->color;
      const colorf &c1 = tri[i1]->color;
      const colorf &c2 = tri[i2]->color;
      cornerT.c = color8p24(c0);
      cornerL.c = color8p24(c1);
      cornerR.c = color8p24(c0 + (c2 - c0) * yT);
      cornerB.c = color8p24(c2);
    }

    TextureArgs tex;
    if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
      const auto &t = mat->colorTexture;
      tex.data = (const uint16_t *)t->linePtr(0);
      tex.stride = t->stride / sizeof(uint16_t);
      int texW = 1 << (int)floorf(log2f(t->width));
      int texH = 1 << (int)floorf(log2f(t->height));
      tex.uMask = texW - 1;
      tex.vMask = texH - 1;
      const vec2 &uv0 = tri[i0]->uv;
      const vec2 &uv1 = tri[i1]->uv;
      const vec2 &uv2 = tri[i2]->uv;
      cornerT.u = fixed12p20::fromFloat(uv0.x * texW);
      cornerL.u = fixed12p20::fromFloat(uv1.x * texW);
      cornerR.u = fixed12p20::fromFloat((uv0.x + (uv2.x - uv0.x) * yT) * texW);
      cornerB.u = fixed12p20::fromFloat(uv2.x * texW);
      cornerT.v = fixed12p20::fromFloat(uv0.y * texH);
      cornerL.v = fixed12p20::fromFloat(uv1.y * texH);
      cornerR.v = fixed12p20::fromFloat((uv0.y + (uv2.y - uv0.y) * yT) * texH);
      cornerB.v = fixed12p20::fromFloat(uv2.y * texH);
    }

    if (cornerL.x > cornerR.x) {
      std::swap(cornerL, cornerR);
    }

    TrapezoidFlags trf = TrapezoidFlags::NONE;
    if (target->format == PixelFormat::RGB444) {
      trf |= TrapezoidFlags::OUTPUT_444;
    }
    if (hasFlag(flags, RenderFlags3D::GOURAUD_SHADING)) {
      trf |= TrapezoidFlags::GOURAUD_SHADING;
    }
    if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
      if (mat->colorTexture->format == PixelFormat::RGB565) {
        trf |= TrapezoidFlags::COLOR_TEXTURE;
      } else if (mat->colorTexture->format == PixelFormat::ARGB4444) {
        trf |= TrapezoidFlags::COLOR_TEXTURE | TrapezoidFlags::TEXTURE_4444;
      }
    }
    if (blendMode != BlendMode::OVERWRITE) {
      trf |= TrapezoidFlags::BLEND;
    }

    // rasterize the triangle using a scanline algorithm
    // todo: optimize
    for (int ic = 0; ic < 2; ic++) {
      ScanLineCounter accumVL, accumVR;
      ScanLineCounter stepVL, stepVR;
      int yStart, yEnd;

      if (ic == 0) {
        accumVL = cornerT;
        accumVR = cornerT;
        stepVL = ScanLineCounter::subDiv(cornerT, cornerL, vSpanT);
        stepVR = ScanLineCounter::subDiv(cornerT, cornerR, vSpanT);
        int yOffset = iyMin > iy0 ? iyMin - iy0 : 0;
        accumVL.step(stepVL, yOffset);
        accumVR.step(stepVR, yOffset);
        yStart = iy0 + yOffset;
        yEnd = (int)fminf(iy1, iyMax);
      } else {
        accumVL = cornerL;
        accumVR = cornerR;
        stepVL = ScanLineCounter::subDiv(cornerL, cornerB, vSpanB);
        stepVR = ScanLineCounter::subDiv(cornerR, cornerB, vSpanB);
        int yOffset = iyMin > iy1 ? iyMin - iy1 : 0;
        accumVL.step(stepVL, yOffset);
        accumVR.step(stepVR, yOffset);
        yStart = iy1 + yOffset;
        yEnd = (int)fminf(iy2, iyMax);
      }

#define XMC_TRAPEZOID(aBlend, tex4444, colorTex, gouraud, out444) \
  renderTrapezoid<aBlend, tex4444, colorTex, gouraud, out444>(    \
      accumVL, accumVR, stepVL, stepVR, yStart, yEnd, tex)

      switch ((uint32_t)trf) {
        case 0: XMC_TRAPEZOID(false, false, false, false, false); break;
        case 1: XMC_TRAPEZOID(false, false, false, false, true); break;
        case 2: XMC_TRAPEZOID(false, false, false, true, false); break;
        case 3: XMC_TRAPEZOID(false, false, false, true, true); break;
        case 4: XMC_TRAPEZOID(false, false, true, false, false); break;
        case 5: XMC_TRAPEZOID(false, false, true, false, true); break;
        case 6: XMC_TRAPEZOID(false, false, true, true, false); break;
        case 7: XMC_TRAPEZOID(false, false, true, true, true); break;
        case 8: XMC_TRAPEZOID(false, false, false, false, false); break;
        case 9: XMC_TRAPEZOID(false, false, false, false, true); break;
        case 10: XMC_TRAPEZOID(false, false, false, true, false); break;
        case 11: XMC_TRAPEZOID(false, false, false, true, true); break;
        case 12: XMC_TRAPEZOID(false, true, true, false, false); break;
        case 13: XMC_TRAPEZOID(false, true, true, false, true); break;
        case 14: XMC_TRAPEZOID(false, true, true, true, false); break;
        case 15: XMC_TRAPEZOID(false, true, true, true, true); break;
        case 16: XMC_TRAPEZOID(true, false, false, true, false); break;
        case 17: XMC_TRAPEZOID(true, false, false, true, true); break;
        case 18: XMC_TRAPEZOID(true, false, false, true, false); break;
        case 19: XMC_TRAPEZOID(true, false, false, true, true); break;
        case 20: XMC_TRAPEZOID(true, false, true, true, false); break;
        case 21: XMC_TRAPEZOID(true, false, true, true, true); break;
        case 22: XMC_TRAPEZOID(true, false, true, true, false); break;
        case 23: XMC_TRAPEZOID(true, false, true, true, true); break;
        case 24: XMC_TRAPEZOID(true, false, false, true, false); break;
        case 25: XMC_TRAPEZOID(true, false, false, true, true); break;
        case 26: XMC_TRAPEZOID(true, false, false, true, false); break;
        case 27: XMC_TRAPEZOID(true, false, false, true, true); break;
        case 28: XMC_TRAPEZOID(true, true, true, true, false); break;
        case 29: XMC_TRAPEZOID(true, true, true, true, true); break;
        case 30: XMC_TRAPEZOID(true, true, true, true, false); break;
        case 31: XMC_TRAPEZOID(true, true, true, true, true); break;
        default: break;
      }

#undef XMC_TRAPEZOID
    }
  }

  void renderPoint(const Vertex3D &v0, const Material3D &mat) {
    RenderFlags3D flags = renderFlags;
    if (!mat || !mat->colorTexture ||
        !hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
      flags &= ~RenderFlags3D::COLOR_TEXTURE;
    }

    if (v0.pos.x < viewport.x || v0.pos.x >= viewport.right()) return;

    float x0 = v0.pos.x, y0 = v0.pos.y;

    float z0 = (float)MAX_DEPTH * (v0.pos.z - zNear) / (zFar - zNear);
    if (z0 < 0 || z0 > MAX_DEPTH) {
      return;
    }

    int iy0 = (int)floorf(y0);
    if (iy0 < viewport.y || iy0 >= viewport.bottom()) return;

    ScanLineCounter cornerL, cornerR;

    cornerL.x = fixed16p16::fromFloat(x0);
    cornerR.x = fixed16p16::fromFloat(x0 + 1);
    cornerL.z = fixed20p12::fromFloat(z0);
    cornerR.z = fixed20p12::fromFloat(z0);

    if (hasFlag(flags, RenderFlags3D::GOURAUD_SHADING)) {
      cornerL.c = color8p24(v0.color);
      cornerR.c = cornerL.c;
    } else {
      cornerL.c =
          color8p24(fixed8p24::fromFloat(1.0f), fixed8p24::fromFloat(1.0f),
                    fixed8p24::fromFloat(1.0f), fixed8p24::fromFloat(1.0f));
      cornerR.c = cornerL.c;
    }

    TextureArgs tex;
    if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
      const auto &t = mat->colorTexture;
      tex.data = (const uint16_t *)t->linePtr(0);
      tex.stride = t->stride / sizeof(uint16_t);
      int texW = 1 << (int)floorf(log2f(t->width));
      int texH = 1 << (int)floorf(log2f(t->height));
      tex.uMask = texW - 1;
      tex.vMask = texH - 1;
      const vec2 &uv0 = v0.uv;
      cornerL.u = fixed12p20::fromFloat(uv0.x * texW);
      cornerR.u = fixed12p20::fromFloat(uv0.x * texW);
      cornerL.v = fixed12p20::fromFloat(uv0.y * texH);
      cornerR.v = fixed12p20::fromFloat(uv0.y * texH);
    }

    TrapezoidFlags trf = TrapezoidFlags::NONE;
    if (target->format == PixelFormat::RGB444) {
      trf |= TrapezoidFlags::OUTPUT_444;
    }
    if (hasFlag(flags, RenderFlags3D::GOURAUD_SHADING)) {
      trf |= TrapezoidFlags::GOURAUD_SHADING;
    }
    if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
      if (mat->colorTexture->format == PixelFormat::RGB565) {
        trf |= TrapezoidFlags::COLOR_TEXTURE;
      } else if (mat->colorTexture->format == PixelFormat::ARGB4444) {
        trf |= TrapezoidFlags::COLOR_TEXTURE | TrapezoidFlags::TEXTURE_4444;
      }
    }
    if (blendMode != BlendMode::OVERWRITE) {
      trf |= TrapezoidFlags::BLEND;
    }

    ScanLineCounter dummyStep;

#define XMC_TRAPEZOID(aBlend, tex4444, colorTex, gouraud, out444) \
  renderTrapezoid<aBlend, tex4444, colorTex, gouraud, out444>(    \
      cornerL, cornerR, dummyStep, dummyStep, iy0, iy0 + 1, tex)

    switch ((uint32_t)trf) {
      case 0: XMC_TRAPEZOID(false, false, false, false, false); break;
      case 1: XMC_TRAPEZOID(false, false, false, false, true); break;
      case 2: XMC_TRAPEZOID(false, false, false, true, false); break;
      case 3: XMC_TRAPEZOID(false, false, false, true, true); break;
      case 4: XMC_TRAPEZOID(false, false, true, false, false); break;
      case 5: XMC_TRAPEZOID(false, false, true, false, true); break;
      case 6: XMC_TRAPEZOID(false, false, true, true, false); break;
      case 7: XMC_TRAPEZOID(false, false, true, true, true); break;
      case 8: XMC_TRAPEZOID(false, false, false, false, false); break;
      case 9: XMC_TRAPEZOID(false, false, false, false, true); break;
      case 10: XMC_TRAPEZOID(false, false, false, true, false); break;
      case 11: XMC_TRAPEZOID(false, false, false, true, true); break;
      case 12: XMC_TRAPEZOID(false, true, true, false, false); break;
      case 13: XMC_TRAPEZOID(false, true, true, false, true); break;
      case 14: XMC_TRAPEZOID(false, true, true, true, false); break;
      case 15: XMC_TRAPEZOID(false, true, true, true, true); break;
      case 16: XMC_TRAPEZOID(true, false, false, true, false); break;
      case 17: XMC_TRAPEZOID(true, false, false, true, true); break;
      case 18: XMC_TRAPEZOID(true, false, false, true, false); break;
      case 19: XMC_TRAPEZOID(true, false, false, true, true); break;
      case 20: XMC_TRAPEZOID(true, false, true, true, false); break;
      case 21: XMC_TRAPEZOID(true, false, true, true, true); break;
      case 22: XMC_TRAPEZOID(true, false, true, true, false); break;
      case 23: XMC_TRAPEZOID(true, false, true, true, true); break;
      case 24: XMC_TRAPEZOID(true, false, false, true, false); break;
      case 25: XMC_TRAPEZOID(true, false, false, true, true); break;
      case 26: XMC_TRAPEZOID(true, false, false, true, false); break;
      case 27: XMC_TRAPEZOID(true, false, false, true, true); break;
      case 28: XMC_TRAPEZOID(true, true, true, true, false); break;
      case 29: XMC_TRAPEZOID(true, true, true, true, true); break;
      case 30: XMC_TRAPEZOID(true, true, true, true, false); break;
      case 31: XMC_TRAPEZOID(true, true, true, true, true); break;
      default: break;
    }

#undef XMC_TRAPEZOID
  }

 private:
  XMC_INLINE mat4 &dirtyModelMatrix() {
    modelMatrixStack[modelMatrixStackPtr].dirty = true;
    mvpDirty = true;
    return modelMatrixStack[modelMatrixStackPtr].local;
  }

  void validateMatrix(bool force = false);
};  // namespace xmc

}  // namespace xmc

#endif
