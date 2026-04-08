#include "xmc/gfx3d/graphics3d.hpp"

#include <string.h>

namespace xmc {

void RasterizerClass::clearDepth(depth_t value) {
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

void RasterizerClass::setTarget(Sprite target, Rect viewport) {
  this->target = target;
  this->viewport = viewport;
  screenMatrix = mat4::identity();
  screenMatrix.m[0] = viewport.width / 2.0f;
  screenMatrix.m[5] = -viewport.height / 2.0f;
  screenMatrix.m[12] = viewport.x + viewport.width / 2.0f;
  screenMatrix.m[13] = viewport.y + viewport.height / 2.0f;
  vpDirty = true;
  mvpDirty = true;
}

void RasterizerClass::validateMatrix(bool force) {
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

    // Recalculate PV matrix
    if (vpDirty || force) {
      vpMatrix = screenMatrix * projectionMatrix * viewMatrix;
      vpDirty = false;
    }

    // Recalculate MVP matrix
    mvpMatrix = vpMatrix * modelMatrixStack[modelMatrixStackPtr].world;
    mvpDirty = false;
  }
}

void RasterizerClass::getModelMatrix(mat4 &out) {
  validateMatrix();
  out = modelMatrixStack[modelMatrixStackPtr].world;
}

void RasterizerClass::getMvpMatrix(mat4 &out) {
  validateMatrix();
  out = mvpMatrix;
}

void RasterizerClass::renderScene(const Scene3D &scene) {
  for (const Node3D &node : scene->rootNodes) {
    renderNode(node);
  }
}

void RasterizerClass::renderNode(const Node3D &node) {
  pushMatrix();
  loadMatrix(node->transform);
  if (node->mesh) {
    renderMesh(node->mesh);
  }
  for (const Node3D &child : node->children) {
    renderNode(child);
  }
  popMatrix();
}

void RasterizerClass::renderMesh(const Mesh3D &mesh) {
  for (const Primitive3D &prim : mesh->primitives) {
    renderPrimitive(prim, prim->material);
  }
}

void RasterizerClass::renderPrimitive(const Primitive3D &prim,
                                      const Material3D &mat) {
  const Vec3Buffer &primPos = prim->position;
  const Vec3Buffer &primNorm = prim->normal;
  const ColorBuffer &primCol = prim->color;
  const Vec2Buffer &primUv = prim->uv;
  const IndexBuffer &primIdx = prim->indexes;

  if (!primPos) return;

  int idxOffset[3];
  int idxData[3];

  validateMatrix(true);

  int numVertices = prim->numVertices();
  int numElems = prim->numElements();

  BakedVertex bakedVerts[3];

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

    mat4 &modelMatrix = modelMatrixStack[modelMatrixStackPtr].world;
    for (int j = 0; j < 3; j++) {
      if (fetchFlags & (1 << j)) {
        // fetch attributes
        vec3 pos = primPos->data[idxData[j]];
        vec3 norm = primNorm ? primNorm->data[idxData[j]] : vec3(0, 0, 1);
        colorf col = primCol ? primCol->data[idxData[j]]
                             : colorf(1.0f, 1.0f, 1.0f, 1.0f);
        vec2 uv = primUv ? primUv->data[idxData[j]] : vec2(0, 0);

        // transform
        norm += pos;
        norm = modelMatrix.transform(norm);
        pos = modelMatrix.transform(pos);
        norm -= pos;
        norm = norm.normalized();

        // shading
        BakedVertex &out = bakedVerts[j];
        out.pos = vpMatrix.transform(pos);
        
        if (hasFlag(renderFlags, RenderFlags3D::VERTEX_SHADING)) {
          colorf vertColor = col;
          if (mat) {
            vertColor *= mat->baseColor;
          }
          if (hasFlag(renderFlags, RenderFlags3D::LIGHTING)) {
            colorf light;
            light += envLight;
            float ndotl = fmaxf(0, norm.dot(parallelLightDir));
            colorf p = parallelLightColor;
            p.a = 1;
            p.r *= ndotl;
            p.g *= ndotl;
            p.b *= ndotl;
            light += p;
            vertColor *= light;
          }
          out.color = vertColor;
        }
        out.uv = uv;
      }
    }

    switch (prim->mode) {
      case PrimitiveMode::POINTS:
        // todo: implement
        break;
      case PrimitiveMode::LINES:
      case PrimitiveMode::LINE_LOOP:
      case PrimitiveMode::LINE_STRIP:
        // todo: implement
        break;
      case PrimitiveMode::TRIANGLES:
      case PrimitiveMode::TRIANGLE_STRIP:
      case PrimitiveMode::TRIANGLE_FAN:
        if (reverse) {
          renderTriangle(bakedVerts[0], bakedVerts[2], bakedVerts[1], mat);
        } else {
          renderTriangle(bakedVerts[0], bakedVerts[1], bakedVerts[2], mat);
        }
        break;
    }
  }
}

}  // namespace xmc
