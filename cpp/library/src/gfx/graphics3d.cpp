#include "xmc/gfx3d/graphics3d.hpp"
#include "xmc/gfx3d/custom_shader.hpp"

#include <string.h>

// todo: delete
#include "xmc/hw/gpio.hpp"
//        gpio::setDir(XMC_PIN_GPIO_0, true);
//        gpio::write(XMC_PIN_GPIO_0, true);
//        gpio::write(XMC_PIN_GPIO_0, false);

namespace xmc {

void Graphics3DClass::beginRender(ClearTarget target) {
  if (hasFlag(target, ClearTarget::STACK)) {
    stateStackTop = 0;
    stateStack[0] = State3D();
  }
  if (hasFlag(target, ClearTarget::DEPTH)) {
    memset(depthBuff, 0xFF, sizeof(Depth3D) * width * height);
  }
}

void Graphics3DClass::endRender() {
  if (stateStackTop) {
    XMC_ERR_LOG(XMC_USER_GENERIC_ERROR);
  }
}

void Graphics3DClass::setTarget(Sprite target, Rect viewport) {
  this->target = target;
  this->viewport = viewport;
  screenMatrix = mat4::identity();
  screenMatrix.m[0] = (float)viewport.width / 2;
  screenMatrix.m[5] = -(float)viewport.height / 2;
  screenMatrix.m[12] = viewport.x + (float)viewport.width / 2;
  screenMatrix.m[13] = viewport.y + (float)viewport.height / 2;
  viewProjectionDirty = true;
  mvpDirty = true;
}

void Graphics3DClass::validateMatrix(bool force) {
  if (mvpDirty || force) {
    // Recalculate model matrix
    int dirtyIndex = stateStackTop + 1;
    if (force) {
      dirtyIndex = 0;
    } else {
      while (dirtyIndex > 0 && stateStack[dirtyIndex - 1].dirty) {
        dirtyIndex--;
      }
    }
    while (dirtyIndex <= stateStackTop) {
      if (dirtyIndex == 0) {
        stateStack[dirtyIndex].world = stateStack[dirtyIndex].local;
      } else {
        stateStack[dirtyIndex].world =
            stateStack[dirtyIndex - 1].world * stateStack[dirtyIndex].local;
      }
      stateStack[dirtyIndex].dirty = false;
      dirtyIndex++;
    }

    // Recalculate view-projection matrix
    if (viewProjectionDirty || force) {
      viewProjectionMatrix = screenMatrix * projectionMatrix * viewMatrix;
      viewProjectionDirty = false;
    }

    // Recalculate MVP matrix
    mvpMatrix = viewProjectionMatrix * stateStack[stateStackTop].world;
    mvpDirty = false;
  }
}

void Graphics3DClass::getModelMatrix(mat4 &out) {
  validateMatrix();
  out = stateStack[stateStackTop].world;
}

void Graphics3DClass::getMvpMatrix(mat4 &out) {
  validateMatrix();
  out = mvpMatrix;
}

void Graphics3DClass::render(const Scene3D &scene) {
  for (const Node3D &node : scene->rootNodes) {
    render(node);
  }
}

void Graphics3DClass::render(const Node3D &node) {
  pushState();
  loadMatrix(node->transform);
  if (node->mesh) {
    render(node->mesh);
  }
  for (const Node3D &child : node->children) {
    render(child);
  }
  popState();
}

void Graphics3DClass::render(const Mesh3D &mesh) {
  for (const Primitive3D &prim : mesh->primitives) {
    render(prim);
  }
}

void Graphics3DClass::render(const Primitive3D &prim) {
  const State3D &state = stackTop();
  RenderFlags3D flags = state.flags;

  const vec3 *primPos = prim->position ? prim->position->data : nullptr;
  const vec3 *primNorm = prim->normal ? prim->normal->data : nullptr;
  const colorf *primCol = prim->color ? prim->color->data : nullptr;
  const vec2 *primUv = prim->uv ? prim->uv->data : nullptr;
  const uint16_t *primIdx = prim->indexes ? prim->indexes->data : nullptr;
  const Material3D &mat = prim->material;

  MaterialFlags3D matFlags = MaterialFlags3D::NONE;
  if (mat) {
    matFlags = mat->flags;
  }

  if (!primPos) return;

  int idxOffset[3];
  int idxData[3];

  validateMatrix(false);

  if (!primNorm) {
    flags &= ~RenderFlags3D::VERTEX_NORMAL;
    flags &= ~RenderFlags3D::LIGHTING;
  }
  if (!primCol) {
    flags &= ~RenderFlags3D::VERTEX_COLOR;
  }
  if (!mat || !mat->vertexShader) {
    flags &= ~RenderFlags3D::CUSTOM_VERTEX_SHADER;
  }
  if (!mat || !mat->colorTexture) {
    flags &= ~RenderFlags3D::COLOR_TEXTURE;
  }

  mat4 &modelMatrix = stateStack[stateStackTop].world;
  if (hasFlag(flags, RenderFlags3D::CUSTOM_VERTEX_SHADER)) {
    VertexShaderArgs shaderArgs(flags, modelMatrix, viewProjectionMatrix, prim,
                                mat);
    mat->vertexShader->beginPrimitive(shaderArgs);
  }

  workerArgs.target = target;
  workerArgs.renderFlags = flags;
  workerArgs.blendMode = state.blendMode;
  workerArgs.depthBuff = depthBuff;
  workerArgs.depthStride = width;
  workerArgs.xMin = viewport.x;
  workerArgs.xMax = viewport.x + viewport.width;
  if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
    const auto &t = mat->colorTexture;
    int uBits = (int)floorf(log2f(t->width));
    int vBits = (int)floorf(log2f(t->height));
    workerArgs.textureData = (const uint16_t *)t->linePtr(0);
    workerArgs.textureStride = t->stride / sizeof(uint16_t);
    workerArgs.textureWidth = 1 << uBits;
    workerArgs.textureHeight = 1 << vBits;
    workerArgs.uMask = workerArgs.textureWidth - 1;
    workerArgs.vMask = workerArgs.textureHeight - 1;
  }
  if (hasFlag(flags, RenderFlags3D::Z_TEST)) {
    workerArgs.zTestOffset = state.zTestOffset;
  } else {
    workerArgs.zTestOffset = -MAX_DEPTH;
  }

