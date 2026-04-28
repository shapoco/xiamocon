#include "xmc/gpio.hpp"
#include "xmc/hw_common.hpp"

#include <hardware/gpio.h>

namespace xmc::gpio {

void setDir(int pin, bool output) {
  gpio_init(pin);
  gpio_set_dir(pin, output ? GPIO_OUT : GPIO_IN);
}

void write(int pin, bool value) { gpio_put(pin, value ? 1 : 0); }

bool read(int pin) { return gpio_get(pin) != 0; }

void setPullup(int pin, bool enable) { gpio_set_pulls(pin, enable, false); }

}  // namespace xmc::gpio
