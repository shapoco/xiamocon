#ifndef XMC_GFX_RASTERIZER_HPP
#define XMC_GFX_RASTERIZER_HPP

#include "xmc/gfx3d/scene3d.hpp"
#include "xmc/gfx2d/color4p12.hpp"
#include "xmc/gfx2d/sprite444.hpp"

namespace xmc {

using depth_t = uint16_t;
static constexpr depth_t MAX_DEPTH = (1 << (sizeof(depth_t) * 8)) - 1;

struct BakedVertex {
  vec3 pos;
  colorf color;
  vec2 uv;
};

struct ScanLineCounter {
  fixed16p16 x;
  fixed20p12 z;
  color4p12 c;
  fixed12p20 u;
  fixed12p20 v;

  static XMC_INLINE ScanLineCounter calcStep(const ScanLineCounter &start,
                                             const ScanLineCounter &end,
                                             int32_t span) {
    if (span <= 0) span = 1;
    return ScanLineCounter{
        (end.x - start.x) / span, (end.z - start.z) / span,
        (end.c - start.c) / span, (end.u - start.u) / span,
        (end.v - start.v) / span,
    };
  }

  XMC_INLINE void step(const ScanLineCounter &step) {
    x += step.x;
    z += step.z;
    c += step.c;
    u += step.u;
    v += step.v;
  }
};

class RasterizerClass;
using Graphics3D = std::shared_ptr<RasterizerClass>;

static inline Graphics3D createRasterizer(int width, int height,
                                          uint32_t stackSize = 16) {
  return std::make_shared<RasterizerClass>(width, height, stackSize);
}

struct MatrixStackEntry {
  mat4 local = mat4::identity();
  mat4 world = mat4::identity();
  bool dirty = true;
};

class RasterizerClass {
 private:
  const int width;
  const int height;
  const int stackSize;

  depth_t *depthBuff;

  Sprite target;
  Rect viewport;

  mat4 screenMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  mat4 vpMatrix;
  bool vpDirty = true;
  mat4 mvpMatrix;
  bool mvpDirty = true;

  MatrixStackEntry *modelMatrixStack;
  int modelMatrixStackPtr = 0;

  float zNear = 0.01f;
  float zFar = 100.0f;

  colorf envLight = {0.1f, 0.1f, 0.1f, 1.0f};

  vec3 parallelLightDir = vec3(0.5f, 0.5f, 1.0f).normalized();
  colorf parallelLightColor = {1, 1, 1, 1};

  Material3D material;

 public:
  RasterizerClass(int w, int h, uint32_t stackSize)
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

  ~RasterizerClass() {
    if (depthBuff) {
      xmcFree(depthBuff);
      depthBuff = nullptr;
    }
    if (modelMatrixStack) {
      xmcFree(modelMatrixStack);
      modelMatrixStack = nullptr;
    }
  }

  void setTarget(Sprite &target, Rect viewport);

  inline void setTarget(Sprite &target) {
    setTarget(target, Rect{0, 0, target->width, target->height});
  }

  inline void setEnvironmentLight(const colorf &color) { envLight = color; }

  inline void setParallelLight(const vec3 &dir, const colorf &color) {
    parallelLightDir = dir.normalized();
    parallelLightColor = color;
  }

  inline void setScreenMatrix(const mat4 &mat) {
    screenMatrix = mat;
    vpDirty = true;
    mvpDirty = true;
  }

  inline void setProjection(const mat4 &mat) {
    projectionMatrix = mat;
    vpDirty = true;
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
    vpDirty = true;
    mvpDirty = true;
  }

  inline const mat4 &getViewMatrix() const { return viewMatrix; }

