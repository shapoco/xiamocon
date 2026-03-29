#include "xmc/battery.hpp"
#include "xmc/hw/i2c.hpp"
#include "xmc/hw/timer.hpp"

namespace xmc::battery {

static const uint8_t ADC101C021_I2C_ADDR = 0x52;

enum class Adc101c021 : uint8_t {
  CONV_RESULT = 0x00,
  ALERT_STATUS = 0x01,
  CONFIG = 0x02,
  LOW_LIMIT = 0x03,
  HIGH_LIMIT = 0x04,
  HYSTERESIS = 0x05,
  LOWEST_CONV = 0x06,
  HIGHEST_CONV = 0x07,
};

static uint64_t nextUpdateMs = 0;
static uint16_t lastMv = 0;

static XmcStatus writeRegU8(Adc101c021 reg, uint8_t value);
static bool tryReadRegU16(Adc101c021 reg, uint16_t *value);

XmcStatus init() {
  uint8_t config = 0;
  config |= (0 << 5);
  XMC_ERR_RET(writeRegU8(Adc101c021::CONFIG, config));
  return XMC_OK;
}

XmcStatus deinit() {
  uint8_t config = 0;
  config |= (0 << 5);
  XMC_ERR_RET(writeRegU8(Adc101c021::CONFIG, config));
  return XMC_OK;
}

XmcStatus service() {
  uint64_t now_ms = xmc::getTimeMs();
  if (now_ms < nextUpdateMs) {
    return XMC_OK;
  }
  nextUpdateMs = now_ms + 1000;
  uint16_t raw;
  if (tryReadRegU16(Adc101c021::CONV_RESULT, &raw)) {
    lastMv = (uint32_t)((raw >> 2) & 0x3FF) * 2 * 3300 / 1024;
  }
  return XMC_OK;
}

uint16_t getVoltageMv() { return lastMv; }

static XmcStatus writeRegU8(Adc101c021 reg, uint8_t value) {
  XmcStatus status = XMC_OK;
  uint8_t buf[2] = {(uint8_t)reg, value};
  XMC_ERR_RET(i2c::lock());
  do {
    XMC_ERR_BRK(
        status,
        i2c::setBaudrate(i2c::getPreferredFrequency(Chipset::BATTERY_MONITOR)));
    XMC_ERR_BRK(status, i2c::writeBlocking(ADC101C021_I2C_ADDR, buf,
                                           sizeof(buf), false));
  } while (0);
  i2c::unlock();
  return XMC_OK;
}

static bool tryReadRegU16(Adc101c021 reg, uint16_t *value) {
  XmcStatus status = XMC_OK;
  uint8_t reg_addr = (uint8_t)reg;
  uint8_t data[2];
  if (!i2c::tryLock()) {
    return false;
  }
  do {
    XMC_ERR_BRK(
        status,
        i2c::setBaudrate(i2c::getPreferredFrequency(Chipset::BATTERY_MONITOR)));
    XMC_ERR_BRK(status,
                i2c::writeBlocking(ADC101C021_I2C_ADDR, &reg_addr, 1, false));
    XMC_ERR_BRK(status, i2c::readBlocking(ADC101C021_I2C_ADDR, data, 2, false));
    *value = ((uint16_t)data[0] << 8) | data[1];
  } while (0);
  i2c::unlock();
  return status == XMC_OK;
}

}  // namespace xmc::battery
