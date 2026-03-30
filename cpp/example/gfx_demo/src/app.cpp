#include "xiamocon.hpp"

#include <stdio.h>

#include "tulip.hpp"

namespace xmc {

Sprite frameBuffers[] = {
    createSprite565(display::WIDTH, display::HEIGHT),
    createSprite565(display::WIDTH, display::HEIGHT),
};
int backIndex = 0;
uint64_t nextVsyncTimeUs = 0;

Mesh3D cube = createColoredCube();
Mesh3D tulip_mesh0 = tulip_mesh0_create();
Mesh3D tulip_mesh1 = tulip_mesh1_create();
Mesh3D tulip_mesh2 = tulip_mesh2_create();
Mesh3D tulip_mesh3 = tulip_mesh3_create();
Mesh3D tulip_mesh4 = tulip_mesh4_create();

Rasterizer rasterizer = createRasterizer(display::WIDTH, display::HEIGHT);

float yaw = 0, pitch = 0;
float vyaw = 0, vpitch = 0;

void waitVsync();
void updateScene();
void renderScene();

AppConfig appGetConfig() {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB565;
  return cfg;
}

void appSetup() {}

void appLoop() {
  int frontIndex = (backIndex + 1) % 2;

  updateScene();
  renderScene();

  // render status bar
  appDrawStatusBar(frameBuffers[backIndex]);
  appDrawDebugInfo(frameBuffers[backIndex]);

  frameBuffers[frontIndex]->completeTransfer();
  waitVsync();
  frameBuffers[backIndex]->startTransferToDisplay(0, 0);

  backIndex = frontIndex;
}

void updateScene() {
  // vyaw += 0.01f;
  // vpitch += 0.005f;
  // yaw += vyaw;
  // pitch += vpitch;
  yaw += 0.01f;
  pitch += 0.01f;
}

void renderScene() {
  Sprite &screen = frameBuffers[backIndex];
  screen->clear(0xFFFF);

  mat4 worldTransform =
      mat4::lookAt(vec3(0, 0, -3), vec3(0, 0, 0), vec3(0, 1, 0));
  rasterizer->setTarget(screen);
  rasterizer->clearDepth();
  // rasterizer->setDepthRange(-1.0f, 1.0f);

  rasterizer->setProjectionPerspective(
      M_PI / 4, (float)screen->width / screen->height, 0.01f, 100.0f);
  // rasterizer->setProjectionOrtho(-1, 1, -1, 1);
  //   rasterizer->setParallelLight(vec3(0.5f, 0.5f, 1.0f),
  //                                {0.8f, 0.8f, 0.8f, 1.0f});

  rasterizer->loadIdentity();
  rasterizer->pushMatrix();
  // rasterizer->loadMatrix(worldTransform);
  // rasterizer->scale(10);
  // rasterizer->rotate(0, M_PI / 2, 0);
  rasterizer->rotate(pitch, 0, yaw);
  rasterizer->translate(0, 0, -(float)(getTimeMs() % 3000) * 0.01f);
  // rasterizer->renderMesh(cube);
  rasterizer->renderMesh(tulip_mesh0);
  rasterizer->renderMesh(tulip_mesh1);
  rasterizer->renderMesh(tulip_mesh2);
  rasterizer->renderMesh(tulip_mesh3);
  rasterizer->renderMesh(tulip_mesh4);
  rasterizer->popMatrix();
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
