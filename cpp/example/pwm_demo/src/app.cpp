#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;
FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, false);
FpsKeeper fpsKeeper(30);

pwm::PwmDriver pwmDriver = pwm::createPwmDriver();
LineGraph<uint16_t> lineGraph = createLineGraph<uint16_t>(display::WIDTH);

int pwmPeriod = 256;
int pwmDutyCycle = 0;

void renderMeter(Graphics2D &gfx);
void renderGraph(Graphics2D &gfx);

AppConfig xmcAppGetConfig(void) {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
  auto pwmCfg = pwm::getDefaultPwmConfig();
  pwmPeriod = pwmCfg.period;
  pwmDriver->start(pwmCfg);
  lineGraph->setYRange(0, pwmPeriod);
}

void xmcAppLoop(void) {
  if (isPressed(Button::UP) || isPressed(Button::RIGHT)) {
    pwmDutyCycle += 4;
    if (pwmDutyCycle > 255) {
      pwmDutyCycle = 255;
    }
    pwmDriver->write(pwmDutyCycle);
  }

  if (isPressed(Button::DOWN) || isPressed(Button::LEFT)) {
    pwmDutyCycle -= 4;
    if (pwmDutyCycle < 0) {
      pwmDutyCycle = 0;
    }
    pwmDriver->write(pwmDutyCycle);
  }

  lineGraph->push(pwmDutyCycle);

  fpsKeeper.waitVsync();
  if (!fpsKeeper.isFrameSkipping()) {
    frameBuffer->beginRender();
    auto gfx = frameBuffer->createGraphics();
    gfx->clear(gfx->devColor(Colors::BLACK));

    renderMeter(gfx);
    renderGraph(gfx);

    frameBuffer->endRender();
  }
}

void renderMeter(Graphics2D &gfx) {
  int yTop = STATUS_BAR_HEIGHT;
  int yBottom = display::HEIGHT / 2;
  int height = yBottom - yTop;
  float angleMin = -150.0f * M_PI / 180.0f;
  float angleMax = -30.0f * M_PI / 180.0f;
  float length = height * 0.9f;

  const int ARC_VERTS = 32;
  for (int i = 0; i < ARC_VERTS; i++) {
    float t = i / (float)(ARC_VERTS - 1);
    float angle = angleMin + t * (angleMax - angleMin);
    int x1 = display::WIDTH / 2 + cosf(angle) * length;
    int y1 = yBottom + sinf(angle) * length;
    int x2 = display::WIDTH / 2 + cosf(angle) * height;
    int y2 = yBottom + sinf(angle) * height;
    gfx->drawLine(x1, y1, x2, y2, gfx->devColor(Colors::GRAY));
  }

  {
    float angle =
        angleMin + ((float)pwmDutyCycle / pwmPeriod) * (angleMax - angleMin);
    int x1 = display::WIDTH / 2;
    int y1 = yBottom;
    int x2 = x1 + cosf(angle) * length;
    int y2 = y1 + sinf(angle) * length;
    gfx->drawLine(x1, y1, x2, y2, gfx->devColor(Colors::GREEN));
  }

  char buf[16];
  int percent = (int)roundf((float)(pwmDutyCycle * 100) / (pwmPeriod - 1));
  gfx->setFont(&ShapoSansP_s12c09a01w02);
  gfx->setTextColor(gfx->devColor(Colors::GREEN));
  gfx->setCursor(5, yBottom - 30);
  gfx->drawString("Duty:");
  gfx->setCursor(5, yBottom - 15);
  snprintf(buf, sizeof(buf), "%1d", pwmDutyCycle);
  gfx->drawString(buf);
  gfx->setCursor(40, yBottom - 15);
  snprintf(buf, sizeof(buf), "%d%%", percent);
  gfx->drawString(buf);
}

void renderGraph(Graphics2D &gfx) {
  int yTop = display::HEIGHT / 2 + 5;
  int yBottom = display::HEIGHT - STATUS_BAR_HEIGHT - 5;
  int height = yBottom - yTop;
  gfx->fillRect(0, yTop, display::WIDTH, 1, gfx->devColor(Colors::GRAY));
  gfx->fillRect(0, yBottom, display::WIDTH, 1, gfx->devColor(Colors::GRAY));
  lineGraph->render(gfx, 0, yTop, display::WIDTH, height,
                    gfx->devColor(Colors::GREEN));
}