  if (multicoreMode != MultiCoreMode3D::NONE) {
    subWorker.beginPrimitive(workerArgs);
  }

  int numVertices = prim->numVertices();
  int numElems = prim->numElements();

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
      idxData[0] = primIdx[idxOffset[0]];
      idxData[1] = primIdx[idxOffset[1]];
      idxData[2] = primIdx[idxOffset[2]];
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
        Vertex3D &out = bakedVerts[j];

        // model transform
        out.pos = primPos[idxData[j]];
        if (hasFlag(flags, RenderFlags3D::VERTEX_NORMAL)) {
          out.normal = primNorm[idxData[j]];
        } else {
          out.normal = vec3(0, 0, 1);
        }

        // vertex color
        if (hasFlag(flags, RenderFlags3D::VERTEX_COLOR)) {
          out.color = primCol[idxData[j]];
        } else {
          out.color = colorf(1.0f, 1.0f, 1.0f, 1.0f);
        }

        // uv
        if (primUv) {
          out.uv = primUv[idxData[j]];
        } else {
          out.uv = vec2(0, 0);
        }

        // vertex shading
        if (hasFlag(flags, RenderFlags3D::CUSTOM_VERTEX_SHADER)) {
          mat->vertexShader->process(&out);
          out.pos = viewProjectionMatrix.transform(out.pos);
        } else {
          if (hasFlag(flags, RenderFlags3D::LIGHTING) |
              hasFlag(matFlags, MaterialFlags3D::ENVIRONMENT_MAPPED)) {
            out.normal = modelMatrix.transformNormal(out.normal);
          }
          if (hasFlag(matFlags, MaterialFlags3D::ENVIRONMENT_MAPPED)) {
            vec3 worldPos = modelMatrix.transform(out.pos);
            vec3 viewDir = (eyePosition - worldPos).normalized();
            vec3 refDir = (out.normal * (out.normal.dot(viewDir) * 2) - viewDir)
                              .normalized();
            out.uv.x = 0.5f + atan2f(refDir.z, refDir.x) / (2 * M_PI);
            out.uv.y = 0.5f - asinf(refDir.y) / M_PI;
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
            light += state.envLight;
            float ndotl = fmaxf(0, out.normal.dot(state.parallelLightDir));
            colorf p = state.parallelLightColor;
            p.a = 1;
            p.r *= ndotl;
            p.g *= ndotl;
            p.b *= ndotl;
            light += p;
            out.color *= light;
          }
        }
      }
    }

    switch (prim->mode) {
      case PrimitiveMode::POINTS:
        renderPoint(workerArgs, bakedVerts[0], mat);
        break;
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
          renderTriangle(workerArgs, bakedVerts[0], bakedVerts[2],
                         bakedVerts[1], mat);
        } else {
          renderTriangle(workerArgs, bakedVerts[0], bakedVerts[1],
                         bakedVerts[2], mat);
        }
        break;
    }
  }

  if (hasFlag(flags, RenderFlags3D::CUSTOM_VERTEX_SHADER)) {
    mat->vertexShader->endPrimitive();
  }

  if (multicoreMode != MultiCoreMode3D::NONE) {
    subWorker.endPrimitive();
  }
}

