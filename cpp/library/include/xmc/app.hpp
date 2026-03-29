/**
 * @file app.h
 * @brief User application interface
 */

#ifndef XMC_APP_H
#define XMC_APP_H

#include "xmc/audio_common.hpp"
#include "xmc/display.hpp"
#include "xmc/gfx.hpp"
#include "xmc/xmc_common.hpp"

namespace xmc {

struct AppConfig {
  display::InterfaceFormat displayPixelFormat;
  bool speakerEnabled;
  audio::SampleFormat speakerSampleFormat;
  uint32_t speakerSampleRateHz;
  uint32_t speakerLatencySamples;
};

/**
 * Get the default application configuration.
 * @return A default application configuration struct.
 */
AppConfig getDefaultAppConfig();

/**
 * Get application configuration parameters. This function will be called before
 * appSetup, and the returned configuration will be used to initialize the
 * application. You can use this function to specify parameters such as speaker
 * sample rate, display resolution, etc.
 */
AppConfig appGetConfig();

/**
 * User defined setup function. This will be called once at the beginning of the
 * program. You can use this function to initialize your application, set up
 * peripherals, etc.
 */
void appSetup();

/**
 * User defined loop function. This will be called repeatedly after
 * appSetup. You can use this function to implement the main logic of
 * your application.
 */
void appLoop();

/**
 * Draw the status bar to the given screen.
 */
void appDrawStatusBar(Sprite &screen);

/**
 * Draw the last error information to the given screen.
 */
void appDrawDebugInfo(Sprite &screen);

}  // namespace xmc

#endif
