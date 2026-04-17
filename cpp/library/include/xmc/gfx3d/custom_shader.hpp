#ifndef XMC_GFX_CUSTOM_SHADER_HPP
#define XMC_GFX_CUSTOM_SHADER_HPP

#include "xmc/gfx3d/gfx3d_common.hpp"
#include "xmc/gfx3d/material3d.hpp"
#include "xmc/gfx3d/primitive3d.hpp"

namespace xmc {

struct VertexShaderArgs {
  RenderFlags3D flags;
  const mat4 &modelMatrix;
  const mat4 &viewProjectionMatrix;
  const Primitive3D &primitive;
  const Material3D &material;

  VertexShaderArgs(RenderFlags3D flags, const mat4 &model, const mat4 &vp,
                   const Primitive3D &prim, const Material3D &mat)
      : flags(flags),
        modelMatrix(model),
        viewProjectionMatrix(vp),
        primitive(prim),
        material(mat) {}
};

class VertexShader {
 public:
  virtual void beginPrimitive(VertexShaderArgs args) {}
  virtual void process(VertexCache3D *vert) = 0;
  virtual void endPrimitive() {}
};

class EnvironmentMapShader : public VertexShader {
 private:
  const mat4 *modelMatrix;
  vec3 eye;

 public:
  void beginPrimitive(VertexShaderArgs args) override {
    modelMatrix = &args.modelMatrix;
  }
  inline void setEyePosition(const vec3 &eye) { this->eye = eye; }

  void process(VertexCache3D *vert) override;
};

}  // namespace xmc
#endif
