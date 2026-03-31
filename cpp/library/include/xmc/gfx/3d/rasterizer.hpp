#ifndef XMC_GFX_RASTERIZER_HPP
#define XMC_GFX_RASTERIZER_HPP

#include "xmc/gfx/3d/scene3d.hpp"
#include "xmc/gfx/color4p12.hpp"

namespace xmc {

using depth_t = uint16_t;
static constexpr depth_t MAX_DEPTH = (1 << (sizeof(depth_t) * 8)) - 1;

static void getUvMask(int size, uint32_t *mask, uint32_t *shift);

struct BakedVertex {
  vec3 pos;
  colorf color;
  vec2 uv;
};

class RasterizerClass;
using Rasterizer = std::shared_ptr<RasterizerClass>;

static inline Rasterizer createRasterizer(int width, int height,
                                          uint32_t stackSize = 16) {
  return std::make_shared<RasterizerClass>(width, height, stackSize);
}

struct MatrixStackEntry {
  mat4 local = mat4::identity();
  mat4 world = mat4::identity();
  bool dirty = true;
};

template <int PREC = 16>
struct TriangleInterp {
  int32_t y0, y1;
  int32_t x0, x1;
  int32_t vStepA, vStepB0, vStepB1;
  int32_t xp;
  int32_t hStep;
  bool hReverse = false;
  TriangleInterp(int32_t y0, int32_t y1, int32_t y2, float x0, float x1,
                 float x2, bool hReverse = false)
      : y0(y0),
        y1(y1),
        x0(x0 * (1 << PREC)),
        x1(x1 * (1 << PREC)),
        vStepA(idiv((x2 - x0) * (1 << PREC), (y2 - y0))),
        vStepB0(idiv((x1 - x0) * (1 << PREC), (y1 - y0))),
        vStepB1(idiv((x2 - x1) * (1 << PREC), (y2 - y1))),
        hReverse(hReverse) {}
  XMC_INLINE void yStep(int32_t y, int32_t xOffset, int32_t xSpan) {
    int32_t xa = x0 + vStepA * (y - y0);
    int32_t xb;
    if (y < y1) {
      xb = x0 + vStepB0 * (y - y0);
    } else {
      xb = x1 + vStepB1 * (y - y1);
    }
    if (hReverse) {
      xp = xb;
      hStep = idiv((xa - xb), xSpan);
    } else {
      xp = xa;
      hStep = idiv((xb - xa), xSpan);
    }
    xp += hStep * xOffset;
  }
  XMC_INLINE int32_t xPeek() const { return xp >> PREC; }
  XMC_INLINE int32_t xStep() {
    int32_t x = xp;
    xp += hStep;
    return x >> PREC;
  }
  static XMC_INLINE int32_t idiv(int32_t a, int32_t b) {
    return (b == 0) ? a : (a / b);
  }
};

class RasterizerClass {
 private:
  const int width;
  const int height;
  const int stackSize;

  depth_t *depthBuff;

  Sprite target;
  rect_t viewport;

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
    depthBuff =
        (depth_t *)xmcMalloc(sizeof(depth_t) * width * height, XMC_RAM_CAP_DMA);
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

  void setTarget(Sprite &target, rect_t viewport);

  inline void setTarget(Sprite &target) {
    setTarget(target, rect_t{0, 0, target->width, target->height});
  }

  inline void setEnvironmentLight(const colorf &color) { envLight = color; }

  inline void setParallelLight(const vec3 &dir, const colorf &color) {
    parallelLightDir = dir.normalized();
    parallelLightColor = color;
  }

