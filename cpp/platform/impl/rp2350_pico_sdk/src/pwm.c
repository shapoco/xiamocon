#include "xmc/hw/pwm.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
#include <stdlib.h>

typedef struct {
  uint slice_num;
  uint channel;
} xmc_pwm_hw_t;

xmc_status_t xmc_pwm_init(xmc_pwm_inst_t *inst, const xmc_pwm_config_t *cfg,
                          float *actual_freq_hz) {
  uint32_t peri_clk_freq = clock_get_hz(clk_peri);
  uint slice_num = pwm_gpio_to_slice_num(cfg->pin);
  pwm_config p_config = pwm_get_default_config();
  pwm_config_set_clkdiv(&p_config,
                        (float)peri_clk_freq / cfg->freq_hz / cfg->period);
  pwm_init(slice_num, &p_config, true);
  gpio_set_function(cfg->pin, GPIO_FUNC_PWM);

  if (actual_freq_hz) {
    pwm_slice_hw_t *hw = &(pwm_hw->slice[slice_num]);
    *actual_freq_hz =
        (float)peri_clk_freq / (hw->top + 1) / ((float)hw->div / 16);
  }

  xmc_pwm_hw_t *hw = malloc(sizeof(xmc_pwm_hw_t));
  if (!hw) {
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  inst->handle = hw;
  hw->slice_num = slice_num;
  hw->channel = pwm_gpio_to_channel(cfg->pin);
  return XMC_OK;
}

xmc_status_t xmc_pwm_deinit(xmc_pwm_inst_t *inst) {
  xmc_pwm_hw_t *hw = inst->handle;
  pwm_set_enabled(hw->slice_num, false);
  free(hw);
  inst->handle = NULL;
  return XMC_OK;
}

xmc_status_t xmc_pwm_set_duty_cycle(xmc_pwm_inst_t *inst, uint32_t cycle) {
  xmc_pwm_hw_t *hw = inst->handle;
#if 0
  if (cycle > pwm_hw->slice[slice_num].top) {
    return XMC_ERR_PWM_INVALID_DUTY_CYCLE;
  }
#endif
  pwm_set_chan_level(hw->slice_num, PWM_CHAN_A, cycle);
  return XMC_OK;
}
