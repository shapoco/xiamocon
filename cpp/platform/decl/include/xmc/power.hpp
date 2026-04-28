/**
 * @file power.hpp
 * @brief Power management interface
 */

#ifndef XMC_HW_POWER_HPP
#define XMC_HW_POWER_HPP

#include "xmc/hw/hw_common.hpp"

#include <stdint.h>

namespace xmc::power {

/** Reset modes for reset. */
typedef enum {
  /** Normal reset mode */
  NORMAL = 0,
} ResetMode;

/**
 * Initialize the power management functionality.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
XmcStatus init();

/**
 * Service the power management functionality.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
XmcStatus service();

/** Enter deep sleep mode.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
XmcStatus deepSleep();

/** Reset entire system.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
XmcStatus reset(ResetMode mode);

}  // namespace xmc::power

#endif
