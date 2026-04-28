/**
 * @file app.h
 * @brief User application interface
 */

#ifndef XMC_APP_H
#define XMC_APP_H

#include "xmc/audio_common.hpp"
#include "xmc/display.hpp"
#include "xmc/gfx.hpp"
#include "xmc/system.hpp"
#include "xmc/xmc_common.hpp"

namespace xmc {

struct AppConfig {
  PixelFormat displayPixelFormat;
  bool speakerEnabled;
  audio::SampleFormat speakerSampleFormat;
  uint32_t speakerSampleRateHz;
  uint32_t speakerLatencySamples;
};

/**
 * Get the default application configuration.
 * @return A default application configuration struct.
 */
AppConfig getDefaultAppConfig(void);

}  // namespace xmc

/**
 * Get application configuration parameters. This function will be called before
 * xmcAppSetup, and the returned configuration will be used to initialize the
 * application. You can use this function to specify parameters such as speaker
 * sample rate, display resolution, etc.
 */
xmc::AppConfig xmcAppGetConfig(void);

/**
 * User defined setup function. This will be called once at the beginning of the
 * program. You can use this function to initialize your application, set up
 * peripherals, etc.
 */
void xmcAppSetup(void);

/**
 * User defined loop function. This will be called repeatedly after
 * xmcAppSetup. You can use this function to implement the main logic of
 * your application.
 */
void xmcAppLoop(void);

/**
 * User defined termination function. This will be called when the application
 * is terminating. You can use this function to clean up resources, save state,
 * etc.
 * @param reason The reason for the application termination, which can be used
 * to determine the appropriate cleanup actions.
 * @return Return XMC_OK to approve the shutdown. If a value other than XMC_OK
 * is returned, the shutdown will be canceled.
 */
XmcStatus xmcAppTerminate(xmc::system::ShutdownReason reason);

#endif
