#include "xiamocon.hpp"

#include <stdio.h>

#include "cave_env_texture.hpp"
#include "cave_hole.hpp"
#include "cave_light.hpp"
#include "quarts.hpp"
#include "tulip.hpp"

using namespace xmc;
using namespace xmc::input;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;
FrameBuffer frameBuffer(DISPLAY_FORMAT, true);

uint64_t nextVsyncTimeUs = 0;

Vec3Buffer leafVerts = createVec3Buffer(3);
ColorBuffer leafColors = createColorBuffer(3);
Material3D leafMat = createMaterial3D();
Primitive3D leafPrim =
    createPrimitive3D(PrimitiveMode::TRIANGLES, leafVerts, nullptr, leafColors,
                      nullptr, nullptr, leafMat);
Mesh3D leafMesh = createMesh3D({leafPrim});

static constexpr int NUM_LEAFS = 500;
struct Leaf {
  vec3 pos;
  vec3 rot;
  vec3 dir;
};
Leaf leafArray[NUM_LEAFS];

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

Graphics3D g3d = createGraphics3D(display::WIDTH, display::HEIGHT);

Sprite envTexture = createSprite565(256, 128, GFX2D_STRIDE_AUTO,
                                    (void *)caveEnvTextureData, false);
Material3D envMat = createMaterial3D();
Mesh3D cube = createCube(0.5f);

int modelIndex = 0;

uint64_t lastUs = 0;
uint64_t autoRotationStartUs = 0;
float eyeYaw = 0;
float eyePitch = 0;
float eyeDistance = 5.0f;
float eyeYawSpeed = 0;
float eyePitchSpeed = 0;
float eyeDistanceSpeed = 0;

void updateScene();
void renderScene();

