#include "xmc/hw/gpio.hpp"
#include "xmc/hw/hw_common.hpp"

#include <Arduino.h>

namespace xmc::gpio {

void setDir(int pin, bool output) { pinMode(pin, output ? OUTPUT : INPUT); }

void write(int pin, bool value) { digitalWrite(pin, value ? HIGH : LOW); }

bool read(int pin) { return digitalRead(pin) != LOW; }

void setPullup(int pin, bool enable) {
  pinMode(pin, enable ? INPUT_PULLUP : INPUT);
}

}  // namespace xmc::gpio
