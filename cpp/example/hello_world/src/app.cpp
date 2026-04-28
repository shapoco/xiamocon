#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

static constexpr PixelFormat DISPLAY_FORMAT =
    PixelFormat::RGB565;

FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, false);
FpsKeeper fpsKeeper(30);

void drawButtonState(Graphics2D &gfx, Button btn, int x, int y);

AppConfig xmcAppGetConfig(void) {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
}

void xmcAppLoop(void) {
  fpsKeeper.waitVsync();
  if (!fpsKeeper.isFrameSkipping()) {
    frameBuffer->beginRender();
    Graphics2D gfx = frameBuffer->createGraphics();
    gfx->clear(0xFFFF);
    gfx->setTextColor(0x0000, 0xFFFF);
    gfx->setCursor(10, 40);
    gfx->setFont(&ShapoSansP_s08c07, 2);
    gfx->drawString("Hello, Xiamocon!");
    int xl = 50;
    int xr = 170;
    int y = 120;
    drawButtonState(gfx, Button::LEFT, xl - 30, y);
    drawButtonState(gfx, Button::UP, xl, y - 30);
    drawButtonState(gfx, Button::RIGHT, xl + 30, y);
    drawButtonState(gfx, Button::DOWN, xl, y + 30);
    drawButtonState(gfx, Button::Y, xr - 30, y);
    drawButtonState(gfx, Button::X, xr, y - 30);
    drawButtonState(gfx, Button::A, xr + 30, y);
    drawButtonState(gfx, Button::B, xr, y + 30);
    drawButtonState(gfx, Button::FUNC, xr + 30, y - 60);
    frameBuffer->endRender();
  }
}

void drawButtonState(Graphics2D &gfx, Button btn, int x, int y) {
  if (isPressed(btn)) {
    gfx->fillRect(x, y, 20, 20, color888To565(0xFF0000));
  } else {
    gfx->fillRect(x, y, 20, 20, color888To565(0x00FFFF));
  }
  gfx->drawRect(x, y, 20, 20, 0x0000);
}
