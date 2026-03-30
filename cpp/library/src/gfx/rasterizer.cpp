#include "xmc/gfx/3d/rasterizer.hpp"

#include <string.h>

namespace xmc {

static inline void getUvMask(int size, uint32_t *mask, uint32_t *shift) {
  // todo: optimize
  if (size >= 256) {
    *mask = 0xFF;
    *shift = 0;
  } else if (size >= 128) {
    *mask = 0x7F;
    *shift = 1;
  } else if (size >= 64) {
    *mask = 0x3F;
    *shift = 2;
  } else if (size >= 32) {
    *mask = 0x1F;
    *shift = 3;
  } else if (size >= 16) {
    *mask = 0x0F;
    *shift = 4;
  } else if (size >= 8) {
    *mask = 0x07;
    *shift = 5;
  } else if (size >= 4) {
    *mask = 0x03;
    *shift = 6;
  } else if (size >= 2) {
    *mask = 0x01;
    *shift = 7;
  } else {
    *mask = 0x00;
    *shift = 8;
  }
}

void RasterizerClass::clearDepth(depth_t value) {
  int pixelCount = width * height;
  if (value == 0 || value == 0xFF) {
    memset(depthBuff, value, sizeof(depth_t) * pixelCount);
  } else {
    for (int i = 0; i < pixelCount; i++) {
      depthBuff[i] = value;
    }
  }
}

void RasterizerClass::setTarget(Sprite &target, rect_t viewport) {
  this->target = target;
  this->viewport = viewport;
  viewPortMatrix = mat4::identity();
  viewPortMatrix.m[0] = viewport.width / 2.0f;
  viewPortMatrix.m[5] = -viewport.height / 2.0f;
  viewPortMatrix.m[12] = viewport.x + viewport.width / 2.0f;
  viewPortMatrix.m[13] = viewport.y + viewport.height / 2.0f;
  viewPortMatrix.m[15] = 1.0f;
}

void RasterizerClass::setProjectionOrtho(float left, float right, float bottom,
                                         float top, float near, float far) {
  projectionMatrix = viewPortMatrix * mat4::ortho(left, right, bottom, top, near, far);
}
void RasterizerClass::setProjectionPerspective(float fovY, float aspect,
                                               float near, float far) {
  projectionMatrix = viewPortMatrix * mat4::perspective(fovY, aspect, near, far);
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

  const mat4 &tfxMatrix = matrixStack[matrixStackTop];
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
        norm = tfxMatrix.transform(norm);
        pos = tfxMatrix.transform(pos);
        norm -= pos;
        norm = norm.normalized();

        // shading
        BakedVertex &out = bakedVerts[j];
        out.pos = projectionMatrix.transform(pos);
        colorf vert_color = col;
        if (mat) {
          vert_color *= mat->baseColor;
        }
        vert_color *= (envLight + parallelLightColor *
                                      fmaxf(0, norm.dot(parallelLightDir)));
        out.color = vert_color;
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

void RasterizerClass::renderTriangle(const BakedVertex &v0,
                                     const BakedVertex &v1,
                                     const BakedVertex &v2,
                                     const Material3D &mat) {
  const BakedVertex *tri[] = {&v0, &v1, &v2};

  int i0 = 0;
  int i1 = 1;
  int i2 = 2;

  // back-face culling
  float ax = tri[i1]->pos.x - tri[i0]->pos.x;
  float ay = tri[i1]->pos.y - tri[i0]->pos.y;
  float bx = tri[i2]->pos.x - tri[i0]->pos.x;
  float by = tri[i2]->pos.y - tri[i0]->pos.y;
  if (ax * by - ay * bx >= 0) return;

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

  float z0 = 255.0f * (tri[i0]->pos.z - zNear) / (zFar - zNear);
  float z1 = 255.0f * (tri[i1]->pos.z - zNear) / (zFar - zNear);
  float z2 = 255.0f * (tri[i2]->pos.z - zNear) / (zFar - zNear);

  colorf c0 = tri[i0]->color;
  colorf c1 = tri[i1]->color;
  colorf c2 = tri[i2]->color;
  colorf c0to1 = c1 - c0;
  colorf c1to2 = c2 - c1;
  colorf c0to2 = c2 - c0;

  vec2 uv0 = tri[i0]->uv;
  vec2 uv1 = tri[i1]->uv;
  vec2 uv2 = tri[i2]->uv;
  vec2 uv0to1 = uv1 - uv0;
  vec2 uv1to2 = uv2 - uv1;
  vec2 uv0to2 = uv2 - uv0;

  int iy_min = (int)ceilf(y0);
  int iy_max = (int)floorf(y2);
  if (iy_min < viewport.y) iy_min = viewport.y;
  if (iy_max >= viewport.bottom()) iy_max = viewport.bottom() - 1;

  const uint16_t *tex_data = nullptr;
  uint32_t tex_stride = 0;
  uint32_t texMaskU = 0, texMaskV = 0;
  uint32_t texShiftU = 0, texShiftV = 0;
  if (mat && mat->colorTexture) {
    const auto &tex = mat->colorTexture;
    tex_data = (const uint16_t *)tex->linePtr(0);
    tex_stride = tex->stride / sizeof(uint16_t);
    getUvMask(tex->width, &texMaskU, &texShiftU);
    getUvMask(tex->height, &texMaskV, &texShiftV);
  }

  // rasterize the triangle using a scanline algorithm
  // todo: optimize
  for (int iy = iy_min; iy <= iy_max; iy++) {
    float y = (float)iy;

    float xa = x0 + (x2 - x0) * (y - y0) * y0to2inv;
    float za = z0 + (z2 - z0) * (y - y0) * y0to2inv;
    colorf ca = c0 + c0to2 * (y - y0) * y0to2inv;
    vec2 uva = uv0 + uv0to2 * (y - y0) * y0to2inv;

    float xb;
    float zb;
    colorf cb;
    vec2 uvb;
    if (y < y1) {
      xb = x0 + (x1 - x0) * (y - y0) * y0to1inv;
      zb = z0 + (z1 - z0) * (y - y0) * y0to1inv;
      cb = c0 + c0to1 * (y - y0) * y0to1inv;
      uvb = uv0 + uv0to1 * (y - y0) * y0to1inv;
    } else {
      xb = x1 + (x2 - x1) * (y - y1) * y1to2inv;
      zb = z1 + (z2 - z1) * (y - y1) * y1to2inv;
      cb = c1 + c1to2 * (y - y1) * y1to2inv;
      uvb = uv1 + uv1to2 * (y - y1) * y1to2inv;
    }

    if (xa > xb) {
      std::swap(xa, xb);
      std::swap(za, zb);
      std::swap(ca, cb);
      std::swap(uva, uvb);
    }

    uint32_t ua = (uint32_t)(uva.x * 0x1000000) >> texShiftU;
    uint32_t va = (uint32_t)(uva.y * 0x1000000) >> texShiftV;
    uint32_t ub = (uint32_t)(uvb.x * 0x1000000) >> texShiftU;
    uint32_t vb = (uint32_t)(uvb.y * 0x1000000) >> texShiftV;

    float zstep = 0;
    colorf cstep = {0, 0, 0, 0};
    int32_t ustep = 0, vstep = 0;
    if (xb > xa) {
      zstep = (zb - za) / (xb - xa);
      cstep = (cb - ca) / (xb - xa);
      ustep = ((int32_t)ub - (int32_t)ua) / (xb - xa);
      vstep = ((int32_t)vb - (int32_t)va) / (xb - xa);
    }

    int ixMin = (int)ceilf(xa);
    int ixMax = (int)floorf(xb);
    if (ixMin < viewport.x) {
      ixMin = viewport.x;
      za += zstep * (ixMin - xa);
      ca += cstep * (ixMin - xa);
      ua += ustep * (ixMin - xa);
      va += vstep * (ixMin - xa);
    }
    if (ixMax >= viewport.right()) {
      ixMax = viewport.right() - 1;
    }

    if (target->format == pixel_format_t::RGB565) {
      uint16_t *cptr = (uint16_t *)target->linePtr(iy) + ixMin;
      depth_t *zptr = depthBuff + iy * width + ixMin;
      for (int x = ixMin; x <= ixMax; x++) {
        if (0 <= za && za <= 255 && za < *zptr) {
          colorf col = ca;
          if (tex_data) {
            uint32_t u = ua >> 16;
            uint32_t v = va >> 16;
            uint16_t texel =
                tex_data[(v & texMaskV) * tex_stride + (u & texMaskU)];
            col *= colorf::from4444(texel);
          }
          if (col.a >= 0.5f) {
            *cptr = col.to565();
            *zptr = (depth_t)fminf(255, fmaxf(0, za));
          }
        }
        zptr++;
        cptr++;
        za += zstep;
        ca += cstep;
        ua += ustep;
        va += vstep;
      }
    }
  }
}

}  // namespace xmc
