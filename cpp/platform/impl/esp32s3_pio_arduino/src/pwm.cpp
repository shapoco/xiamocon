#include "xmc/pwm.hpp"
#include "xmc/heap.hpp"

#include <driver/ledc.h>

namespace xmc::pwm {

typedef struct {
  ledc_timer_config_t ledc_timer;
  ledc_channel_config_t ledc_channel;
} PwmHw;

Driver::Driver(int pin) : pin(pin) {
  handle = (PwmHw *)xmcMalloc(sizeof(PwmHw), XMC_HEAP_CAP_DMA);
}

Driver::~Driver() {
  if (handle) {
    xmcFree(handle);
    handle = nullptr;
  }
}

XmcStatus Driver::start(const Config &cfg, float *actualFreqHz) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHw *hw = (PwmHw *)handle;
  esp_err_t err;

  if (cfg.period < 4) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_2_BIT;
  } else if (cfg.period < 8) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_3_BIT;
  } else if (cfg.period < 16) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_4_BIT;
  } else if (cfg.period < 32) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_5_BIT;
  } else if (cfg.period < 64) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_6_BIT;
  } else if (cfg.period < 128) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_7_BIT;
  } else if (cfg.period < 256) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;
  } else if (cfg.period < 512) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_9_BIT;
  } else if (cfg.period < 1024) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
  } else if (cfg.period < 2048) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_11_BIT;
  } else if (cfg.period < 4096) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_12_BIT;
  } else if (cfg.period < 8192) {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_13_BIT;
  } else {
    hw->ledc_timer.duty_resolution = LEDC_TIMER_14_BIT;
  }
  hw->ledc_timer.freq_hz = cfg.freqHz;
  hw->ledc_timer.speed_mode = LEDC_SPEED_MODE_MAX;
  err = ledc_timer_config(&hw->ledc_timer);
  if (err != ESP_OK) {
    return XMC_ERR_PWM_INIT_FAILED;
  }

  hw->ledc_channel.channel = LEDC_CHANNEL_0;
  hw->ledc_channel.duty = 0;
  hw->ledc_channel.gpio_num = pin;
  hw->ledc_channel.speed_mode = LEDC_SPEED_MODE_MAX;
  err = ledc_channel_config(&hw->ledc_channel);
  if (err != ESP_OK) {
    return XMC_ERR_PWM_INIT_FAILED;
  }

  if (actualFreqHz) {
    uint32_t realFreqHz =
        ledc_get_freq(hw->ledc_channel.speed_mode, hw->ledc_timer.timer_num);
    *actualFreqHz = (float)realFreqHz;
  }

  return XMC_OK;
}

XmcStatus Driver::stop() {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHw *hw = (PwmHw *)handle;
  ledc_stop(hw->ledc_channel.speed_mode, hw->ledc_channel.channel, 0);
  xmcFree(hw);
  handle = nullptr;
  return XMC_OK;
}

XmcStatus Driver::setDutyCycle(uint32_t cycle) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  PwmHw *hw = (PwmHw *)handle;
  ledc_set_duty(hw->ledc_channel.speed_mode, hw->ledc_channel.channel, cycle);
  ledc_update_duty(hw->ledc_channel.speed_mode, hw->ledc_channel.channel);
  return XMC_OK;
}

}  // namespace xmc::pwm
