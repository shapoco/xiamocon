#include <xiamocon.hpp>

#include <stdio.h>

// materials
#include "cave_hole.hpp"
#include "cave_light.hpp"
#include "quarts.hpp"
#include "tulip.hpp"

#include "cubes.hpp"
#include "flame.hpp"
#include "grass.hpp"
#include "particles.hpp"

using namespace xmc;
using namespace xmc::input;

PixelFormat displayFormat = PixelFormat::RGB565;
FrameBuffer frameBuffer;

uint64_t nextVsyncTimeUs = 0;

const float CRYSTAL_SIZE[] = {2.5f, 3.0f, 2.0f, 3.5f};
constexpr int NUM_CRYSTALS = sizeof(CRYSTAL_SIZE) / sizeof(float);

Graphics3D g3d = createGraphics3D(display::WIDTH, display::HEIGHT);

uint64_t lastMs = 0;
uint64_t autoRotationStartMs = 0;
float eyeYaw = 0;
float eyePitch = 0;
float eyeDistance = 5.0f;
float eyeYawSpeed = 0;
float eyePitchSpeed = 0;
float eyeDistanceSpeed = 0;

int parallelMode = 3;
constexpr int PARALLEL_MODE_NONE = 0;
constexpr int PARALLEL_MODE_INTERLACE = 1;
constexpr int PARALLEL_MODE_PIPELINE = 2;
constexpr int PARALLEL_MODE_AUTO = 3;
const char **parallelNames = (const char *[]){
    "NONE",
    "INTERLACE",
    "PIPELINE",
    "AUTO",
};

int testModel = 1;
constexpr int TEST_MODEL_NONE = 0;
constexpr int TEST_MODEL_FLOWER = 1;
constexpr int TEST_MODEL_CUBES = 2;
constexpr int TEST_MODEL_FLAME = 3;
const char **testModelNames = (const char *[]){
    "NONE",
    "FLOWER",
    "CUBES",
    "FLAME",
};

bool enableGrass = true;
bool enableCrystals = true;
bool enableCaveHole = true;
bool enableCaveLight = true;
bool enableParticles = true;
bool enableLighting = false;
bool enableTexture = true;
bool enableDepthTest = true;
bool enableGouraudShading = true;
bool enableAlphaBlending = true;

enum class MenuItemType {
  ENUM,
  FLOAT,
  BOOL,
};

struct MenuItem {
  MenuItemType type;
  float minValue;
  float maxValue;
  const char *name;
  void *value;
  const char **enumNames = nullptr;
};

MenuItem menuItems[] = {
    {MenuItemType::ENUM, 0, 3, "Parallel", &parallelMode, parallelNames},
    {MenuItemType::ENUM, 0, 3, "Test Model", &testModel, testModelNames},
    {MenuItemType::BOOL, 0, 1, "Grass", &enableGrass},
    {MenuItemType::BOOL, 0, 1, "Crystals", &enableCrystals},
    {MenuItemType::BOOL, 0, 1, "Cave Hole", &enableCaveHole},
    {MenuItemType::BOOL, 0, 1, "Cave Light", &enableCaveLight},
    {MenuItemType::BOOL, 0, 1, "Particles", &enableParticles},
    {MenuItemType::BOOL, 0, 1, "Lighting", &enableLighting},
    {MenuItemType::BOOL, 0, 1, "Gouraud Shading", &enableGouraudShading},
    {MenuItemType::BOOL, 0, 1, "Alpha Blending", &enableAlphaBlending},
    {MenuItemType::BOOL, 0, 1, "Texture", &enableTexture},
    {MenuItemType::BOOL, 0, 1, "Depth Test", &enableDepthTest},
    //{MenuItemType::FLOAT, 0.01f, 1.0f, "Flame Speed", &flameSpeed},
    //{MenuItemType::FLOAT, 1.0f, 100, "Flame Buoyancy", &flameBuoyancy},
    //{MenuItemType::FLOAT, 0, 1, "Flame Attraction", &flameAttraction},
    //{MenuItemType::FLOAT, 0, 1, "Flame Repulsion", &flameRepulsion},
};
constexpr int NUM_MENU_ITEMS = sizeof(menuItems) / sizeof(MenuItem);
bool menuShowing = false;
int selectedMenuItem = 0;

bool core1Loop();

void updateScene();
void updateCamera(float dt);

void renderScene();
void renderMenu(Graphics2D gfx);

