#ifndef XMC_GFX_RASTERIZER_HPP
#define XMC_GFX_RASTERIZER_HPP

#include "xmc/gfx2d/color8p24.hpp"
#include "xmc/gfx2d/raster_scan.hpp"
#include "xmc/gfx2d/sprite444.hpp"
#include "xmc/gfx3d/gfx3d_common.hpp"
#include "xmc/gfx3d/scene3d.hpp"
#include "xmc/gfx3d/worker3d.hpp"

namespace xmc {

enum class ClearTarget : uint8_t {
  STACK = 1 << 0,
  DEPTH = 1 << 1,
  ALL = 0xFF,
};
XMC_ENUM_FLAGS(ClearTarget, uint8_t);

class Graphics3DClass;
using Graphics3D = std::shared_ptr<Graphics3DClass>;

static inline Graphics3D createGraphics3D(int width, int height,
                                          uint32_t stackSize = 16) {
  return std::make_shared<Graphics3DClass>(width, height, stackSize);
}

class Graphics3DClass {
 private:
  const int width;
  const int height;
  const int stackSize;

  Depth3D *depthBuff;

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

  State3D *stateStack;
  int stateStackTop = 0;

  WorkerArgs3D workerArgs;
  Vertex3D bakedVerts[3];
  EdgeScanVars esvL, esvR, esvB;
  Trapezoid3D trapU, trapL;

  MultiCoreMode3D multicoreMode = MultiCoreMode3D::NONE;
  Worker3D subWorker;

 public:
  Graphics3DClass(int w, int h, uint32_t stackSize)
      : width(w), height(h), stackSize(stackSize) {
    depthBuff = (Depth3D *)xmcMalloc(sizeof(Depth3D) * width * height,
                                     XMC_RAM_CAP_SPIRAM);
    stateStack =
        (State3D *)xmcMalloc(sizeof(State3D) * stackSize, XMC_RAM_CAP_DMA);
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
    if (stateStack) {
      xmcFree(stateStack);
      stateStack = nullptr;
    }
  }

  void setTarget(Sprite target, Rect viewport);
  inline void setTarget(Sprite target) {
    setTarget(target, Rect{0, 0, target->width, target->height});
  }

  inline void setMultiCoreMode(MultiCoreMode3D mode) { multicoreMode = mode; }
  inline MultiCoreMode3D getMultiCoreMode() const { return multicoreMode; }
  inline void serviceSubWorker() { subWorker.service(); }

  inline RenderFlags3D getFlags() const { return stackTop().flags; }
  inline void setFlags(RenderFlags3D flags) { stackTop().flags = flags; }
  inline void enableFlags(RenderFlags3D flags) { stackTop().flags |= flags; }
  inline void disableFlags(RenderFlags3D flags) { stackTop().flags &= ~flags; }

  inline BlendMode getBlendMode() const { return stackTop().blendMode; }
  inline void setBlendMode(BlendMode mode) { stackTop().blendMode = mode; }

  inline void setEnvironmentLight(const colorf &color) {
    stackTop().envLight = color;
  }

  inline void setParallelLight(const vec3 &dir, const colorf &color) {
    stackTop().parallelLightDir = dir.normalized();
    stackTop().parallelLightColor = color;
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

  inline void pushState() {
    if (stateStackTop >= stackSize - 1) {
      XMC_ERR_LOG(XMC_USER_GENERIC_ERROR);
      return;
    }
    stateStackTop++;
    stateStack[stateStackTop] = stateStack[stateStackTop - 1];
    stateStack[stateStackTop].local = mat4::identity();
    stateStack[stateStackTop].dirty = true;
    mvpDirty = true;
  }

  inline void popState() {
    if (stateStackTop <= 0) {
      XMC_ERR_LOG(XMC_USER_GENERIC_ERROR);
      return;
    }
    stateStackTop--;
    mvpDirty = true;
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
    stackTop().zNear = near;
    stackTop().zFar = far;
  }

  inline int32_t getZTestOffset() const { return stackTop().zTestOffset; }
  inline void setZTestOffset(int32_t offset) {
    stackTop().zTestOffset = offset;
  }

  void beginRender(ClearTarget target = ClearTarget::ALL);
  void endRender();

  void render(const Scene3D &scene);
  void render(const Node3D &node);
  void render(const Mesh3D &mesh);
  void render(const Primitive3D &prim);

 private:
  void renderTriangle(WorkerArgs3D &workerArgs, const Vertex3D &v0,
                      const Vertex3D &v1, const Vertex3D &v2,
                      const Material3D &mat);
  void renderPoint(WorkerArgs3D &workerArgs, const Vertex3D &v0,
                   const Material3D &mat);

  XMC_INLINE State3D &stackTop() { return stateStack[stateStackTop]; }
  XMC_INLINE const State3D &stackTop() const {
    return stateStack[stateStackTop];
  }

  XMC_INLINE mat4 &dirtyModelMatrix() {
    stackTop().dirty = true;
    mvpDirty = true;
    return stackTop().local;
  }

  void validateMatrix(bool force = false);
};  // namespace xmc

}  // namespace xmc

#endif
