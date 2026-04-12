#include "xmc/hw/i2c.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/semaphore.hpp"
#include "xmc/hw/timer.hpp"

#include <Wire.h>

namespace xmc::i2c {

static Semaphore *semaphore = nullptr;
bool isInited = false;

uint32_t getPreferredFrequency(Chipset device) {
  switch (device) {
    case Chipset::IO_EXPANDER: return 400000;
    case Chipset::BATTERY_MONITOR: return 400000;
    default: return 400000;
  }
}

XmcStatus init() {
  resetBus();
  Wire.begin();
  Wire.setClock(getPreferredFrequency(Chipset::IO_EXPANDER));
  isInited = true;
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
  isInited = false;
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

XmcStatus resetBus() {
  XmcStatus sts = XMC_OK;

  bool reInit = isInited;
  if (reInit) {
    deinit();
  }

  gpio::setDir(XMC_PIN_I2C_SDA, false);
  gpio::setDir(XMC_PIN_I2C_SCL, false);
  int numRetriesLeft = BUS_RESET_RETRY_COUNT;
  while (--numRetriesLeft >= 0) {
    if (!gpio::read(XMC_PIN_I2C_SDA)) {
      // Clock the bus until the slave releases the SDA line
      gpio::setDir(XMC_PIN_I2C_SCL, true);
      gpio::write(XMC_PIN_I2C_SCL, false);
      sleepUs(100);
      for (int i = 0; i < 9; i++) {
        gpio::write(XMC_PIN_I2C_SCL, true);
        sleepUs(100);
        gpio::write(XMC_PIN_I2C_SCL, false);
        sleepUs(100);
        if (gpio::read(XMC_PIN_I2C_SDA)) {
          break;
        }
      }
      // Send a stop condition
      gpio::write(XMC_PIN_I2C_SCL, false);
      gpio::write(XMC_PIN_I2C_SDA, false);
      gpio::setDir(XMC_PIN_I2C_SCL, true);
      gpio::setDir(XMC_PIN_I2C_SDA, true);
      sleepUs(100);
      gpio::setDir(XMC_PIN_I2C_SCL, false);
      sleepUs(100);
      gpio::setDir(XMC_PIN_I2C_SDA, false);
      sleepUs(100);
    }
  }
  if (numRetriesLeft < 0) {
    sts = XNC_ERR_I2C_BUS_RESET_FAILED;
  }

  if (reInit) {
    sts = init();
  }
  return sts;
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
