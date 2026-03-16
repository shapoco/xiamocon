#ifndef XMC_BATTERY_H
#define XMC_BATTERY_H

#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

xmc_status_t xmc_battery_init();
xmc_status_t xmc_battery_deinit();
xmc_status_t xmc_battery_service();
uint16_t xmc_battery_get_voltage_mv();

#if defined(__cplusplus)
}
#endif

#endif  // XMC_BATTERY_H

