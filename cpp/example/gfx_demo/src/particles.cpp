#include "particles.hpp"

using namespace xmc;

static constexpr int NUM_PARTICLES = 100;
static constexpr float PARTICLE_SPAWN_R = 8;
static constexpr float PARTICLE_MAX_Y = 10;
struct Particle {
  vec3 vel;
};
Particle particleArray[NUM_PARTICLES];
Vec3Buffer particleVerts = createVec3Buffer(NUM_PARTICLES);
Primitive3D particlePrim =
    createPrimitive3D(PrimitiveMode::POINTS, particleVerts, nullptr, nullptr);
Mesh3D particleMesh = createMesh3D({particlePrim});

void setupParticles() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    vec3 pos;
    do {
      pos.x = (randomF32() * 2 - 1) * PARTICLE_SPAWN_R;
      pos.z = (randomF32() * 2 - 1) * PARTICLE_SPAWN_R;
    } while (pos.length() > PARTICLE_SPAWN_R);
    pos.y = randomF32() * PARTICLE_MAX_Y;
    particleVerts->data[i] = pos;
    particleArray[i].vel = {0, 1, 0};
  }
}

void updateParticles(float dt) {
  uint64_t nowMs = getTimeMs();
  float pt = (float)nowMs * 0.001f;
  float friction = powf(0.1f, dt);
  for (int i = 0; i < NUM_PARTICLES; i++) {
    vec3 &vel = particleArray[i].vel;
    vel *= friction;
    vel.x += sin(i + pt) * dt;
    vel.y += (cos(i + pt * 1.3f) * 0.1f + 0.5f) * dt;
    vel.z += cos(i + pt * 1.1f) * 0.5f * dt;
    vec3 &pos = particleVerts->data[i];
    pos += particleArray[i].vel * dt;
    if (pos.y < 0 || pos.y > PARTICLE_MAX_Y) {
      pos.y = 0;
      do {
        pos.x = (randomF32() * 2 - 1) * PARTICLE_SPAWN_R;
        pos.z = (randomF32() * 2 - 1) * PARTICLE_SPAWN_R;
      } while (pos.length() > PARTICLE_SPAWN_R);
      vel = {0, 1, 0};
    }
  }
}

void renderParticles(Graphics3D &g3d) {
  g3d->pushState();
  g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);
  g3d->render(particleMesh);
  g3d->popState();
}