  inline void setProjection(const mat4 &proj) {
    projectionMatrix = proj;
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
    float y0to1inv = (y1 - y0) > 1e-8f ? 1.0f / (y1 - y0) : 0.0f;
    float y1to2inv = (y2 - y1) > 1e-8f ? 1.0f / (y2 - y1) : 0.0f;
    float y0to2inv = (y2 - y0) > 1e-8f ? 1.0f / (y2 - y0) : 0.0f;

    float z0 = (float)MAX_DEPTH * (tri[i0]->pos.z - zNear) / (zFar - zNear);
    float z1 = (float)MAX_DEPTH * (tri[i1]->pos.z - zNear) / (zFar - zNear);
    float z2 = (float)MAX_DEPTH * (tri[i2]->pos.z - zNear) / (zFar - zNear);

    if (z0 < 0 || z0 > MAX_DEPTH || z1 < 0 || z1 > MAX_DEPTH || z2 < 0 ||
        z2 > MAX_DEPTH) {
      return;
    }

    vec2 uv0 = tri[i0]->uv;
    vec2 uv1 = tri[i1]->uv;
    vec2 uv2 = tri[i2]->uv;

    int iy0 = (int)ceilf(y0);
    int iy1 = (int)roundf(y1);
    int iy2 = (int)floorf(y2);
    if (iy0 < viewport.y) iy0 = viewport.y;
    if (iy2 >= viewport.bottom()) iy2 = viewport.bottom() - 1;

    const uint16_t *texData = nullptr;
    uint32_t texStride = 0;
    uint32_t texW = 0, texH = 0;
    uint32_t uMask = 0, vMask = 0;
    if (mat && mat->colorTexture) {
      const auto &tex = mat->colorTexture;
      texData = (const uint16_t *)tex->linePtr(0);
      texStride = tex->stride / sizeof(uint16_t);
      texW = 1 << (int)floorf(log2f(tex->width));
      texH = 1 << (int)floorf(log2f(tex->height));
      uMask = texW - 1;
      vMask = texH - 1;
    }

    int xMid = (int)(x0 + (x2 - x0) * (y1 - y0) * y0to2inv);
    bool hReverse = xMid > x1;

    TriangleInterp<12> zInterp(iy0, iy1, iy2, z0, z1, z2, hReverse);
    TriangleInterp<20> uInterp(iy0, iy1, iy2, uv0.x * texW, uv1.x * texW,
                               uv2.x * texW, hReverse);
    TriangleInterp<20> vInterp(iy0, iy1, iy2, uv0.y * texH, uv1.y * texH,
                               uv2.y * texH, hReverse);

    color4p12 c0a(tri[i0]->color);
    color4p12 c1a(tri[i1]->color);
    color4p12 c2a(tri[i2]->color);
    color4p12 cStepA = (c2a - c0a) / (int32_t)fmaxf(iy2 - iy0, 1);
    color4p12 cStepB0 = (c1a - c0a) / (int32_t)fmaxf(iy1 - iy0, 1);
    color4p12 cStepB1 = (c2a - c1a) / (int32_t)fmaxf(iy2 - iy1, 1);

    // rasterize the triangle using a scanline algorithm
    // todo: optimize
    for (int iy = iy0; iy <= iy2; iy++) {
      bool firstHalf = iy < iy1;

      float y = (float)iy;

      float xa = x0 + (x2 - x0) * (y - y0) * y0to2inv;

      float xb;
      if (y < y1) {
        xb = x0 + (x1 - x0) * (y - y0) * y0to1inv;
      } else {
        xb = x1 + (x2 - x1) * (y - y1) * y1to2inv;
      }

      if (hReverse) {
        std::swap(xa, xb);
      }

      int32_t ixMin = (int)ceilf(xa);
      int32_t ixMax = (int)floorf(xb);
      if (ixMin < viewport.x) {
        ixMin = viewport.x;
      }
      if (ixMax >= viewport.right()) {
        ixMax = viewport.right() - 1;
      }

      int32_t xSpan = (int)(xb - xa);
      if (xSpan < 1) xSpan = 1;
      int xOffset = ixMin - (int)floorf(xa);
      zInterp.yStep(iy, xOffset, xSpan);
      uInterp.yStep(iy, xOffset, xSpan);
      vInterp.yStep(iy, xOffset, xSpan);

      color4p12 ca = c0a + cStepA * (iy - iy0);
      color4p12 cb;
      if (firstHalf) {
        cb = c0a + cStepB0 * (iy - iy0);
      } else {
        cb = c1a + cStepB1 * (iy - iy1);
      }

      color4p12 cp;
      color4p12 cStep;
      if (hReverse) {
        cp = cb;
        cStep = (ca - cb) / xSpan;
      } else {
        cp = ca;
        cStep = (cb - ca) / xSpan;
      }

      if (target->format == pixel_format_t::RGB565) {
        uint16_t *cptr = (uint16_t *)target->linePtr(iy) + ixMin;
        depth_t *zptr = depthBuff + iy * width + ixMin;
        for (int x = ixMin; x <= ixMax; x++) {
          int32_t z = zInterp.xStep();
          if (z < *zptr) {
            color4p12 c = cp;
            if (useTexture) {
              int32_t u = uInterp.xPeek();
              int32_t v = vInterp.xPeek();
              c *= color4444(texData[(v & vMask) * texStride + (u & uMask)]);
            }
            if (c.a.raw != 0) {
              *cptr = c.to565();
              *zptr = z;
            }
          }
          zptr++;
          cptr++;
          uInterp.xStep();
          vInterp.xStep();
        }
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
