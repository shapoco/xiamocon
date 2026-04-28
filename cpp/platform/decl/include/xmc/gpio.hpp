/**
 * @file gpio.hpp
 * @brief GPIO hardware interface
 */

#ifndef XMC_HW_GPIO_HPP
#define XMC_HW_GPIO_HPP

#include "xmc/hw/hw_common.hpp"
#include "xmc/hw/pins.hpp"

#include <stdint.h>

namespace xmc::gpio {

/**
 * Set the direction of a GPIO pin.
 * @param pin The GPIO pin number.
 * @param output If true, set the pin as an output. If false, set it as an
 * input.
 */
void setDir(int pin, bool output);

/**
 * Write a value to a GPIO pin.
 * @param pin The GPIO pin number.
 * @param value The value to write. True for high, false for low.
 */
void write(int pin, bool value);

/**
 * Read the value of a GPIO pin.
 * @param pin The GPIO pin number.
 * @return True if the pin is high, false if it is low.
 */
bool read(int pin);

/**
 * Set the pull-up resistor of a GPIO pin.
 * @param pin The GPIO pin number.
 * @param enable If true, enable the pull-up resistor. If false, disable it.
 */
void setPullup(int pin, bool enable);

}  // namespace xmc

#endif
