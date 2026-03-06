/**
 * @file power.h
 * @brief Power management interface
 */

#ifndef XMC_HW_POWER_H
#define XMC_HW_POWER_H

#include "xmc/hw/hw_common.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** Reset modes for xmc_power_reset. */
typedef enum {
  /** Normal reset mode */
  XMC_RESET_MODE_NORMAL = 0,
} xmc_reset_mode_t;

/**
 * Initialize the power management functionality.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
xmc_status_t xmc_power_init();

/**
 * Service the power management functionality.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
xmc_status_t xmc_power_service();

/** Enter deep sleep mode.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
xmc_status_t xmc_power_deep_sleep();

/** Reset entire system.
 *
 * @warning This function is used internally by the System API. It should not
 * be called from user applications.
 */
xmc_status_t xmc_power_reset(xmc_reset_mode_t mode);

#if defined(__cplusplus)
}
#endif

#endif
