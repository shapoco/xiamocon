#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

PixelFormat displayFormat = PixelFormat::RGB565;
FrameBuffer frameBuffer;
FpsKeeper fpsKeeper(60);

constexpr int NUM_STARS = 32;
constexpr float STAR_SIZE = 40.0f;
struct Star {
  float x;
  float y;
  float zInv;
};
Star stars[NUM_STARS];

float sinTable[360];

void updateScene();
void renderScene();
void renderStar(Graphics2D &gfx, float x, float y, float radius, float angle,
                DevColor color);

AppConfig xmcAppGetConfig(void) {
  if (isPressed(Button::X)) {
    displayFormat = PixelFormat::RGB444;
  }
  frameBuffer = createFrameBuffer(displayFormat, false);

  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = displayFormat;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);

  for (int i = 0; i < 360; i++) {
    float angle = i * M_PI / 180.0f;
    sinTable[i] = sinf(angle);
  }

  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = randomF32() * display::WIDTH;
    stars[i].y = randomF32() * display::HEIGHT;
    stars[i].zInv = 1.0f / (randomF32() * 2.0f + 1.0f);
  }
}

void xmcAppLoop(void) {
  updateScene();
  fpsKeeper.waitVsync();
  if (!fpsKeeper.isFrameSkipping()) {
    frameBuffer->beginRender();
    renderScene();
    frameBuffer->endRender();
  }
}

void updateScene() {
  for (int i = 0; i < NUM_STARS; i++) {
    Star &s = stars[i];
    float r = STAR_SIZE * s.zInv;
    float dx = -1.0f * s.zInv;
    float dy = 1.5f * s.zInv;
    s.x += dx;
    s.y += dy;
    if (s.x < -r) {
      s.x += display::WIDTH + r * 2;
    } else if (s.x >= display::WIDTH + r) {
      s.x -= display::WIDTH + r * 2;
    }
    if (s.y < -r) {
      s.y += display::HEIGHT + r * 2;
    } else if (s.y >= display::HEIGHT + r) {
      s.y -= display::HEIGHT + r * 2;
    }
  }
}

void renderScene() {
  uint64_t nowMs = getTimeMs();
  auto gfx = frameBuffer->createGraphics();
  gfx->clear(gfx->devColor(Colors::WHITE));

  for (int i = 0; i < NUM_STARS; i++) {
    Star &s = stars[i];
    float angle = nowMs / 1000.0f + M_PI * 2 * i / NUM_STARS;
    DevColor color = gfx->devColorHSV(i * 360 / NUM_STARS, 255, 255);
    renderStar(gfx, s.x, s.y, STAR_SIZE * s.zInv, angle, color);
  }
}

void renderStar(Graphics2D &gfx, float x, float y, float r, float a,
                DevColor col) {
  float aStep = M_PI * 2 / 10.0f;
  int x0, y0;
  for (int i = 0; i <= 10; i++) {
    float dist = (i % 2 == 1) ? r : (r / 2);
    float theta = i * aStep + a;
    int x1 = x + cosf(theta) * dist;
    int y1 = y + sinf(theta) * dist;
    if (i > 0) {
      gfx->drawLine(x0, y0, x1, y1, col);
    }
    x0 = x1;
    y0 = y1;
  }
}