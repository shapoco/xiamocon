#include "xmc/adc.hpp"
#include <Arduino.h>
#include "xmc/gpio.hpp"

namespace xmc::adc {

struct AdcHwEsp32s3 {
  int adcChannel;
};

AdcConfig getDefaultAdcConfig() { return {}; }

AdcDriverClass::AdcDriverClass(int pin) : pin(pin) {
  AdcHwEsp32s3 *hw = new AdcHwEsp32s3();
  hw->adcChannel = digitalPinToAnalogChannel(pin);
  handle = hw;
}

AdcDriverClass::~AdcDriverClass() {
  if (handle) {
    delete static_cast<AdcHwEsp32s3 *>(handle);
    handle = nullptr;
  }
}

XmcStatus AdcDriverClass::init(const AdcConfig &cfg) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  gpio::setDir(pin, false);
  return XMC_OK;
}

void AdcDriverClass::deinit() {
  // No specific deinitialization needed for Arduino ADC
}

void AdcDriverClass::getMaxValue(uint16_t *raw, float *voltage) {
  if (raw) *raw = 4095;
  if (voltage) *voltage = 3.1f;
}

XmcStatus AdcDriverClass::readRaw(uint16_t *value) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  *value = analogRead(pin);
  return XMC_OK;
}

}  // namespace xmc::adc