AppConfig xmc::appGetConfig() {
  if (isPressed(Button::X)) {
    displayFormat = PixelFormat::RGB444;
  }

  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = displayFormat;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmc::appSetup() {
  frameBuffer = createFrameBuffer(displayFormat, true);
  setupGrass();
  setupFlame();
  setupParticles();
  setupCubes();
  startCore1(core1Loop);
}

void xmc::appLoop() {
  updateScene();

  frameBuffer->beginRender();
  renderScene();
  frameBuffer->endRender();
}

bool core1Loop() {
  g3d->serviceSubWorker();
  return true;
}

void updateScene() {
  uint64_t nowMs = getTimeMs();
  float dt = (lastMs != 0) ? (nowMs - lastMs) * 1e-3f : 0.0f;
  lastMs = nowMs;

  bool stopAutoRotation = false;

  // toggle menu
  if (wasPressed(Button::X)) {
    menuShowing = !menuShowing;
  }

  if (menuShowing) {
    // menu navigation
    if (wasPressed(Button::UP)) {
      selectedMenuItem =
          (selectedMenuItem - 1 + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
    } else if (wasPressed(Button::DOWN)) {
      selectedMenuItem = (selectedMenuItem + 1) % NUM_MENU_ITEMS;
    }

    MenuItem &item = menuItems[selectedMenuItem];
    if (item.type == MenuItemType::BOOL) {
      // toggle boolean value
      if (wasPressed(Button::LEFT) || wasPressed(Button::RIGHT)) {
        bool *value = (bool *)item.value;
        *value = !*value;
      }
    } else if (item.type == MenuItemType::ENUM) {
      // cycle through enum values
      if (wasPressed(Button::LEFT)) {
        int *value = (int *)item.value;
        *value =
            (*value - 1 + (int)item.maxValue + 1) % ((int)item.maxValue + 1);
      } else if (wasPressed(Button::RIGHT)) {
        int *value = (int *)item.value;
        *value = (*value + 1) % ((int)item.maxValue + 1);
      }
    } else {
      // adjust numeric value
      float *value = (float *)item.value;
      if (isPressed(Button::LEFT)) {
        *value *= 0.99f;
      } else if (isPressed(Button::RIGHT)) {
        *value *= 1.01f;
      }
    }
    stopAutoRotation = true;
  } else {
    // camera position control
    if (isPressed(Button::LEFT)) {
      eyeYawSpeed -= dt;
      stopAutoRotation = true;
    } else if (isPressed(Button::RIGHT)) {
      eyeYawSpeed += dt;
      stopAutoRotation = true;
    }
    if (isPressed(Button::UP)) {
      eyePitchSpeed += dt;
      stopAutoRotation = true;
    } else if (isPressed(Button::DOWN)) {
      eyePitchSpeed -= dt;
      stopAutoRotation = true;
    }
  }

  // zoom control
  if (isPressed(Button::A)) {
    eyeDistanceSpeed -= dt * 5;
    stopAutoRotation = true;
  } else if (isPressed(Button::B)) {
    eyeDistanceSpeed += dt * 5;
    stopAutoRotation = true;
  }

  // model switch
  if (wasPressed(Button::Y)) {
    testModel = (testModel + 1) % 4;
    stopAutoRotation = true;
  }

  // if any control button is pressed, start auto rotation after 5 seconds of
  // inactivity
  if (stopAutoRotation) {
    autoRotationStartMs = nowMs + 5000;
  }

  updateCamera(dt);
  updateParticles(dt);
  updateFlame(dt);
}

void updateCamera(float dt) {
  uint64_t nowMs = getTimeMs();

  // auto camera rotation
  if (nowMs > autoRotationStartMs) {
    eyeYawSpeed += dt * 0.1f;
    eyePitchSpeed += (sinf(nowMs * 2e-4f) * 0.5f - eyePitch) * dt * 0.1f;
    eyeDistanceSpeed += (5.0f - eyeDistance) * dt * 0.1f;
  }

  // apply damping to camera movement
  float damping = powf(0.5f, dt);
  eyeYawSpeed *= damping;
  eyePitchSpeed *= damping;
  eyeDistanceSpeed *= damping;

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
}

void renderScene() {
  Graphics2D gfx = frameBuffer->createGraphics();

  // clear screen
  gfx->clear(0x0000);

  // specify target frame buffer for 3D rendering
  g3d->setTarget(frameBuffer->getBackBuffer());

  // multicore rendering setup
  if (parallelMode < PARALLEL_MODE_AUTO) {
    g3d->setParallelMode((ParallelMode3D)parallelMode);
  }

  // clear state stack and depth buffer
  g3d->beginRender();
  RenderFlags3D renderFlagMask = RenderFlags3D::ALL;
  if (!enableLighting) {
    renderFlagMask &= ~RenderFlags3D::LIGHTING;
  }
  if (!enableGouraudShading) {
    renderFlagMask &= ~RenderFlags3D::GOURAUD_SHADING;
  }
  if (!enableDepthTest) {
    renderFlagMask &= ~RenderFlags3D::Z_TEST;
  }
  if (!enableTexture) {
    renderFlagMask &= ~RenderFlags3D::COLOR_TEXTURE;
  }
  g3d->setFlags(renderFlagMask);

  // setup camera
  g3d->setDepthRange(0.1f, 10.0f);
  g3d->setPerspectiveProjection(
      M_PI / 4, (float)frameBuffer->getWidth() / frameBuffer->getHeight(),
      0.01f, 100.0f);
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

  if (testModel == TEST_MODEL_FLOWER) {
    // flower
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::INTERLACE);
    }
    g3d->pushState();
    g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);
    g3d->render(tulip);
    g3d->popState();
  }

  if (testModel == TEST_MODEL_CUBES) {
    // metalic cubes
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::INTERLACE);
    }
    renderCubes(g3d);
  }

  // crystals
  if (enableCrystals) {
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::INTERLACE);
    }
    for (int i = 0; i < NUM_CRYSTALS; i++) {
      g3d->pushState();
      g3d->scale(CRYSTAL_SIZE[i]);
      g3d->translate(0, 0, -20);
      g3d->rotate(0, i * 90 * M_PI / 180.0f, 0);
      g3d->render(quarts_scene0);
      g3d->popState();
    }
  }

  // grass
  if (enableGrass) {
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::PIPELINE);
    }
    renderGrass(g3d);
  }

  // cave hole
  if (enableCaveHole) {
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::INTERLACE);
    }
    g3d->pushState();
    g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);
    g3d->scale(4);
    g3d->render(cave_hole);
    g3d->popState();
  }

  // light from hole
  if (enableCaveLight) {
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::INTERLACE);
    }
    g3d->pushState();
    if (enableAlphaBlending) {
      g3d->setBlendMode(BlendMode::ADD);
    }
    g3d->scale(4);
    g3d->render(cave_light);
    g3d->popState();
  }

  // flame
  if (testModel == TEST_MODEL_FLAME) {
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::INTERLACE);
    }
    renderFlame(g3d, enableAlphaBlending);
  }

  // particles
  if (enableParticles) {
    if (parallelMode == PARALLEL_MODE_AUTO) {
      g3d->setParallelMode(ParallelMode3D::PIPELINE);
    }
    renderParticles(g3d);
  }

  g3d->endRender();

  if (menuShowing) {
    renderMenu(gfx);
  }
}

