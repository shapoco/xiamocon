#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

PixelFormat displayFormat = PixelFormat::RGB565;
FrameBuffer frameBuffer;
FpsKeeper fpsKeeper(60);

constexpr int DROP_SIZE = 32;
constexpr int DROP_X_INTERVAL = DROP_SIZE * 1.5f;
constexpr int DROP_Y_INTERVAL = DROP_SIZE * (1.5f * 0.866f);
constexpr int DROP_COLS = display::WIDTH / DROP_X_INTERVAL + 1;
constexpr int DROP_ROWS = (display::HEIGHT / DROP_Y_INTERVAL + 3) / 2 * 2;

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
void renderDrops(Graphics2D &gfx, uint64_t nowMs);
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

  renderDrops(gfx, nowMs);
  for (int i = 0; i < NUM_STARS; i++) {
    Star &s = stars[i];
    float angle = nowMs / 1000.0f + M_PI * 2 * i / NUM_STARS;
    DevColor color = gfx->devColorHSV(i * 360 / NUM_STARS, 255, 255);
    renderStar(gfx, s.x, s.y, STAR_SIZE * s.zInv, angle, color);
  }
}

void renderDrops(Graphics2D &gfx, uint64_t nowMs) {
  DevColor col = gfx->devColor(192, 224, 255);
  int scrollPeriodX = DROP_X_INTERVAL * DROP_COLS;
  int scrollPeriodY = DROP_Y_INTERVAL * DROP_ROWS;
  int shift = (nowMs / 50);
  for (int i = 0; i < DROP_ROWS; i++) {
    for (int j = 0; j < DROP_COLS; j++) {
      int x = j * DROP_X_INTERVAL + (i % 2) * (DROP_X_INTERVAL / 2);
      int y = i * DROP_Y_INTERVAL;
      x = (x + shift) % scrollPeriodX - DROP_SIZE / 2;
      y = (y + shift) % scrollPeriodY - DROP_SIZE / 2;
      gfx->fillEllipse(x - DROP_SIZE / 2, y - DROP_SIZE / 2, DROP_SIZE,
                       DROP_SIZE, col);
    }
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