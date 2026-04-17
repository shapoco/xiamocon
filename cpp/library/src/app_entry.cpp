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

}  // namespace xmc
