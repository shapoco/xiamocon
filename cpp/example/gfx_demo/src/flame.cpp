#include "flame.hpp"

using namespace xmc;

constexpr int COLS = 16;
constexpr int ROWS = 16;
Vec3Buffer bodyVerts = createVec3Buffer((COLS + 1) * 2);
ColorBuffer bodyColors = createColorBuffer((COLS + 1) * 2);
Vec3Buffer bottomVerts = createVec3Buffer(COLS + 2);
ColorBuffer bottomColors = createColorBuffer(COLS + 2);
Material3D fireMat = createMaterial3D();
Primitive3D bodyPrim =
    createPrimitive3D(PrimitiveMode::TRIANGLE_STRIP, bodyVerts, nullptr,
                      bodyColors, nullptr, nullptr, fireMat);
Primitive3D bottomPrim =
    createPrimitive3D(PrimitiveMode::TRIANGLE_FAN, bottomVerts, nullptr,
                      bottomColors, nullptr, nullptr, fireMat);

struct FlameParticle {
  vec3 pos;
  vec3 vel;
};

// Allocate particles in PSRAM because ESP32S3 has limited SRAM
FlameParticle *particles = (FlameParticle *)xmcMalloc(
    sizeof(FlameParticle) * COLS * (ROWS + 1), XMC_HEAP_CAP_SPIRAM);

constexpr float COL_SHIFT_INTERVAL = 0.05f;
float timeAccum = 0;

float flameBuoyancy = 10.0f;    // 浮力
float flameAttraction = 0.55f;  // 引力
float flameRepulsion = 0.035f;  // 斥力

void setupFlame() {
  fireMat->flags |= MaterialFlags3D::DOUBLE_SIDED;
  for (int i = 0; i < COLS * (ROWS + 1); i++) {
    particles[i].pos = {0, 0, 0};
    particles[i].vel = {0, 0, 0};
  }
}

void updateFlame(float dt) {
  constexpr float ROOT_RADIUS = 0.05f;
  constexpr float ROOT_VELOCITY = 2.0f;
  constexpr int NUM_NEIGHBOURS = 4;

  float friction = powf(0.1f, dt);

  // shift vertex data up the column every COL_SHIFT_INTERVAL seconds
  timeAccum += dt;
  while (timeAccum >= COL_SHIFT_INTERVAL) {
    timeAccum -= COL_SHIFT_INTERVAL;
    for (int row = ROWS; row >= 1; row--) {
      memcpy(&particles[row * COLS], &particles[(row - 1) * COLS],
             sizeof(FlameParticle) * COLS);
    }

    // bottom vertices
    for (int col = 0; col < COLS; col++) {
      float a = col * M_PI * 2 / COLS;
      float noise = 1.0f + randomF32() * 0.5f;
      float cs = cosf(a);
      float sn = sinf(a);
      FlameParticle &p = particles[col];
      p.pos.x = cs * ROOT_RADIUS * noise;
      p.pos.y = 0;
      p.pos.z = sn * ROOT_RADIUS * noise;
      p.vel.x = cs * ROOT_VELOCITY * noise;
      p.vel.y = 2.0f * noise;
      p.vel.z = sn * ROOT_VELOCITY * noise;
      p.pos += p.vel * timeAccum;
    }
  }

  // acceleration
  for (int row = ROWS - 1; row >= 1; row--) {
    for (int col = 0; col < COLS; col++) {
      FlameParticle &p = particles[row * COLS + col];
      vec3 acc = {0, 0, 0};

      // buoyancy is stronger towards the edges
      acc.y += sqrtf(p.pos.x * p.pos.x + p.pos.z * p.pos.z) * flameBuoyancy;

      // interaction with adjacent vertices (up, down, left, right)
      vec3 *neighbours[NUM_NEIGHBOURS];
      neighbours[0] = &particles[row * COLS + (col + COLS - 1) % COLS].pos;
      neighbours[1] = &particles[row * COLS + (col + 1) % COLS].pos;
      neighbours[2] = &particles[(row + 1) * COLS + col].pos;
      neighbours[3] = &particles[(row - 1) * COLS + col].pos;
      for (int j = 0; j < NUM_NEIGHBOURS; j++) {
        vec3 dir = *neighbours[j] - p.pos;
        float dd = fmaxf(dir.squaredLength(), 0.01f);
        float d = sqrtf(dd);
        float ddd = dd * d;
        dir *= 1.0f / d;
        acc += dir * (flameAttraction / dd - flameRepulsion / ddd);
      }

      p.vel *= friction;
      p.vel += acc * dt;
    }
  }

  // update positions
  for (int i = 0; i < COLS * (ROWS + 1); i++) {
    particles[i].pos += particles[i].vel * dt;
  }
}

void renderFlame(xmc::Graphics3D &g3d, bool alphaBlending) {
  g3d->pushState();
  g3d->disableFlags(RenderFlags3D::Z_UPDATE);
  if (alphaBlending) {
    g3d->setBlendMode(BlendMode::ADD);
  }
  g3d->scale(1.5f);
  g3d->translate(0, 2.5f, 0);

  // render bottom lid
  colorf colorH = {0.5f, 0, 1.0f, 1};
  bottomVerts->data[0] = {0, 0, 0};
  bottomColors->data[0] = {0, 0, 0, 1};
  for (int col = 0; col < COLS + 1; col++) {
    bottomVerts->data[col + 1] = particles[col % COLS].pos;
    bottomColors->data[col + 1] = colorH;
  }
  g3d->render(bottomPrim);

  // render body cylinders
  for (int row = 0; row < ROWS - 1; row++) {
    colorf colorL = colorH;
    float alpha = (float)(ROWS - row) / ROWS;
    colorH.r = alpha * 0.1f + alpha * 1.5f;
    colorH.g = alpha * 0.1f + fmaxf(0, alpha * 0.5f - 0.2f);
    colorH.b = alpha * 0.1f;
    colorH.a = 1;
    for (int col = 0; col < COLS + 1; col++) {
      int i = row * COLS + col % COLS;
      bodyVerts->data[col * 2] = particles[i].pos;
      bodyVerts->data[col * 2 + 1] = particles[i + COLS].pos;
      bodyColors->data[col * 2] = colorL;
      bodyColors->data[col * 2 + 1] = colorH;
    }
    g3d->render(bodyPrim);
  }

  g3d->popState();
}
