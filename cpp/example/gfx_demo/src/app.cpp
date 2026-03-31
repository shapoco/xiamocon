#include "xiamocon.hpp"

#include <stdio.h>

#include "tulip.hpp"

#define DOUBLE_BUFFER (1)

namespace xmc {

Sprite frameBuffers[] = {
    createSprite565(display::WIDTH, display::HEIGHT),
#if DOUBLE_BUFFER
    createSprite565(display::WIDTH, display::HEIGHT),
#endif
};
#if DOUBLE_BUFFER
int backIndex = 0;
#else
const int backIndex = 0;
#endif

uint64_t nextVsyncTimeUs = 0;

Scene3D tulip = tulip_scene0_create();

Vec3Buffer weedPoses = createVec3Buffer(3);
ColorBuffer weedColors = createColorBuffer(3);
Material3D weedMat = createMaterial3D();
Primitive3D weedPrim =
    createPrimitive3D(PrimitiveMode::TRIANGLES, weedPoses, nullptr, weedColors,
                      nullptr, nullptr, weedMat);
Mesh3D weed = createMesh3D({weedPrim});

static constexpr int NUM_WEEDS = 1000;
struct WeedInstance {
  vec3 pos;
  vec3 rot;
  vec3 dir;
};
WeedInstance weedArray[NUM_WEEDS];

Rasterizer rasterizer = createRasterizer(display::WIDTH, display::HEIGHT);

float eyeYaw = 0;
float eyePitch = 0;
float eyeDistance = 5.0f;

void waitVsync();
void updateScene();
void renderScene();

AppConfig appGetConfig() {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB565;
  return cfg;
}

void appSetup() {
  weedPoses->data[0] = vec3(-0.2f, 0, 0);
  weedPoses->data[1] = vec3(0.2f, 0, 0);
  weedPoses->data[2] = vec3(0, 1.5f, 0);
  weedColors->data[0] = {0, 0, 0, 1};
  weedColors->data[1] = {0, 0, 0, 1};
  weedColors->data[2] = {0, 1, 0, 1};
  weedMat->doubleSided = true;
  for (int i = 0; i < NUM_WEEDS; i++) {
    const float r = 10.0f;
    do {
      weedArray[i].pos = vec3(r * (float)(rand() % 1000) / 500.0f - r, 0,
                              r * (float)(rand() % 1000) / 500.0f - r);
    } while (weedArray[i].pos.length() > r);
    float a = (float)(rand() & 0xFFFF) / 32768.0f * M_PI;
    weedArray[i].rot = vec3(sinf(a) * 0.2f, 0, cosf(a) * 0.2f);
    weedArray[i].dir = {
        (float)((rand() & 0xFF) - 0x80) / 128.0f,
        1.0f + (float)(rand() & 0xFF) / 512.0f,
        (float)((rand() & 0xFF) - 0x80) / 128.0f,
    };
  }
}

void appLoop() {
#if DOUBLE_BUFFER
  int frontIndex = (backIndex + 1) % 2;
#endif

  updateScene();

#if !DOUBLE_BUFFER
  frameBuffers[0]->completeTransfer();
#endif

  renderScene();

  // render status bar

  appDrawStatusBar(frameBuffers[backIndex]);
  appDrawDebugInfo(frameBuffers[backIndex]);

#if DOUBLE_BUFFER
  frameBuffers[frontIndex]->completeTransfer();
#endif
  waitVsync();
  frameBuffers[backIndex]->startTransferToDisplay(0, 0);

#if DOUBLE_BUFFER
  backIndex = frontIndex;
#endif
}

void updateScene() {
  // vyaw += 0.01f;
  // vpitch += 0.005f;
  // yaw += vyaw;
  // pitch += vpitch;
  eyeYaw += 0.01f;
  // eyePitch = sinf((float)getTimeMs() * 0.0005f) * 1.0f;
  // eyePitch = -10.0f * M_PI / 180;
  eyePitch = 10.0f * M_PI / 180;
  // eyeDistance = 5.0f + sinf((float)getTimeMs() * 0.0002f) * 3.0f;
  // eyeDistance = 10.0f;
  eyeDistance = 5.0f;
}

void renderScene() {
  Sprite &screen = frameBuffers[backIndex];
  screen->clear(0x0000);
  // screen->clear(rgb565(0, 32, 31));

  rasterizer->setTarget(screen);
  rasterizer->clearDepth();
  // rasterizer->setDepthRange(-1.0f, 1.0f);

  rasterizer->setEnvironmentLight({0.6f, 0.8f, 1.0f, 1.0f});
  rasterizer->setParallelLight(vec3(0.2f, 1.0f, 0.2f), {1.2f, 1.0f, 0.8f, 1});
  rasterizer->setPerspectiveProjection(
      M_PI / 4, (float)screen->width / screen->height, 0.01f, 100.0f);
  // rasterizer->setOrthoProjection(-1, 1, -1, 1);
  //    rasterizer->setParallelLight(vec3(0.5f, 0.5f, 1.0f),
  //                                 {0.8f, 0.8f, 0.8f, 1.0f});

  vec3 focus = vec3(0, 3.3f, 0);
  vec3 eye = focus + vec3(eyeDistance * sinf(eyeYaw) * cosf(eyePitch),
                          eyeDistance * sinf(eyePitch),
                          eyeDistance * cosf(eyeYaw) * cosf(eyePitch));
  rasterizer->lookAt(eye, focus, vec3(0, 1, 0));

  rasterizer->loadIdentity();
  // rasterizer->pushMatrix();
  //  rasterizer->scale(10);
  //  rasterizer->rotate(0, M_PI / 2, 0);

  // rasterizer->renderMesh(cube);
  rasterizer->renderScene(tulip);

  for (int i = 0; i < NUM_WEEDS; i++) {
    weedPoses->data[0] = weedArray[i].pos - weedArray[i].rot;
    weedPoses->data[1] = weedArray[i].pos + weedArray[i].rot;
    weedPoses->data[2] = weedArray[i].pos + weedArray[i].dir;
    rasterizer->renderMesh(weed);
  }
  // rasterizer->popMatrix();
}

void waitVsync() {
  uint64_t nowUs = getTimeUs();
  if (nowUs < nextVsyncTimeUs) {
    sleepUs(nextVsyncTimeUs - nowUs);
  }
  nextVsyncTimeUs += 1000000 / 60;
  if (nextVsyncTimeUs < nowUs) {
    nextVsyncTimeUs = nowUs + 1000000 / 60;
  }
}

}  // namespace xmc
