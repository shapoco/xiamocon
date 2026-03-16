#include "xmc/system.h"
#include "xmc/display.h"
#include "xmc/hw/gpio.h"
#include "xmc/hw/i2c.h"
#include "xmc/hw/pins.h"
#include "xmc/hw/power.h"
#include "xmc/hw/spi.h"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/ioex.h"
#include "xmc/battery.h"

xmc_status_t xmc_sys_init() {
  xmc_gpio_set_dir(XMC_PIN_POWER_BUTTON, false);

  xmc_i2c_init();
  xmc_spi_init();
  
  xmc_ioex_init();
  xmc_ioex_set_dir_masked(0, 0xFF, 0xFF);
  xmc_ioex_set_dir_masked(1, 0xFF, 0xFF);

  xmc_battery_init();

  // Mute speaker during initialization to avoid noise
  xmc_ioex_write(XMC_IOEX_PIN_PERI_EN, true);
  xmc_ioex_set_dir(XMC_IOEX_PIN_SPEAKER_MUTE, true);

  // Reset LCD
  xmc_ioex_write(XMC_IOEX_PIN_DISPLAY_RESET, false);
  xmc_ioex_set_dir(XMC_IOEX_PIN_DISPLAY_RESET, true);

  // Power on peripherals
  xmc_ioex_write(XMC_IOEX_PIN_PERI_EN, true);
  xmc_ioex_set_dir(XMC_IOEX_PIN_PERI_EN, true);
  xmc_sleep_ms(100);
  xmc_ioex_write(XMC_IOEX_PIN_PERI_EN, false);
  xmc_sleep_ms(100);

  xmc_display_init(XMC_DISP_INTF_FORMAT_RGB444, 0);

  xmc_input_init();

  return XMC_OK;
}

xmc_status_t xmc_sys_service() {
  xmc_battery_service();
  xmc_input_service();
  return XMC_OK;
}

xmc_status_t xmc_sys_request_shutdown() {
  xmc_display_deinit();
  xmc_input_deinit();
  xmc_battery_deinit();
  xmc_ioex_set_dir(XMC_IOEX_PIN_DISPLAY_RESET, false);
  xmc_ioex_set_dir(XMC_IOEX_PIN_SPEAKER_MUTE, false);
  xmc_ioex_write(XMC_IOEX_PIN_PERI_EN, true);
  xmc_ioex_deinit();
  xmc_i2c_deinit();
  xmc_spi_deinit();
  XMC_ERR_RET(xmc_power_deep_sleep());
  XMC_ERR_RET(xmc_power_reset(XMC_RESET_MODE_NORMAL));
  return XMC_OK;
}
