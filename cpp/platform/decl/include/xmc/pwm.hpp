/**
 * @file pwm.hpp
 * @brief Hardware PWM interface
 */

#ifndef XMC_HW_PWM_HPP
#define XMC_HW_PWM_HPP

#include "xmc/dma.hpp"
#include "xmc/hw_common.hpp"
#include "xmc/pins.hpp"

#include <memory>

namespace xmc::pwm {

struct PwmConfig {
  uint32_t freqHz;
  uint32_t period;
};

PwmConfig getDefaultPwmConfig();

class PwmDriverClass {
 public:
  const int pin;
  void *handle = nullptr;
  PwmDriverClass(int pin);
  ~PwmDriverClass();

  /**
   * Start the PWM signal generation.
   * @return XmcStatus indicating success or failure.
   */
  XmcStatus start(const PwmConfig &cfg, float *actualFreqHz = nullptr);

  /**
   * Stop the PWM signal generation.
   * @return XmcStatus indicating success or failure.
   */
  XmcStatus stop();

  /**
   * Set the duty cycle of the PWM signal.
   * @param cycle The duty cycle value.
   * @return XmcStatus indicating success or failure.
   */
  XmcStatus write(uint32_t cycle);
};

using PwmDriver = std::shared_ptr<PwmDriverClass>;

static inline PwmDriver createPwmDriver(int pin = XMC_PIN_GPIO_0) {
  return std::make_shared<PwmDriverClass>(pin);
}

}  // namespace xmc::pwm

#endif