  inline void lookAt(const vec3 &eye, const vec3 &focus, const vec3 &up) {
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

  inline void rotate(float pitch, float roll, float yaw) {
    dirtyModelMatrix().rotate(pitch, roll, yaw);
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

  inline void setMaterial(const Material3D &mat) { material = mat; }

  void clearDepth(depth_t value = MAX_DEPTH);

  void renderScene(const Scene3D &scene);

  void renderNode(const Node3D &node);

  void renderMesh(const Mesh3D &mesh);

  inline void renderPrimitive(const Primitive3D &prim) {
    renderPrimitive(prim, material);
  }

  void renderPrimitive(const Primitive3D &prim, const Material3D &mat);

  template <bool useTexture>
  void renderTriangleParam(const BakedVertex &v0, const BakedVertex &v1,
                           const BakedVertex &v2, const Material3D &mat) {
    int vpl = viewport.x;
    int vpr = viewport.right() - 1;
    if (v0.pos.x < vpl && v1.pos.x < vpl && v2.pos.x < vpl) return;
    if (v0.pos.x > vpr && v1.pos.x > vpr && v2.pos.x > vpr) return;

    if (!mat || !mat->doubleSided) {
      // back-face culling
      float ax = v1.pos.x - v0.pos.x;
      float ay = v1.pos.y - v0.pos.y;
      float bx = v2.pos.x - v0.pos.x;
      float by = v2.pos.y - v0.pos.y;
      if (ax * by - ay * bx >= 0) return;
    }

    const BakedVertex *tri[] = {&v0, &v1, &v2};

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
    if (z0 < 0 || z0 > MAX_DEPTH || z1 < 0 || z1 > MAX_DEPTH || z2 < 0 ||
        z2 > MAX_DEPTH) {
      return;
    }

    const colorf &c0 = tri[i0]->color;
    const colorf &c1 = tri[i1]->color;
    const colorf &c2 = tri[i2]->color;

    const vec2 &uv0 = tri[i0]->uv;
    const vec2 &uv1 = tri[i1]->uv;
    const vec2 &uv2 = tri[i2]->uv;

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

    const uint16_t *__restrict__ texData = nullptr;
    uint32_t texStride = 0;
    uint32_t texW = 0, texH = 0;
    uint32_t uMask = 0;
    uint32_t vMask = 0;
    if (mat && mat->colorTexture) {
      const auto &tex = mat->colorTexture;
      texData = (const uint16_t *)tex->linePtr(0);
      texStride = tex->stride / sizeof(uint16_t);
      texW = 1 << (int)floorf(log2f(tex->width));
      texH = 1 << (int)floorf(log2f(tex->height));
      uMask = texW - 1;
      vMask = texH - 1;
    }

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

    cornerT.u = fixed12p20::fromFloat(uv0.x * texW);
    cornerL.u = fixed12p20::fromFloat(uv1.x * texW);
    cornerR.u = fixed12p20::fromFloat((uv0.x + (uv2.x - uv0.x) * yT) * texW);
    cornerB.u = fixed12p20::fromFloat(uv2.x * texW);

    cornerT.v = fixed12p20::fromFloat(uv0.y * texH);
    cornerL.v = fixed12p20::fromFloat(uv1.y * texH);
    cornerR.v = fixed12p20::fromFloat((uv0.y + (uv2.y - uv0.y) * yT) * texH);
    cornerB.v = fixed12p20::fromFloat(uv2.y * texH);

    cornerT.c = color4p12(c0);
    cornerL.c = color4p12(c1);
    cornerR.c = color4p12(c0 + (c2 - c0) * yT);
    cornerB.c = color4p12(c2);

    if (cornerL.x > cornerR.x) {
      std::swap(cornerL, cornerR);
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
        stepVL = ScanLineCounter::calcStep(cornerT, cornerL, vSpanT);
        stepVR = ScanLineCounter::calcStep(cornerT, cornerR, vSpanT);
        yStart = iyMin;
        yEnd = (int)fminf(iy1, iyMax);
      } else {
        accumVL = cornerL;
        accumVR = cornerR;
        stepVL = ScanLineCounter::calcStep(cornerL, cornerB, vSpanB);
        stepVR = ScanLineCounter::calcStep(cornerR, cornerB, vSpanB);
        yStart = (int)fmaxf(iy1, iyMin);
        yEnd = iyMax;
      }

      for (int iy = yStart; iy <= yEnd; iy++) {
        int32_t ixMin = accumVL.x.roundToInt();
        int32_t ixMax = accumVR.x.roundToInt();
        int32_t hSpan = ixMax - ixMin + 1;

        if (ixMin < viewport.x) ixMin = viewport.x;
        if (ixMax >= viewport.right()) ixMax = viewport.right() - 1;

        if (ixMin <= ixMax) {
          ScanLineCounter accumH = accumVL;
          ScanLineCounter stepH =
              ScanLineCounter::calcStep(accumVL, accumVR, hSpan);

          int32_t xOffset = ixMin - accumVL.x.roundToInt();
          accumH.x += stepH.x * xOffset;
          accumH.z += stepH.z * xOffset;
          accumH.c += stepH.c * xOffset;
          accumH.u += stepH.u * xOffset;
          accumH.v += stepH.v * xOffset;

          if (target->format == PixelFormat::RGB565) {
            uint16_t *__restrict__ cptr = (uint16_t *)target->linePtr(iy);
            depth_t *__restrict__ zptr = depthBuff + iy * width;
            for (int x = ixMin; x <= ixMax; x++) {
              int32_t z = accumH.z.floorToInt();
              if (z < zptr[x]) {
                color4p12 c = accumH.c;
                if (useTexture) {
                  int32_t u = accumH.u.floorToInt() & uMask;
                  int32_t v = accumH.v.floorToInt() & vMask;
                  c *= color4444(texData[v * texStride + u]);
                }
                if (c.a.raw != 0) {
                  cptr[x] = c.to565();
                  zptr[x] = z;
                }
              }
              accumH.step(stepH);
            }
          } else if (target->format == PixelFormat::RGB444) {
            Scanner444 scanner(*target, ixMin, iy);
            depth_t *__restrict__ zptr = depthBuff + iy * width;
            for (int x = ixMin; x <= ixMax; x++) {
              int32_t z = accumH.z.floorToInt();
              bool written = false;
              if (z < zptr[x]) {
                color4p12 c = accumH.c;
                if (useTexture) {
                  int32_t u = accumH.u.floorToInt() & uMask;
                  int32_t v = accumH.v.floorToInt() & vMask;
                  c *= color4444(texData[v * texStride + u]);
                }
                if (c.a.raw != 0) {
                  scanner.push444(c.to444());
                  written = true;
                  zptr[x] = z;
                }
              }
              accumH.step(stepH);
              if (!written) scanner.skip();
            }
          }
        }

        accumVL.step(stepVL);
        accumVR.step(stepVR);
      }
    }
  }

  void renderTriangle(const BakedVertex &v0, const BakedVertex &v1,
                      const BakedVertex &v2, const Material3D &mat) {
    if (mat && mat->colorTexture) {
      renderTriangleParam<true>(v0, v1, v2, mat);
    } else {
      renderTriangleParam<false>(v0, v1, v2, mat);
    }
  }

 private:
  XMC_INLINE mat4 &dirtyModelMatrix() {
    modelMatrixStack[modelMatrixStackPtr].dirty = true;
    mvpDirty = true;
    return modelMatrixStack[modelMatrixStackPtr].local;
  }

  void validateMatrix(bool force = false);
};

}  // namespace xmc

#endif
