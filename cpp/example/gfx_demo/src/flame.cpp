#include "flame.hpp"

using namespace xmc;

static constexpr int FIRE_NUM_COLS = 16;
static constexpr int FIRE_NUM_ROWS = 16;
Vec3Buffer bodyVerts = createVec3Buffer((FIRE_NUM_COLS + 1) * 2);
ColorBuffer bodyColors = createColorBuffer((FIRE_NUM_COLS + 1) * 2);
Vec3Buffer bottomVerts = createVec3Buffer(FIRE_NUM_COLS + 2);
ColorBuffer bottomColors = createColorBuffer(FIRE_NUM_COLS + 2);
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
FlameParticle particles[FIRE_NUM_COLS * (FIRE_NUM_ROWS + 1)];

float fireSpeed = 0.05f;
float fireBuoyancy = 10.0f;    // 浮力
float fireAttraction = 0.55f;  // 引力
float fireRepulsion = 0.035f;  // 斥力

void setupFlame() { fireMat->flags |= MaterialFlags3D::DOUBLE_SIDED; }

void updateFlame(float dt) {
  constexpr float ROOT_RADIUS = 0.05f;
  constexpr float ROOT_VELOCITY = 2.0f;
  constexpr int NUM_NEIGHBOURS = 4;

  float friction = powf(0.1f, fireSpeed);

  // 筒の頂点データを下から上へシフト
  for (int row = FIRE_NUM_ROWS; row >= 1; row--) {
    memcpy(&particles[row * FIRE_NUM_COLS],
           &particles[(row - 1) * FIRE_NUM_COLS],
           sizeof(FlameParticle) * FIRE_NUM_COLS);
  }

  // 根元
  for (int col = 0; col < FIRE_NUM_COLS; col++) {
    float a = col * M_PI * 2 / FIRE_NUM_COLS;
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
  }

  // 加速
  for (int row = FIRE_NUM_ROWS - 1; row >= 1; row--) {
    for (int col = 0; col < FIRE_NUM_COLS; col++) {
      FlameParticle &p = particles[row * FIRE_NUM_COLS + col];
      vec3 &pos = p.pos;
      vec3 &vel = p.vel;
      vec3 acc = {0, 0, 0};

      // 上昇気流 (外側ほど強い)
      acc.y += sqrtf(pos.x * pos.x + pos.z * pos.z) * fireBuoyancy;

      // 上下左右の隣接する頂点との相互作用
      vec3 *neighbours[NUM_NEIGHBOURS];
      neighbours[0] = &particles[row * FIRE_NUM_COLS +
                                 (col + FIRE_NUM_COLS - 1) % FIRE_NUM_COLS]
                           .pos;
      neighbours[1] =
          &particles[row * FIRE_NUM_COLS + (col + 1) % FIRE_NUM_COLS].pos;
      neighbours[2] = &particles[(row + 1) * FIRE_NUM_COLS + col].pos;
      neighbours[3] = &particles[(row - 1) * FIRE_NUM_COLS + col].pos;
      for (int j = 0; j < NUM_NEIGHBOURS; j++) {
        vec3 dir = *neighbours[j] - pos;
        float dd = fmaxf(dir.squaredLength(), 0.01f);
        float d = sqrtf(dd);
        float ddd = dd * d;
        dir *= 1.0f / d;
        acc += dir * (fireAttraction / dd - fireRepulsion / ddd);
      }

      vel *= friction;
      vel += acc * fireSpeed;
    }
  }

  // 移動
  for (int i = 0; i < FIRE_NUM_COLS * (FIRE_NUM_ROWS + 1); i++) {
    particles[i].pos += particles[i].vel * fireSpeed;
  }
}

void renderFlame(xmc::Graphics3D &g3d) {
  g3d->pushState();
  g3d->disableFlags(RenderFlags3D::Z_UPDATE);
  g3d->setBlendMode(BlendMode::ADD);
  g3d->scale(1.5f);
  g3d->translate(0, 2.5f, 0);
  colorf colorH = {0.5f, 0, 1.0f, 1};
  bottomVerts->data[0] = {0, 0, 0};
  bottomColors->data[0] = {0, 0, 0, 1};
  for (int col = 0; col < FIRE_NUM_COLS + 1; col++) {
    bottomVerts->data[col + 1] = particles[col % FIRE_NUM_COLS].pos;
    bottomColors->data[col + 1] = colorH;
  }
  g3d->render(bottomPrim);
  for (int row = 0; row < FIRE_NUM_ROWS - 1; row++) {
    colorf colorL = colorH;
    float alpha = (float)(FIRE_NUM_ROWS - row) / FIRE_NUM_ROWS;
    colorH.r = alpha * 0.1f + alpha * 1.5f;
    colorH.g = alpha * 0.1f + fmaxf(0, alpha * 0.5f - 0.2f);
    colorH.b = alpha * 0.1f;
    colorH.a = 1;
    for (int col = 0; col < FIRE_NUM_COLS + 1; col++) {
      int i = row * FIRE_NUM_COLS + col % FIRE_NUM_COLS;
      bodyVerts->data[col * 2] = particles[i].pos;
      bodyVerts->data[col * 2 + 1] = particles[i + FIRE_NUM_COLS].pos;
      bodyColors->data[col * 2] = colorL;
      bodyColors->data[col * 2 + 1] = colorH;
    }
    g3d->render(bodyPrim);
  }
  g3d->popState();
}
