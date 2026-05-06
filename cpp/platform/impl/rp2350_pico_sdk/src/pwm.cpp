#include "xmc/pwm.hpp"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
#include <stdlib.h>

namespace xmc::pwm {

struct PwmHwRp {
  uint sliceNum;
  uint channel;
};

PwmConfig getDefaultPwmConfig() {
  PwmConfig cfg;
  cfg.freqHz = 1000;
  cfg.period = 256;
  return cfg;
}

PwmDriverClass::PwmDriverClass(int pin) : pin(pin) { handle = (PwmHwRp *)malloc(sizeof(PwmHwRp)); }

PwmDriverClass::~PwmDriverClass() {
  if (handle) {
    free(handle);
    handle = nullptr;
  }
}

XmcStatus PwmDriverClass::start(const PwmConfig &cfg, float *actualFreqHz) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHwRp *hw = (PwmHwRp *)handle;

  uint32_t periClkFreq = clock_get_hz(clk_peri);
  uint sliceNum = pwm_gpio_to_slice_num(pin);
  pwm_config pc = pwm_get_default_config();
  pwm_config_set_clkdiv(&pc, (float)periClkFreq * cfg.period / cfg.freqHz);
  pwm_config_set_wrap(&pc, cfg.period - 1);
  pwm_init(sliceNum, &pc, true);
  gpio_set_function(pin, GPIO_FUNC_PWM);

  if (actualFreqHz) {
    pwm_slice_hw_t *sliceHw = &(pwm_hw->slice[sliceNum]);
    *actualFreqHz = (float)periClkFreq / (sliceHw->top + 1) / ((float)sliceHw->div / 16);
  }

  hw->sliceNum = sliceNum;
  hw->channel = pwm_gpio_to_channel(pin);
  return XMC_OK;
}

XmcStatus PwmDriverClass::stop() {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHwRp *hw = (PwmHwRp *)handle;
  pwm_set_enabled(hw->sliceNum, false);
  free(hw);
  handle = nullptr;
  return XMC_OK;
}

XmcStatus PwmDriverClass::write(uint32_t cycle) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHwRp *hw = (PwmHwRp *)handle;
#if 0
  if (cycle > pwm_hw->slice[sliceNum].top) {
    return XMC_ERR_PWM_INVALID_DUTY_CYCLE;
  }
#endif
  pwm_set_chan_level(hw->sliceNum, PWM_CHAN_A, cycle);
  return XMC_OK;
}

}  // namespace xmc::pwm
