#include "xmc/gfx3d/custom_shader.hpp"

namespace xmc {

void EnvironmentMapShader::process(VertexCache3D *vert) {
  vert->pos = modelMatrix->transform(vert->pos);
  vert->normal = modelMatrix->transformNormal(vert->normal);
  vec3 viewDir = (eye - vert->pos).normalized();
  vec3 reflectDir =
      (vert->normal * 2 * vert->normal.dot(viewDir) - viewDir).normalized();
  float u = 0.5f + atan2f(reflectDir.z, reflectDir.x) / (2 * M_PI);
  float v = 0.5f - asinf(reflectDir.y) / M_PI;
  vert->uv = vec2(u, v);
}

}  // namespace xmc
