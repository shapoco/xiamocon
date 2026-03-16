#include "xmc/app.h"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/speaker.h"
#include "xmc/system.h"

#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <pico/stdlib.h>

static const uint32_t SYS_CLK_FREQ = 250000000;

int main() {
#if 1
  vreg_set_voltage(VREG_VOLTAGE_1_30);
  sleep_ms(100);
  set_sys_clock_khz(SYS_CLK_FREQ / 1000, true);
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                  SYS_CLK_FREQ, SYS_CLK_FREQ);
#endif
  xmc_sys_init();

  xmc_app_config_t cfg = xmc_app_get_config();
  xmc_speaker_init(cfg.speaker_sample_format, cfg.speaker_sample_rate_hz,
                   cfg.speaker_latency_samples, NULL);
  xmc_app_setup();
  while (1) {
    xmc_sys_service();
    xmc_app_loop();
  }
  return 0;
}
