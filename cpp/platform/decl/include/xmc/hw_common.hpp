#ifndef XMC_HW_HW_COMMON_H
#define XMC_HW_HW_COMMON_H

#include "xmc/xmc_common.hpp"

/**
 * I2C device identifiers for getting preferred baud rates.
 */
enum class Chipset {
  /** Display */
  DISPLAY = (1 << 0),
  /** Memory Card */
  MEMORY_CARD = (1 << 1),
  /** IO Expander */
  IO_EXPANDER = (1 << 2),
  /** Battery Monitor */
  BATTERY_MONITOR = (1 << 3),
};

#endif