void Graphics3DClass::renderTriangle(WorkerArgs3D &workerArgs,
                                     const Vertex3D &v0, const Vertex3D &v1,
                                     const Vertex3D &v2,
                                     const Material3D &mat) {
  const State3D &state = stackTop();
  RenderFlags3D flags = state.flags;
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

  float z0 = (float)MAX_DEPTH * (tri[i0]->pos.z - state.zNear) /
             (state.zFar - state.zNear);
  float z1 = (float)MAX_DEPTH * (tri[i1]->pos.z - state.zNear) /
             (state.zFar - state.zNear);
  float z2 = (float)MAX_DEPTH * (tri[i2]->pos.z - state.zNear) /
             (state.zFar - state.zNear);
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

  //               cornerT
  //                  *
  //                 / ＼
  //                /    ＼
  //  - - - - - - -/-------＼- - - - - Top of Viewport
  //              /      <---＼---- Upper Trapezoid
  //     cornerL * - - - - - - * cornerR
  //            /     <----／------ Lower Trapezoid
  //           /        ／
  //  - - - - /------／- - - - - - - - Bottom of Viewport
  //         /    ／
  //        /  ／
  //       /／
  //       *
  //   cornerB

  trapU.topLeft.x = fixed16p16::fromFloat(x0);
  trapL.topLeft.x = fixed16p16::fromFloat(x1);
  trapL.topRight.x = fixed16p16::fromFloat(x0 + (x2 - x0) * yT);
  esvB.x = fixed16p16::fromFloat(x2);

  trapU.topLeft.z = fixed20p12::fromFloat(z0);
  trapL.topLeft.z = fixed20p12::fromFloat(z1);
  trapL.topRight.z = fixed20p12::fromFloat(z0 + (z2 - z0) * yT);
  esvB.z = fixed20p12::fromFloat(z2);

  const colorf &c0 = tri[i0]->color;
  const colorf &c1 = tri[i1]->color;
  const colorf &c2 = tri[i2]->color;
  trapU.topLeft.c = color8p24(c0);
  trapL.topLeft.c = color8p24(c1);
  trapL.topRight.c = color8p24(c0 + (c2 - c0) * yT);
  esvB.c = color8p24(c2);

  if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
    const vec2 &uv0 = tri[i0]->uv;
    const vec2 &uv1 = tri[i1]->uv;
    const vec2 &uv2 = tri[i2]->uv;
    const int tw = workerArgs.textureWidth;
    const int th = workerArgs.textureHeight;
    trapU.topLeft.u = fixed12p20::fromFloat(uv0.x * tw);
    trapL.topLeft.u = fixed12p20::fromFloat(uv1.x * tw);
    trapL.topRight.u =
        fixed12p20::fromFloat((uv0.x + (uv2.x - uv0.x) * yT) * tw);
    esvB.u = fixed12p20::fromFloat(uv2.x * tw);
    trapU.topLeft.v = fixed12p20::fromFloat(uv0.y * th);
    trapL.topLeft.v = fixed12p20::fromFloat(uv1.y * th);
    trapL.topRight.v =
        fixed12p20::fromFloat((uv0.y + (uv2.y - uv0.y) * yT) * th);
    esvB.v = fixed12p20::fromFloat(uv2.y * th);
  }

  trapU.topRight = trapU.topLeft;

  if (trapL.topLeft.x > trapL.topRight.x) {
    std::swap(trapL.topLeft, trapL.topRight);
  }

  // rasterize the triangle using a scanline algorithm
  // todo: optimize
  bool useGouraud = hasFlag(flags, RenderFlags3D::GOURAUD_SHADING);
  bool useTexture = hasFlag(flags, RenderFlags3D::COLOR_TEXTURE);
  int yOffset;

  // upper trapezoid
  yOffset = iyMin > iy0 ? iyMin - iy0 : 0;
  trapU.yTop = iy0 + yOffset;
  trapU.yBottom = (int)fminf(iy1, iyMax);
  if (trapU.yBottom > trapU.yTop) {
    trapU.stepLeft = EdgeScanVars::subDiv(trapU.topLeft, trapL.topLeft, vSpanT);
    trapU.stepRight =
        EdgeScanVars::subDiv(trapU.topRight, trapL.topRight, vSpanT);
    trapU.topLeft.step(trapU.stepLeft, yOffset);
    trapU.topRight.step(trapU.stepRight, yOffset);
    if (multicoreMode == MultiCoreMode3D::INTERLACE) {
      EdgeScanVars stepL = trapU.stepLeft;
      EdgeScanVars stepR = trapU.stepRight;
      trapU.yStep = 2;
      trapU.stepLeft.step(trapU.stepLeft);
      trapU.stepRight.step(trapU.stepRight);
      subWorker.push(trapU);
      trapU.yTop += 1;
      trapU.topLeft.step(stepL);
      trapU.topRight.step(stepR);
      renderTrapezoid3D(workerArgs, trapU);
    } else if (multicoreMode == MultiCoreMode3D::PIPELINE) {
      trapU.yStep = 1;
      subWorker.push(trapU);
    } else {
      trapU.yStep = 1;
      renderTrapezoid3D(workerArgs, trapU);
    }
  }

  // lower trapezoid
  yOffset = iyMin > iy1 ? iyMin - iy1 : 0;
  trapL.yTop = iy1 + yOffset;
  trapL.yBottom = (int)fminf(iy2, iyMax);
  if (trapL.yBottom > trapL.yTop) {
    trapL.stepLeft = EdgeScanVars::subDiv(trapL.topLeft, esvB, vSpanB);
    trapL.stepRight = EdgeScanVars::subDiv(trapL.topRight, esvB, vSpanB);
    trapL.topLeft.step(trapL.stepLeft, yOffset);
    trapL.topRight.step(trapL.stepRight, yOffset);
    if (multicoreMode == MultiCoreMode3D::INTERLACE) {
      EdgeScanVars stepL = trapL.stepLeft;
      EdgeScanVars stepR = trapL.stepRight;
      trapL.yStep = 2;
      trapL.stepLeft.step(trapL.stepLeft);
      trapL.stepRight.step(trapL.stepRight);
      subWorker.push(trapL);
      trapL.yTop += 1;
      trapL.topLeft.step(stepL);
      trapL.topRight.step(stepR);
      renderTrapezoid3D(workerArgs, trapL);
    } else if (multicoreMode == MultiCoreMode3D::PIPELINE) {
      trapL.yStep = 1;
      subWorker.push(trapL);
    } else {
      trapL.yStep = 1;
      renderTrapezoid3D(workerArgs, trapL);
    }
  }
}

