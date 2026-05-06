#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;
FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, false);
FpsKeeper fpsKeeper(30);
adc::AdcDriver adcDriver = adc::createAdcDriver();
LineGraph lineGraph = createLineGraph(display::WIDTH);

void renderMeter(Graphics2D &gfx, float adcVoltage);
void renderGraph(Graphics2D &gfx);

AppConfig xmcAppGetConfig(void) {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
  adcDriver->init(adc::getDefaultAdcConfig());

  uint16_t maxRaw;
  adcDriver->getMaxValue(&maxRaw, nullptr);
  lineGraph->setYRange(0, maxRaw);
}

void xmcAppLoop(void) {
  uint16_t adcRawValue;
  float adcVoltage;
  adcDriver->readRaw(&adcRawValue);
  adcVoltage = adcDriver->rawToVoltage(adcRawValue);
  lineGraph->pushValue(adcRawValue);

  fpsKeeper.waitVsync();
  if (!fpsKeeper.isFrameSkipping()) {
    frameBuffer->beginRender();
    auto gfx = frameBuffer->createGraphics();
    gfx->clear(gfx->devColor(Colors::BLACK));

    renderMeter(gfx, adcVoltage);
    renderGraph(gfx);

    frameBuffer->endRender();
  }
}

void renderMeter(Graphics2D &gfx, float voltage) {
  int yTop = STATUS_BAR_HEIGHT;
  int yBottom = display::HEIGHT / 2;
  int height = yBottom - yTop;
  float angleMin = -150.0f * M_PI / 180.0f;
  float angleMax = -30.0f * M_PI / 180.0f;
  float length = height * 0.9f;

  const int ARC_VERTS = 32;
  for (int i = 0; i <= ARC_VERTS; i++) {
    float t = i / (float)(ARC_VERTS - 1);
    float angle = angleMin + t * (angleMax - angleMin);
    int x1 = display::WIDTH / 2 + cosf(angle) * length;
    int y1 = yBottom + sinf(angle) * length;
    int x2 = display::WIDTH / 2 + cosf(angle) * height;
    int y2 = yBottom + sinf(angle) * height;
    gfx->drawLine(x1, y1, x2, y2, gfx->devColor(Colors::GRAY));
  }

  {
    float angle = angleMin + (voltage / 3.3f) * (angleMax - angleMin);
    int x1 = display::WIDTH / 2;
    int y1 = yBottom;
    int x2 = x1 + cosf(angle) * length;
    int y2 = y1 + sinf(angle) * length;
    gfx->drawLine(x1, y1, x2, y2, gfx->devColor(Colors::GREEN));
  }
}

void renderGraph(Graphics2D &gfx) {
  int yTop = display::HEIGHT / 2;
  int yBottom = display::HEIGHT - STATUS_BAR_HEIGHT;
  int height = yBottom - yTop;
  lineGraph->render(gfx, 0, yTop, display::WIDTH, height,
                    gfx->devColor(Colors::GREEN));
}