AppConfig xmc::appGetConfig() {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmc::appSetup() {
  // prepare leaf data
  leafColors->data[0] = {0, 0, 0, 1};
  leafColors->data[1] = {0, 0, 0, 1};
  leafColors->data[2] = {0, 1, 0, 1};
  leafMat->flags |= MaterialFlags3D::DOUBLE_SIDED;
  for (int i = 0; i < NUM_LEAFS; i++) {
    // distribute within a circle of radius R
    const float R = 7.0f;
    do {
      vec3 pos(randomF32() * 2 - 1, 0, randomF32() * 2 - 1);
      pos *= R * pow(pos.length(), 1.1f);
      leafArray[i].pos = pos;
    } while (leafArray[i].pos.length() > R);

    // Random rotation
    float a = randomF32() * 2 * M_PI;
    leafArray[i].rot = vec3(sinf(a) * 0.2f, 0, cosf(a) * 0.2f);

    // Random direction
    leafArray[i].dir = {
        randomF32() * 2 - 1,
        1.0f + powf(randomF32(), 2),
        randomF32() * 2 - 1,
    };
  }

  // prepare particle data
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

  // prepare environment map
  envMat->flags |= MaterialFlags3D::ENVIRONMENT_MAPPED;
  envMat->colorTexture = envTexture;
  cube->setMaterial(envMat);
}

void xmc::appLoop() {
  updateScene();

  frameBuffer.beginRender();
  renderScene();
  frameBuffer.endRender();
}

void updateScene() {
  uint64_t nowUs = getTimeUs();
  float dt = (lastUs != 0) ? (nowUs - lastUs) * 1e-6f : 0.0f;
  lastUs = nowUs;

  bool pressed = false;

  // camera position control
  if (isPressed(Button::LEFT)) {
    eyeYawSpeed -= dt;
    pressed = true;
  } else if (isPressed(Button::RIGHT)) {
    eyeYawSpeed += dt;
    pressed = true;
  }
  if (isPressed(Button::UP)) {
    eyePitchSpeed += dt;
    pressed = true;
  } else if (isPressed(Button::DOWN)) {
    eyePitchSpeed -= dt;
    pressed = true;
  }

  // zoom control
  if (isPressed(Button::A)) {
    eyeDistanceSpeed -= dt * 5;
    pressed = true;
  } else if (isPressed(Button::B)) {
    eyeDistanceSpeed += dt * 5;
    pressed = true;
  }

  // model switch
  if (wasPressed(Button::Y)) {
    modelIndex = (modelIndex + 1) % 2;
    pressed = true;
  }

  // if any control button is pressed, start auto rotation after 5 seconds of
  // inactivity
  if (pressed) {
    autoRotationStartUs = nowUs + 5 * 1000000;
  }
  if (nowUs > autoRotationStartUs) {
    eyeYawSpeed += dt * 0.1f;
    eyePitchSpeed += (sinf(nowUs * 2e-7f) * 0.5f - eyePitch) * dt * 0.1f;
    eyeDistanceSpeed += (5.0f - eyeDistance) * dt * 0.1f;
  }

  // apply damping to camera movement
  eyeYawSpeed *= powf(0.5f, dt);
  eyePitchSpeed *= powf(0.5f, dt);
  eyeDistanceSpeed *= powf(0.5f, dt);

  // update camera angles and distance
  eyeYaw += dt * eyeYawSpeed;
  if (eyeYaw > M_PI) {
    eyeYaw -= 2 * M_PI;
  }
  if (eyeYaw < -M_PI) {
    eyeYaw += 2 * M_PI;
  }

  eyePitch += dt * eyePitchSpeed;
  if (eyePitch > M_PI / 2 - 0.01f) {
    eyePitch = M_PI / 2 - 0.01f;
    eyePitchSpeed = 0;
  } else if (eyePitch < -M_PI / 2 + 0.01f) {
    eyePitch = -M_PI / 2 + 0.01f;
    eyePitchSpeed = 0;
  }

  eyeDistance += dt * eyeDistanceSpeed;
  if (eyeDistance < 0.5f) {
    eyeDistance = 0.5f;
    eyeDistanceSpeed = 0;
  } else if (eyeDistance > 30.0f) {
    eyeDistance = 30.0f;
    eyeDistanceSpeed = 0;
  }

  // update particles
  float pt = (float)(nowUs / 10000) * 0.01f;
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

void renderScene() {
  uint64_t nowMs = getTimeMs();

  Graphics2D gfx = frameBuffer.createGraphics();

  // clear screen
  gfx->clear(0x0000);

  // specify target frame buffer for 3D rendering
  g3d->setTarget(frameBuffer.getBackBuffer());

  // clear depth buffer
  g3d->clearDepth();

  // setup camera
  g3d->setDepthRange(0.1f, 10.0f);
  g3d->setPerspectiveProjection(
      M_PI / 4, (float)frameBuffer.getWidth() / frameBuffer.getHeight(), 0.01f,
      100.0f);
  vec3 focus = vec3(0, 3.3f, 0);
  vec3 eye = focus + vec3(eyeDistance * sinf(eyeYaw) * cosf(eyePitch),
                          eyeDistance * sinf(eyePitch),
                          eyeDistance * cosf(eyeYaw) * cosf(eyePitch));
  if (eye.y < 0.5f) {
    focus.y += 0.5f - eye.y;
    eye.y = 0.5f;
  }
  g3d->lookAt(eye, focus, vec3(0, 1, 0));

  // setup lights
  g3d->setEnvironmentLight({0.6f, 0.8f, 1.0f, 1.0f});
  g3d->setParallelLight(vec3(0.2f, 1.0f, 0.2f), {1.2f, 1.0f, 0.8f, 1});

  // reset model matrix
  g3d->loadIdentity();
  g3d->setBlendMode(BlendMode::OVERWRITE);

  if (modelIndex == 0) {
    // flower
    g3d->disableFlags(RenderFlags3D::LIGHTING);
    g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);

    g3d->renderScene(tulip);
  } else if (modelIndex == 1) {
    // mirror cubes
    g3d->disableFlags(RenderFlags3D::LIGHTING);
    g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);
    g3d->pushMatrix();
    float largeRot = (float)nowMs * 0.0002f;
    g3d->rotate(largeRot * 1.1f, largeRot * 1.2f, largeRot * 1.3f);
    g3d->translate(0, 3.3f, 0);
    for (int iz = 0; iz < 2; iz++) {
      for (int iy = 0; iy < 2; iy++) {
        for (int ix = 0; ix < 2; ix++) {
          int i = ix + iy * 2 + iz * 4;
          float t = ((float)nowMs * 0.0002f);
          int tInt = (int)floorf(t);
          int axis = tInt % 3;
          float p = (t - tInt) * 2;
          g3d->pushMatrix();
          if (tInt % 8 == i && p < 1.0f) {
            if (p < 0.5f) {
              p *= 2;
              p *= p;
              p /= 2;
            } else {
              p = (1 - p) * 2;
              p *= p;
              p = 1 - p / 2;
            }
            float rot = p * (M_PI / 2);
            switch (axis) {
              case 0: g3d->rotate(rot, 0, 0); break;
              case 1: g3d->rotate(0, rot, 0); break;
              case 2: g3d->rotate(0, 0, rot); break;
              default: break;
            }
          }
          g3d->translate((ix * 2 - 1) * 0.6f, (iy * 2 - 1) * 0.6f,
                         (iz * 2 - 1) * 0.6f);
          g3d->renderMesh(cube);
          g3d->popMatrix();
        }
      }
    }
    g3d->popMatrix();
  }

  // crystals
  g3d->enableFlags(RenderFlags3D::LIGHTING);
  g3d->enableFlags(RenderFlags3D::GOURAUD_SHADING);
  g3d->pushMatrix();
  g3d->scale(2.5f);
  g3d->translate(0, 0, -20);
  g3d->renderScene(quarts_scene0);
  g3d->popMatrix();
  g3d->pushMatrix();
  g3d->scale(3);
  g3d->translate(0, 0, -20);
  g3d->rotate(0, 90 * M_PI / 180.0f, 0);
  g3d->renderScene(quarts_scene0);
  g3d->popMatrix();
  g3d->pushMatrix();
  g3d->scale(2);
  g3d->translate(0, 0, -20);
  g3d->rotate(0, 180 * M_PI / 180.0f, 0);
  g3d->renderScene(quarts_scene0);
  g3d->popMatrix();
  g3d->pushMatrix();
  g3d->scale(3.5f);
  g3d->translate(0, 0, -20);
  g3d->rotate(0, 270 * M_PI / 180.0f, 0);
  g3d->renderScene(quarts_scene0);
  g3d->popMatrix();

  // leafs
  g3d->disableFlags(RenderFlags3D::LIGHTING);
  g3d->enableFlags(RenderFlags3D::GOURAUD_SHADING);
  for (int i = 0; i < NUM_LEAFS; i++) {
    leafVerts->data[0] = leafArray[i].pos - leafArray[i].rot;
    leafVerts->data[1] = leafArray[i].pos + leafArray[i].rot;
    leafVerts->data[2] = leafArray[i].pos + leafArray[i].dir;
    g3d->renderMesh(leafMesh);
  }

  // cave hole and light
  g3d->disableFlags(RenderFlags3D::LIGHTING);
  g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);
  g3d->pushMatrix();
  g3d->scale(4);
  g3d->renderScene(cave_hole);
  g3d->enableFlags(RenderFlags3D::GOURAUD_SHADING);
  g3d->setBlendMode(BlendMode::ADD);
  g3d->renderScene(cave_light);
  g3d->setBlendMode(BlendMode::OVERWRITE);
  g3d->popMatrix();

  // particles
  g3d->disableFlags(RenderFlags3D::LIGHTING);
  g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);
  g3d->renderMesh(particleMesh);
  g3d->popMatrix();
}