void Graphics3DClass::renderPoint(WorkerArgs3D &workerArgs, const Vertex3D &v0,
                                  const Material3D &mat) {
  const State3D &state = stackTop();
  RenderFlags3D flags = state.flags;
  if (!mat || !mat->colorTexture ||
      !hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
    flags &= ~RenderFlags3D::COLOR_TEXTURE;
  }

  if (v0.pos.x < viewport.x || v0.pos.x >= viewport.right()) return;

  float x0 = v0.pos.x, y0 = v0.pos.y;

  float z0 =
      (float)MAX_DEPTH * (v0.pos.z - state.zNear) / (state.zFar - state.zNear);
  if (z0 < 0 || z0 > MAX_DEPTH) {
    return;
  }

  int iy0 = (int)floorf(y0);
  if (iy0 < viewport.y || iy0 >= viewport.bottom()) return;

  esvL.x = fixed16p16::fromFloat(x0);
  esvR.x = fixed16p16::fromFloat(x0 + 1);
  esvL.z = fixed20p12::fromFloat(z0);
  esvR.z = fixed20p12::fromFloat(z0);

  if (hasFlag(flags, RenderFlags3D::GOURAUD_SHADING)) {
    esvL.c = color8p24(v0.color);
    esvR.c = esvL.c;
  } else {
    esvL.c = color8p24(fixed8p24::fromFloat(1.0f), fixed8p24::fromFloat(1.0f),
                       fixed8p24::fromFloat(1.0f), fixed8p24::fromFloat(1.0f));
    esvR.c = esvL.c;
  }

  if (hasFlag(flags, RenderFlags3D::COLOR_TEXTURE)) {
    const vec2 &uv0 = v0.uv;
    const int tw = workerArgs.textureWidth;
    const int th = workerArgs.textureHeight;
    esvL.u = fixed12p20::fromFloat(uv0.x * tw);
    esvR.u = fixed12p20::fromFloat(uv0.x * tw);
    esvL.v = fixed12p20::fromFloat(uv0.y * th);
    esvR.v = fixed12p20::fromFloat(uv0.y * th);
  }

  EdgeScanVars dummyStep;
  Trapezoid3D trap;
  trap.topLeft = esvL;
  trap.topRight = esvR;
  trap.stepLeft = dummyStep;
  trap.stepRight = dummyStep;
  trap.yTop = iy0;
  trap.yBottom = iy0 + 1;
  trap.yStep = 1;
  if (multicoreMode != MultiCoreMode3D::NONE) {
    subWorker.push(trap);
  } else {
    renderTrapezoid3D(workerArgs, trap);
  }
}

}  // namespace xmc
