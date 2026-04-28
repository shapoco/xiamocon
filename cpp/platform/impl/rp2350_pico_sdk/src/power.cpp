#include "xmc/power.hpp"
#include "xmc/i2c.hpp"
#include "xmc/pins.hpp"

#include <hardware/clocks.h>
#include <hardware/powman.h>
#include <hardware/vreg.h>
#include <hardware/watchdog.h>
#include <pico/multicore.h>
#include <pico/sleep.h>
#include <pico/stdio_usb.h>
#include <pico/stdlib.h>

namespace xmc::power {

static const int XIAO_PIN_BAT_ADC_EN = 19;
static const int XIAO_PIN_NEO_PWR = 23;
static const int XIAO_PIN_LED_Y = 25;

XmcStatus init() { return XMC_OK; }

XmcStatus service() { return XMC_OK; }

XmcStatus deepSleep() {
  multicore_reset_core1();

  stdio_flush();
  stdio_usb_deinit();

  gpio_set_pulls(PICO_DEFAULT_UART_TX_PIN, false, false);
  gpio_set_pulls(PICO_DEFAULT_UART_RX_PIN, false, false);
  gpio_deinit(PICO_DEFAULT_UART_TX_PIN);
  gpio_deinit(PICO_DEFAULT_UART_RX_PIN);

  gpio_deinit(XIAO_PIN_LED_Y);
  gpio_pull_up(XIAO_PIN_LED_Y);

  gpio_deinit(XIAO_PIN_BAT_ADC_EN);
  gpio_pull_down(XIAO_PIN_BAT_ADC_EN);

  gpio_deinit(XIAO_PIN_NEO_PWR);
  gpio_pull_down(XIAO_PIN_NEO_PWR);

  gpio_init(XMC_PIN_POWER_BUTTON);
  gpio_set_dir(XMC_PIN_POWER_BUTTON, false);
  powman_enable_gpio_wakeup(0, XMC_PIN_POWER_BUTTON, false, true);

  powman_set_debug_power_request_ignored(true);

  powman_power_state P1_7 = POWMAN_POWER_STATE_NONE;
  powman_power_state P0_3 = POWMAN_POWER_STATE_NONE;
  P0_3 = powman_power_state_with_domain_on(P0_3,
                                           POWMAN_POWER_DOMAIN_SWITCHED_CORE);
  P0_3 = powman_power_state_with_domain_on(P0_3, POWMAN_POWER_DOMAIN_XIP_CACHE);
  powman_power_state off_state = P1_7;
  powman_power_state on_state = P0_3;

  powman_configure_wakeup_state(off_state, on_state);

  // reboot to main
  powman_hw->boot[0] = 0;
  powman_hw->boot[1] = 0;
  powman_hw->boot[2] = 0;
  powman_hw->boot[3] = 0;

  // Switch to required power state
  powman_set_power_state(off_state);

  // Power down
  while (true) __wfi();

  return XMC_OK;
}

XmcStatus reset(ResetMode mode) {
  watchdog_reboot(0, 0, 0);
  while (true) {
    tight_loop_contents();
  }
  return XMC_ERR_POWER_RESET_FAILED;
}

}  // namespace xmc::power
