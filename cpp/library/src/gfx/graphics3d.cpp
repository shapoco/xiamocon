#include "xmc/gfx3d/graphics3d.hpp"
#include "xmc/gfx3d/custom_shader.hpp"

#include <string.h>

namespace xmc {

void Graphics3DClass::clearDepth(depth_t value) {
  int pixelCount = width * height;
  if (value == 0) {
    memset(depthBuff, 0x00, sizeof(depth_t) * pixelCount);
  } else if (value == MAX_DEPTH) {
    memset(depthBuff, 0xFF, sizeof(depth_t) * pixelCount);
  } else {
    for (int i = 0; i < pixelCount; i++) {
      depthBuff[i] = value;
    }
  }
}

void Graphics3DClass::setTarget(Sprite target, Rect viewport) {
  this->target = target;
  this->viewport = viewport;
  screenMatrix = mat4::identity();
  screenMatrix.m[0] = viewport.width / 2.0f;
  screenMatrix.m[5] = -viewport.height / 2.0f;
  screenMatrix.m[12] = viewport.x + viewport.width / 2.0f;
  screenMatrix.m[13] = viewport.y + viewport.height / 2.0f;
  viewProjectionDirty = true;
  mvpDirty = true;
}

void Graphics3DClass::validateMatrix(bool force) {
  if (mvpDirty || force) {
    // Recalculate model matrix
    int dirtyIndex = modelMatrixStackPtr + 1;
    if (force) {
      dirtyIndex = 0;
    } else {
      while (dirtyIndex > 0 && !modelMatrixStack[dirtyIndex - 1].dirty) {
        dirtyIndex--;
      }
    }
    while (dirtyIndex <= modelMatrixStackPtr) {
      if (dirtyIndex == 0) {
        modelMatrixStack[dirtyIndex].world = modelMatrixStack[dirtyIndex].local;
      } else {
        modelMatrixStack[dirtyIndex].world =
            modelMatrixStack[dirtyIndex - 1].world *
            modelMatrixStack[dirtyIndex].local;
      }
      modelMatrixStack[dirtyIndex].dirty = false;
      dirtyIndex++;
    }

    // Recalculate view-projection matrix
    if (viewProjectionDirty || force) {
      viewProjectionMatrix = screenMatrix * projectionMatrix * viewMatrix;
      viewProjectionDirty = false;
    }

    // Recalculate MVP matrix
    mvpMatrix =
        viewProjectionMatrix * modelMatrixStack[modelMatrixStackPtr].world;
    mvpDirty = false;
  }
}

void Graphics3DClass::getModelMatrix(mat4 &out) {
  validateMatrix();
  out = modelMatrixStack[modelMatrixStackPtr].world;
}

void Graphics3DClass::getMvpMatrix(mat4 &out) {
  validateMatrix();
  out = mvpMatrix;
}

void Graphics3DClass::renderScene(const Scene3D &scene, void *userContext) {
  for (const Node3D &node : scene->rootNodes) {
    renderNode(node, userContext);
  }
}

void Graphics3DClass::renderNode(const Node3D &node, void *userContext) {
  pushMatrix();
  loadMatrix(node->transform);
  if (node->mesh) {
    renderMesh(node->mesh, userContext);
  }
  for (const Node3D &child : node->children) {
    renderNode(child, userContext);
  }
  popMatrix();
}

void Graphics3DClass::renderMesh(const Mesh3D &mesh, void *userContext) {
  for (const Primitive3D &prim : mesh->primitives) {
    renderPrimitive(prim, userContext);
  }
}

void Graphics3DClass::renderPrimitive(const Primitive3D &prim,
                                      void *userContext) {
  RenderFlags3D flags = renderFlags;
  const Vec3Buffer &primPos = prim->position;
  const Vec3Buffer &primNorm = prim->normal;
  const ColorBuffer &primCol = prim->color;
  const Vec2Buffer &primUv = prim->uv;
  const IndexBuffer &primIdx = prim->indexes;
  const Material3D &mat = prim->material;

  MaterialFlags3D matFlags = MaterialFlags3D::NONE;
  if (mat) {
    matFlags = mat->flags;
  }

  if (!primPos) return;

  int idxOffset[3];
  int idxData[3];

  validateMatrix(true);

  if (!primNorm) {
    flags &= ~RenderFlags3D::VERTEX_NORMAL;
    flags &= ~RenderFlags3D::GOURAUD_SHADING;
    flags &= ~RenderFlags3D::LIGHTING;
  }
  if (!primCol) {
    flags &= ~RenderFlags3D::VERTEX_COLOR;
  }
  if (!mat || !mat->vertexShader) {
    flags &= ~RenderFlags3D::CUSTOM_VERTEX_SHADER;
  }

  mat4 &modelMatrix = modelMatrixStack[modelMatrixStackPtr].world;
  if (hasFlag(flags, RenderFlags3D::CUSTOM_VERTEX_SHADER)) {
    VertexShaderArgs shaderArgs(flags, modelMatrix, viewProjectionMatrix, prim,
                                mat);
    mat->vertexShader->beginPrimitive(shaderArgs);
  }

  int numVertices = prim->numVertices();
  int numElems = prim->numElements();

  Vertex3D bakedVerts[3];

  for (int i = 0; i < numElems; i++) {
    bool reverse = false;
    uint32_t shiftFlags = 0;
    uint32_t fetchFlags = 0;
    switch (prim->mode) {
      case PrimitiveMode::POINTS:
        idxOffset[0] = i;
        shiftFlags = 0b000;
        fetchFlags = 0b001;
        break;
      case PrimitiveMode::LINES:
        idxOffset[0] = i * 2 + 0;
        idxOffset[1] = i * 2 + 1;
        shiftFlags = 0b000;
        fetchFlags = 0b011;
        break;
      case PrimitiveMode::LINE_LOOP:
      case PrimitiveMode::LINE_STRIP:
        idxOffset[0] = i;
        idxOffset[1] = (i + 1) % numVertices;
        if (i == 0) {
          shiftFlags = 0b000;
          fetchFlags = 0b011;
        } else {
          shiftFlags = 0b001;
          fetchFlags = 0b010;
        }
        break;
      case PrimitiveMode::TRIANGLES:
        idxOffset[0] = i * 3 + 0;
        idxOffset[1] = i * 3 + 1;
        idxOffset[2] = i * 3 + 2;
        shiftFlags = 0b000;
        fetchFlags = 0b111;
        break;
      case PrimitiveMode::TRIANGLE_STRIP:
        idxOffset[0] = i + 0;
        idxOffset[1] = i + 1;
        idxOffset[2] = i + 2;
        if (i == 0) {
          shiftFlags = 0b000;
          fetchFlags = 0b111;
        } else {
          shiftFlags = 0b011;
          fetchFlags = 0b100;
        }
        reverse = (i % 2 == 1);
        break;
      case PrimitiveMode::TRIANGLE_FAN:
        idxOffset[0] = 0;
        idxOffset[1] = i + 1;
        idxOffset[2] = i + 2;
        if (i == 0) {
          shiftFlags = 0b000;
          fetchFlags = 0b111;
        } else {
          shiftFlags = 0b010;
          fetchFlags = 0b100;
        }
        break;
      default: return;
    }

    if (primIdx) {
      idxData[0] = primIdx->data[idxOffset[0]];
      idxData[1] = primIdx->data[idxOffset[1]];
      idxData[2] = primIdx->data[idxOffset[2]];
    } else {
      idxData[0] = idxOffset[0];
      idxData[1] = idxOffset[1];
      idxData[2] = idxOffset[2];
    }

    for (int j = 0; j < 2; j++) {
      if (shiftFlags & (1 << j)) {
        bakedVerts[j] = bakedVerts[j + 1];
      }
    }

    for (int j = 0; j < 3; j++) {
      if (fetchFlags & (1 << j)) {
        Vertex3D out;

        // model transform
        out.pos = primPos->data[idxData[j]];
        if (hasFlag(flags, RenderFlags3D::VERTEX_NORMAL)) {
          out.normal = primNorm->data[idxData[j]];
        } else {
          out.normal = vec3(0, 0, 1);
        }

        // vertex color
        if (hasFlag(flags, RenderFlags3D::VERTEX_COLOR)) {
          out.color = primCol->data[idxData[j]];
        } else {
          out.color = colorf(1.0f, 1.0f, 1.0f, 1.0f);
        }

        // uv
        if (primUv) {
          out.uv = primUv->data[idxData[j]];
        } else {
          out.uv = vec2(0, 0);
        }

        // vertex shading
        if (hasFlag(flags, RenderFlags3D::CUSTOM_VERTEX_SHADER)) {
          mat->vertexShader->process(&out);
          out.pos = viewProjectionMatrix.transform(out.pos);
        } else {
          out.normal = modelMatrix.transformNormal(out.normal);
          if (hasFlag(matFlags, MaterialFlags3D::ENVIRONMENT_MAPPED)) {
            vec3 worldPos = modelMatrix.transform(out.pos);
            vec3 viewDir = (eyePosition - worldPos).normalized();
            vec3 reflectDir =
                (out.normal * 2 * out.normal.dot(viewDir) - viewDir)
                    .normalized();
            out.uv.x = 0.5f + atan2f(reflectDir.z, reflectDir.x) / (2 * M_PI);
            out.uv.y = 0.5f - asinf(reflectDir.y) / M_PI;
          }
          out.pos = mvpMatrix.transform(out.pos);
          if (mat && hasFlag(mat->flags, MaterialFlags3D::HAS_BASE_COLOR)) {
            if (hasFlag(flags, RenderFlags3D::VERTEX_COLOR)) {
              out.color *= mat->baseColor;
            } else {
              out.color = mat->baseColor;
            }
          }
          if (hasFlag(flags, RenderFlags3D::LIGHTING)) {
            colorf light;
            light += envLight;
            float ndotl = fmaxf(0, out.normal.dot(parallelLightDir));
            colorf p = parallelLightColor;
            p.a = 1;
            p.r *= ndotl;
            p.g *= ndotl;
            p.b *= ndotl;
            light += p;
            out.color *= light;
          }
        }

        bakedVerts[j] = out;
      }
    }

    switch (prim->mode) {
      case PrimitiveMode::POINTS: renderPoint(bakedVerts[0], mat); break;
      case PrimitiveMode::LINES:
      case PrimitiveMode::LINE_LOOP:
      case PrimitiveMode::LINE_STRIP:
        // todo: implement
        break;
      case PrimitiveMode::TRIANGLES:
      case PrimitiveMode::TRIANGLE_STRIP:
      case PrimitiveMode::TRIANGLE_FAN:
        if (hasFlag(matFlags, MaterialFlags3D::ENVIRONMENT_MAPPED)) {
          // fix uv seam for environment mapping
          float dx01 = bakedVerts[1].uv.x - bakedVerts[0].uv.x;
          float dx02 = bakedVerts[2].uv.x - bakedVerts[0].uv.x;
          if (dx01 > 0.5f) {
            bakedVerts[1].uv.x -= 1.0f;
          } else if (dx01 < -0.5f) {
            bakedVerts[1].uv.x += 1.0f;
          }
          if (dx02 > 0.5f) {
            bakedVerts[2].uv.x -= 1.0f;
          } else if (dx02 < -0.5f) {
            bakedVerts[2].uv.x += 1.0f;
          }
          float dy01 = bakedVerts[1].uv.y - bakedVerts[0].uv.y;
          float dy02 = bakedVerts[2].uv.y - bakedVerts[0].uv.y;
          if (dy01 > 0.5f) {
            bakedVerts[1].uv.y -= 1.0f;
          } else if (dy01 < -0.5f) {
            bakedVerts[1].uv.y += 1.0f;
          }
          if (dy02 > 0.5f) {
            bakedVerts[2].uv.y -= 1.0f;
          } else if (dy02 < -0.5f) {
            bakedVerts[2].uv.y += 1.0f;
          }
        }
        if (reverse) {
          renderTriangle(bakedVerts[0], bakedVerts[2], bakedVerts[1], mat);
        } else {
          renderTriangle(bakedVerts[0], bakedVerts[1], bakedVerts[2], mat);
        }
        break;
    }
  }

  if (hasFlag(flags, RenderFlags3D::CUSTOM_VERTEX_SHADER)) {
    mat->vertexShader->endPrimitive();
  }
}

}  // namespace xmc
