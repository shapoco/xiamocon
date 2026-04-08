#include "xmc/app_entry.hpp"
#include "xmc/app.hpp"
#include "xmc/diagnostic.hpp"
#include "xmc/fs.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/speaker.hpp"
#include "xmc/system.hpp"

#include <stddef.h>

namespace xmc {

uint64_t fpsLastUpdateUs = 0;
uint32_t fpsFrameCount = 0;
char fpsString[16] = {0};
AppConfig appConfig;
diagnostic::App *diagnosticApp = nullptr;

AppConfig getDefaultAppConfig() {
  AppConfig config = {
      .displayPixelFormat = PixelFormat::RGB565,
      .speakerEnabled = true,
      .speakerSampleFormat = audio::SampleFormat::LINEAR_PCM_S16_MONO,
      .speakerSampleRateHz = audio::getPreferredSamplingRate(),
      .speakerLatencySamples = 1024,
  };
  return config;
}

void libSetup() {
  system::init();
  input::init();
  fs::init();

  input::service();
  input::service();
  if (input::isPressed(input::Button::FUNC)) {
    diagnosticApp = new diagnostic::App();
    appConfig = diagnosticApp->getConfig();
  } else {
    appConfig = appGetConfig();
  }

  display::init(appConfig.displayPixelFormat, 0);
  if (appConfig.speakerEnabled) {
    speaker::init(appConfig.speakerSampleFormat, appConfig.speakerSampleRateHz,
                  appConfig.speakerLatencySamples, NULL);
  }

  if (diagnosticApp) {
    diagnosticApp->setup();
  } else {
    appSetup();
  }
}

void libLoop() {
  system::service();
  if (appConfig.speakerEnabled) {
    speaker::service();
  }
  if (diagnosticApp) {
    diagnosticApp->loop();
  } else {
    appLoop();
  }
}

void appDrawStatusBar(Graphics2D &gfx) {
  uint64_t nowUs = getTimeUs();

  char buf[64];
  int w = gfx->getBounds().width;
  int h = 10;
  int baseLine = h - 1;

  GraphicsState2D backup = gfx->getState();

  gfx->setFont(&ShapoSansP_s08c07, 1);
  gfx->setTextColor(0xFFFF);
  gfx->fillSmokeRect(0, 0, w, h, false);

  if (true) {
    if (nowUs >= fpsLastUpdateUs + 1000000) {
      float fps = fpsFrameCount * 1000000.0f / ((nowUs - fpsLastUpdateUs));
      snprintf(fpsString, sizeof(fpsString), "%5.2f FPS", fps);
      fpsFrameCount = 0;
      fpsLastUpdateUs = nowUs;
    }
    gfx->setCursor(1, baseLine);
    gfx->drawString(fpsString);
    fpsFrameCount++;
  }

  if (true) {
    uint16_t batMv = battery::getVoltageMv();
    uint16_t batMin = 3300;
    uint16_t batMax = 4200;
    int batteryGuageWidth = 12 * (batMv - batMin) / (batMax - batMin);
    if (batteryGuageWidth < 0) {
      batteryGuageWidth = 0;
    } else if (batteryGuageWidth > 12) {
      batteryGuageWidth = 12;
    }
    gfx->drawRect(w - 50, 1, 15, h - 2, 0xFFFF);
    gfx->fillRect(w - 34, 4, 2, h - 7, 0xFFFF);
    gfx->fillRect(w - 48, 3, batteryGuageWidth, h - 5, 0xFFFF);

    snprintf(buf, sizeof(buf), "%4.2fV", batMv / 1000.0f);
    gfx->setCursor(w - 30, baseLine);
    gfx->drawString(buf);
  }

  gfx->setState(backup);
}

void appDrawDebugInfo(Graphics2D &gfx) {
  XmcStatus err;
  const char *file;
  int line;
  xmcGetLastError(&err, &file, &line);
  if (err == XMC_OK) return;

  int w = gfx->getBounds().width;
  int h = 10;

  char buf[64];
  GraphicsState2D backup = gfx->getState();
  gfx->fillSmokeRect(0, gfx->getBounds().height - h, w, h, false);
  gfx->setFont(&ShapoSansP_s08c07, 1);
  snprintf(buf, sizeof(buf), "ERR 0x%X: L%d in %s", err, line, file);
  gfx->setCursor(0, gfx->getBounds().height - 2);
  gfx->setTextColor(0xFFFF);
  gfx->drawString(buf);
  gfx->setState(backup);
}

}  // namespace xmc
