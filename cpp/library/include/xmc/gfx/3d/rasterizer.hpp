#ifndef XMC_GFX_RASTERIZER_HPP
#define XMC_GFX_RASTERIZER_HPP

#include "xmc/gfx/3d/scene3d.hpp"

namespace xmc {

using depth_t = uint8_t;

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

class RasterizerClass {
 private:
  const int width;
  const int height;
  const int stackSize;

  depth_t *depthBuff;

  Sprite target;
  rect_t viewport;

  mat4 projectionMatrix;
  mat4 viewPortMatrix;

  mat4 *matrixStack;
  int matrixStackTop = 0;
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
    matrixStack = (mat4 *)xmcMalloc(sizeof(mat4) * stackSize, XMC_RAM_CAP_DMA);
    loadIdentity();
    projectionMatrix = mat4::identity();
    viewPortMatrix = mat4::identity();
  };

  ~RasterizerClass() {
    if (depthBuff) {
      xmcFree(depthBuff);
      depthBuff = nullptr;
    }
    if (matrixStack) {
      xmcFree(matrixStack);
      matrixStack = nullptr;
    }
  }

  void setTarget(Sprite &target, rect_t viewport);

  inline void setTarget(Sprite &target) {
    setTarget(target, rect_t{0, 0, target->width, target->height});
  }

  inline void setParallelLight(const vec3 &dir, const colorf &color) {
    parallelLightDir = dir.normalized();
    parallelLightColor = color;
  }

  inline void setProjection(const mat4 &proj) { projectionMatrix = proj; }
  inline const mat4 &getProjection() const { return projectionMatrix; }

  void setProjectionOrtho(float left, float right, float bottom, float top,
                          float near = -1.0f, float far = 1.0f);
  void setProjectionPerspective(float fovY, float aspect, float near = 0.01f,
                                float far = 100.0f);

  inline void loadMatrix(const mat4 &proj) {
    matrixStack[matrixStackTop] = proj;
  }

  inline void loadIdentity() { matrixStack[matrixStackTop] = mat4::identity(); }

  inline void pushMatrix() {
    if (matrixStackTop < stackSize - 1) {
      matrixStackTop++;
      matrixStack[matrixStackTop] = matrixStack[matrixStackTop - 1];
    }
  }

  inline void popMatrix() {
    if (matrixStackTop > 0) {
      matrixStackTop--;
    }
  }

  inline mat4 &currentMatrix() { return matrixStack[matrixStackTop]; }
  inline void translate(const vec3 &t) { currentMatrix().translate(t); }
  inline void translate(float x, float y, float z) {
    currentMatrix().translate(x, y, z);
  }
  inline void rotate(const quat &q) { currentMatrix().rotate(q); }
  inline void rotate(float pitch, float roll, float yaw) {
    currentMatrix().rotate(pitch, roll, yaw);
  }
  inline void rotate(const vec3 &axis, float angle) {
    currentMatrix().rotate(axis, angle);
  }
  inline void scale(const vec3 &s) { currentMatrix().scale(s); }
  inline void scale(float s) { currentMatrix().scale(s); }

  inline void setDepthRange(float near, float far) {
    zNear = near;
    zFar = far;
  }

  inline void setMaterial(const Material3D &mat) { material = mat; }

  void clearDepth(depth_t value = 0xFF);

  void renderMesh(const Mesh3D &mesh);

  inline void renderPrimitive(const Primitive3D &prim) {
    renderPrimitive(prim, material);
  }

  void renderPrimitive(const Primitive3D &prim, const Material3D &mat);

  void renderTriangle(const BakedVertex &v0, const BakedVertex &v1,
                      const BakedVertex &v2, const Material3D &mat);
};

}  // namespace xmc

#endif
