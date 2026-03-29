#include "xmc/hw/power.hpp"
#include "xmc/hw/i2c.hpp"
#include "xmc/hw/pins.hpp"

#include <Arduino.h>
#include <driver/rtc_io.h>

namespace xmc::power {

XmcStatus init() { return XMC_OK; }

XmcStatus service() { return XMC_OK; }

XmcStatus deepSleep() {
  Serial.end();

  esp_sleep_enable_ext0_wakeup((gpio_num_t)XMC_PIN_POWER_BUTTON, 1);
  esp_deep_sleep_start();

  return XMC_OK;
}

XmcStatus reset(ResetMode mode) {
  esp_restart();
  return XMC_ERR_POWER_RESET_FAILED;
}

}  // namespace xmc::power
