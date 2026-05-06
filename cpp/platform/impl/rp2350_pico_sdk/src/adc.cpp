#include "xmc/adc.hpp"
#include <hardware/adc.h>
#include "xmc/gpio.hpp"

namespace xmc::adc {

bool adcInitialized = false;

struct AdcHwRp2350 {
  int adcChannel;
};

AdcConfig getDefaultAdcConfig() { return {}; }

AdcDriverClass::AdcDriverClass(int pin) : pin(pin) {
  AdcHwRp2350 *hw = new AdcHwRp2350();
  hw->adcChannel = pin - 26;
  handle = hw;
}

AdcDriverClass::~AdcDriverClass() {
  if (handle) {
    delete static_cast<AdcHwRp2350 *>(handle);
    handle = nullptr;
  }
}

XmcStatus AdcDriverClass::init(const AdcConfig &cfg) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  if (!adcInitialized) {
    adc_init();
    adcInitialized = true;
  }
  gpio::setDir(pin, false);
  return XMC_OK;
}

void AdcDriverClass::deinit() {
  // No specific deinitialization needed for RP2350 ADC
}

void AdcDriverClass::getMaxValue(uint16_t *raw, float *voltage) {
  if (raw) *raw = 4095;
  if (voltage) *voltage = 3.3f;
}

XmcStatus AdcDriverClass::readRaw(uint16_t *value) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  AdcHwRp2350 *hw = (AdcHwRp2350 *)handle;
  adc_select_input(hw->adcChannel);
  *value = adc_read();
  return XMC_OK;
}

XmcStatus AdcDriverClass::readVoltage(float *value) {
  uint16_t valueRaw;
  XMC_ERR_RET(readRaw(&valueRaw));
  uint16_t maxRaw;
  float maxVoltage;
  getMaxValue(&maxRaw, &maxVoltage);
  *value = (valueRaw / (float)maxRaw) * maxVoltage;
  return XMC_OK;
}

}  // namespace xmc::adc