void renderMenu(Graphics2D gfx) {
  DevColor white = gfx->devColor(Colors::WHITE);
  DevColor yellow = gfx->devColor(Colors::YELLOW);
  gfx->setFont(&ShapoSansP_s08c07);
  gfx->setFontSize(1);
  char buf[64];
  for (int i = 0; i < NUM_MENU_ITEMS; i++) {
    MenuItem &item = menuItems[i];
    int y = STATUS_BAR_HEIGHT + 10 + i * 13;
    gfx->setTextColor(i == selectedMenuItem ? yellow : white);
    gfx->setCursor(10, y);
    gfx->drawString(item.name);
    bool changing = isPressed(Button::LEFT) || isPressed(Button::RIGHT);
    if (changing && i == selectedMenuItem) {
      gfx->fillRect(95, y - 1, 64, 10, yellow);
      gfx->setTextColor(0x0000);
    }
    if (item.type == MenuItemType::BOOL) {
      snprintf(buf, sizeof(buf), "%s", *(bool *)item.value ? "ON" : "OFF");
    } else if (item.type == MenuItemType::ENUM) {
      snprintf(buf, sizeof(buf), "%s", item.enumNames[*(int *)item.value]);
    } else {
      snprintf(buf, sizeof(buf), "%.4f", *(float *)item.value);
    }
    gfx->setCursor(100, y);
    gfx->drawString(buf);
  }
}
