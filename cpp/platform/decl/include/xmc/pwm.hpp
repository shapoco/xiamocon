/**
 * @file pwm.hpp
 * @brief Hardware PWM interface
 */

#ifndef XMC_HW_PWM_HPP
#define XMC_HW_PWM_HPP

#include "xmc/dma.hpp"
#include "xmc/hw_common.hpp"

namespace xmc::pwm {

struct Config {
  uint32_t freqHz;
  uint32_t period;
};

class Driver {
 public:
  const int pin;
  void *handle = nullptr;
  Driver(int pin);
  ~Driver();

  /**
   * Start the PWM signal generation.
   * @return XmcStatus indicating success or failure.
   */
  XmcStatus start(const Config &cfg, float *actualFreqHz = nullptr);

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
  XmcStatus setDutyCycle(uint32_t cycle);
};

}  // namespace xmc::pwm

#endif
