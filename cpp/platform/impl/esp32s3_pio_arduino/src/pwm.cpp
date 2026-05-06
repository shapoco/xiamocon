#include "xmc/pwm.hpp"
#include "xmc/heap.hpp"

#include <driver/ledc.h>
#include <math.h>
#include <string.h>

namespace xmc::pwm {

struct PwmHwEsp {
  ledc_timer_config_t ledc_timer;
  ledc_channel_config_t ledc_channel;
};

PwmConfig getDefaultPwmConfig() {
  PwmConfig cfg;
  cfg.freqHz = 1000;
  cfg.period = 256;
  return cfg;
}

PwmDriverClass::PwmDriverClass(int pin) : pin(pin) {
  handle = (PwmHwEsp *)xmcMalloc(sizeof(PwmHwEsp), XMC_HEAP_CAP_DMA);
  memset(handle, 0, sizeof(PwmHwEsp));
}

PwmDriverClass::~PwmDriverClass() {
  if (handle) {
    xmcFree(handle);
    handle = nullptr;
  }
}

XmcStatus PwmDriverClass::start(const PwmConfig &cfg, float *actualFreqHz) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHwEsp *hw = (PwmHwEsp *)handle;
  esp_err_t err;

  hw->ledc_timer.duty_resolution =
      ledc_timer_bit_t(ceilf(logf(cfg.period) / logf(2)));
  hw->ledc_timer.freq_hz = cfg.freqHz;
  hw->ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
  hw->ledc_timer.timer_num = LEDC_TIMER_0;
  hw->ledc_timer.clk_cfg = LEDC_AUTO_CLK;
  err = ledc_timer_config(&hw->ledc_timer);
  if (err != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_PWM_INIT_FAILED);
  }

  hw->ledc_channel.channel = LEDC_CHANNEL_0;
  hw->ledc_channel.duty = 0;
  hw->ledc_channel.gpio_num = pin;
  hw->ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
  hw->ledc_channel.timer_sel = LEDC_TIMER_0;
  err = ledc_channel_config(&hw->ledc_channel);
  if (err != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_PWM_INIT_FAILED);
  }

  if (actualFreqHz) {
    uint32_t realFreqHz =
        ledc_get_freq(hw->ledc_channel.speed_mode, hw->ledc_timer.timer_num);
    *actualFreqHz = (float)realFreqHz;
  }

  return XMC_OK;
}

XmcStatus PwmDriverClass::stop() {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHwEsp *hw = (PwmHwEsp *)handle;
  ledc_stop(hw->ledc_channel.speed_mode, hw->ledc_channel.channel, 0);
  xmcFree(hw);
  handle = nullptr;
  return XMC_OK;
}

XmcStatus PwmDriverClass::write(uint32_t cycle) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHwEsp *hw = (PwmHwEsp *)handle;
  esp_err_t err;
  err = ledc_set_duty(hw->ledc_channel.speed_mode, hw->ledc_channel.channel,
                      cycle);
  if (err != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_PWM_UPDATE_FAILED);
  }
  err = ledc_update_duty(hw->ledc_channel.speed_mode, hw->ledc_channel.channel);
  if (err != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_PWM_UPDATE_FAILED);
  }
  return XMC_OK;
}

}  // namespace xmc::pwm
