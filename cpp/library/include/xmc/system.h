/**
 * @file system.h
 * @brief System-level functions for Xiamocon
 */

#ifndef XMC_SYSTEM_H
#define XMC_SYSTEM_H

#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Initialize the Xiamocon system.
 *
 * @warning This function is used internally by the platform API. It should not
 * be called from user applications.
 *
 * @return XMC_OK if the system was successfully initialized, or an appropriate
 * error code if initialization failed.
 */
xmc_status_t xmc_sys_init();

/**
 * Perform periodic system tasks.
 * 
 * @warning This function is used internally by the platform API. It should not
 * be called from user applications.
 *
 * @return XMC_OK if the system service was successfully performed, or an
 * appropriate error code if there was a problem.
 */
xmc_status_t xmc_sys_service();

/**
 * Request a system shutdown. This will attempt to gracefully shut down the
 * system by disabling peripherals, and then entering deep sleep mode.
 *
 * @return XMC_OK if the system was successfully shut down, or an appropriate
 * error code if shutdown failed.
 */
xmc_status_t xmc_sys_request_shutdown();

#if defined(__cplusplus)
}
#endif

#endif  // XMC_SYSTEM_H
