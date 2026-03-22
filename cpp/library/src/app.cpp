#include "xmc/app.hpp"
#include "xmc/app_entry.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/speaker.hpp"
#include "xmc/system.hpp"

#include <stddef.h>

namespace xmc {

uint64_t fpsLastUpdateUs = 0;
uint32_t fpsFrameCount = 0;
char fpsString[16] = {0};

AppConfig getDefaultAppConfig() {
  AppConfig config = {
      .displayPixelFormat = display::InterfaceFormat::RGB565,
      .speakerSampleFormat = audio::SampleFormat::LINEAR_PCM_S16_MONO,
      .speakerSampleRateHz = 22050,
      .speakerLatencySamples = 1024,
  };
  return config;
}

void appMain() {
  AppConfig cfg = appGetConfig();
  system::init();
  display::init(cfg.displayPixelFormat, 0);
  input::init();
  speaker::init(cfg.speakerSampleFormat, cfg.speakerSampleRateHz,
                cfg.speakerLatencySamples, NULL);
  appSetup();
  while (1) {
    system::service();
    appLoop();
  }
}

void appDrawStatusBar(Sprite &screen) {
  uint64_t nowUs = getTimeUs();

  char buf[64];
  int w = screen->width;
  int h = 10;
  int baseLine = h - 1;

  TextState prevTextState = screen->textState;

  screen->setFont(&ShapoSansP_s08c07, 1);
  screen->setTextColor(0xFFFF);
  screen->fillSmokeRect(0, 0, w, h, 0);

  if (true) {
    if (nowUs >= fpsLastUpdateUs + 1000000) {
      float fps = fpsFrameCount * 1000000.0f / ((nowUs - fpsLastUpdateUs));
      snprintf(fpsString, sizeof(fpsString), "%5.2f FPS", fps);
      fpsFrameCount = 0;
      fpsLastUpdateUs = nowUs;
    }
    screen->setCursor(1, baseLine);
    screen->drawString(fpsString);
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
    screen->drawRect(w - 50, 1, 15, h - 2, 0xFFFF);
    screen->fillRect(w - 34, 4, 2, h - 7, 0xFFFF);
    screen->fillRect(w - 48, 3, batteryGuageWidth, h - 5, 0xFFFF);

    snprintf(buf, sizeof(buf), "%4.2fV", batMv / 1000.0f);
    screen->setCursor(w - 30, baseLine);
    screen->drawString(buf);
  }

  screen->textState = prevTextState;
}

void appDrawDebugInfo(Sprite &screen) {
  XmcStatus err;
  const char *file;
  int line;
  xmcGetLastError(&err, &file, &line);
  if (err == XMC_OK) return;

  int w = screen->width;
  int h = 10;

  char buf[64];
  TextState prevTextState = screen->textState;
  screen->fillSmokeRect(0, screen->height - h, w, h);
  screen->setFont(&ShapoSansP_s08c07, 1);
  snprintf(buf, sizeof(buf), "ERR 0x%X: L%d in %s", err, line, file);
  screen->setCursor(0, screen->height - 2);
  screen->setTextColor(0xFFFF);
  screen->drawString(buf);
  screen->textState = prevTextState;
}

}  // namespace xmc
