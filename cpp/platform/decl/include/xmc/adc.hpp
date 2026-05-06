#ifndef XMC_HW_ADC_HPP
#define XMC_HW_ADC_HPP

/**
 * @file adc.hpp
 * @brief ADC API declarations
 */

#include "xmc/hw_common.hpp"
#include "xmc/pins.hpp"

#include <memory>

namespace xmc::adc {

struct AdcConfig {};

AdcConfig getDefaultAdcConfig();

class AdcDriverClass {
 public:
  const int pin;
  void *handle = nullptr;
  AdcDriverClass(int pin = XMC_PIN_GPIO_0);
  ~AdcDriverClass();
  XmcStatus init(const AdcConfig &cfg);
  void deinit();
  void getMaxValue(uint16_t *raw = nullptr, float *voltage = nullptr);
  XmcStatus readRaw(uint16_t *value);

  XmcStatus readVoltage(float *value) {
    uint16_t valueRaw;
    XMC_ERR_RET(readRaw(&valueRaw));
    *value = rawToVoltage(valueRaw);
    return XMC_OK;
  }

  float rawToVoltage(uint16_t raw) {
    uint16_t maxRaw;
    float maxVoltage;
    getMaxValue(&maxRaw, &maxVoltage);
    return (raw / (float)maxRaw) * maxVoltage;
  }
};

using AdcDriver = std::shared_ptr<AdcDriverClass>;

static inline AdcDriver createAdcDriver(int pin = XMC_PIN_GPIO_0) {
  return std::make_shared<AdcDriverClass>(pin);
}

}  // namespace xmc::adc

#endif
