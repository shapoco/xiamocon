#include "xmc/pwm.hpp"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
#include <stdlib.h>

namespace xmc::pwm {

typedef struct {
  uint sliceNum;
  uint channel;
} PwmHw;

Driver::Driver(int pin) : pin(pin) { handle = (PwmHw *)malloc(sizeof(PwmHw)); }

Driver::~Driver() {
  if (handle) {
    free(handle);
    handle = nullptr;
  }
}

XmcStatus Driver::start(const Config &cfg, float *actualFreqHz) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHw *hw = (PwmHw *)handle;

  uint32_t periClkFreq = clock_get_hz(clk_peri);
  uint sliceNum = pwm_gpio_to_slice_num(pin);
  pwm_config pc = pwm_get_default_config();
  pwm_config_set_clkdiv(&pc, (float)periClkFreq / cfg.freqHz / cfg.period);
  pwm_init(sliceNum, &pc, true);
  gpio_set_function(pin, GPIO_FUNC_PWM);

  if (actualFreqHz) {
    pwm_slice_hw_t *hw = &(pwm_hw->slice[sliceNum]);
    *actualFreqHz = (float)periClkFreq / (hw->top + 1) / ((float)hw->div / 16);
  }

  hw->sliceNum = sliceNum;
  hw->channel = pwm_gpio_to_channel(pin);
  return XMC_OK;
}

XmcStatus Driver::stop() {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHw *hw = (PwmHw *)handle;
  pwm_set_enabled(hw->sliceNum, false);
  free(hw);
  handle = nullptr;
  return XMC_OK;
}

XmcStatus Driver::setDutyCycle(uint32_t cycle) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHw *hw = (PwmHw *)handle;
#if 0
  if (cycle > pwm_hw->slice[sliceNum].top) {
    return XMC_ERR_PWM_INVALID_DUTY_CYCLE;
  }
#endif
  pwm_set_chan_level(hw->sliceNum, PWM_CHAN_A, cycle);
  return XMC_OK;
}

}  // namespace xmc::pwm
