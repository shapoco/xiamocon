#include "xmc/hw/i2c.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/semaphore.hpp"

#include <Wire.h>

namespace xmc::i2c {

static Semaphore *semaphore = nullptr;

uint32_t getPreferredFrequency(Chipset device) {
  switch (device) {
    case Chipset::IO_EXPANDER: return 400000;
    case Chipset::BATTERY_MONITOR: return 400000;
    default: return 400000;
  }
}

XmcStatus init() {
  Wire.begin();
  Wire.setClock(getPreferredFrequency(Chipset::IO_EXPANDER));
  if (semaphore) delete semaphore;
  semaphore = new Semaphore();
  if (!semaphore->isInitialized()) {
    XMC_ERR_RET(XMC_ERR_SEMAPHORE_INIT_FAILED);
  }
  return XMC_OK;
}

void deinit() {
  Wire.end();
  gpio::setDir(XMC_PIN_I2C_SDA, false);
  gpio::setDir(XMC_PIN_I2C_SCL, false);
  if (semaphore) {
    delete semaphore;
    semaphore = nullptr;
  }
}

bool tryLock() {
  if (!semaphore) return true;
  return semaphore->tryTake();
}

XmcStatus unlock() {
  if (semaphore) semaphore->give();
  return XMC_OK;
}

XmcStatus setBaudrate(uint32_t baudrate) {
  Wire.setClock(baudrate);
  return XMC_OK;
}

XmcStatus writeBlocking(uint8_t dev_addr, const uint8_t *data, uint32_t size,
                        bool nostop) {
  Wire.beginTransmission(dev_addr);
  Wire.write(data, size);
  Wire.endTransmission(!nostop);
  return XMC_OK;
}

XmcStatus readBlocking(uint8_t dev_addr, uint8_t *data, uint32_t size,
                       bool nostop) {
  Wire.requestFrom(dev_addr, size, nostop);
  for (uint32_t i = 0; i < size; i++) {
    if (Wire.available()) {
      data[i] = Wire.read();
    } else {
      return XMC_ERR_I2C_READ_FAILED;
    }
  }
  return XMC_OK;
}

}  // namespace xmc::i2c
