#ifndef XMC_XMC_COMMON_H
#define XMC_XMC_COMMON_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  XMC_OK = 0,
  XMC_ERR_BAD_PARAMETER = 1,
  XMC_ERR_FUNCTION_NOT_SUPPORTED = 2,
  XMC_ERR_BASE_RAM = 0x100,
  XMC_ERR_RAM_ALLOC_FAILED = XMC_ERR_BASE_RAM + 1,
  XMC_ERR_BASE_LOCK = 0x200,
  XMC_ERR_SPINLOCK_INIT_FAILED = XMC_ERR_BASE_LOCK + 1,
  XMC_ERR_SEMAPHORE_INIT_FAILED = XMC_ERR_BASE_LOCK + 2,
  XMC_ERR_BASE_POWER = 0x300,
  XMC_ERR_POWER_INIT_FAILED = XMC_ERR_BASE_POWER + 1,
  XMC_ERR_POWER_SLEEP_FAILED = XMC_ERR_BASE_POWER + 2,
  XMC_ERR_POWER_RESET_FAILED = XMC_ERR_BASE_POWER + 3,
  XMC_ERR_BASE_GPIO = 0x400,
  XMC_ERR_BASE_TIMER = 0x500,
  XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED = XMC_ERR_BASE_TIMER + 1,
  XMC_ERR_BASE_I2C = 0x600,
  XMC_ERR_I2C_NOT_INITIALIZED = XMC_ERR_BASE_I2C + 1,
  XMC_ERR_I2C_INVALID_BAUDRATE = XMC_ERR_BASE_I2C + 2,
  XMC_ERR_I2C_WRITE_FAILED = XMC_ERR_BASE_I2C + 3,
  XMC_ERR_I2C_READ_FAILED = XMC_ERR_BASE_I2C + 4,
  XMC_ERR_BASE_SPI = 0x700,
  XMC_ERR_SPI_NOT_INITIALIZED = XMC_ERR_BASE_SPI + 1,
  XMC_ERR_SPI_INVALID_BAUDRATE = XMC_ERR_BASE_SPI + 2,
  XMC_ERR_SPI_WRITE_FAILED = XMC_ERR_BASE_SPI + 3,
  XMC_ERR_SPI_READ_FAILED = XMC_ERR_BASE_SPI + 4,
  XMC_ERR_BASE_PWM = 0x800,
  XMC_ERR_BASE_DMA = 0x1000,
  XMC_ERR_DMA_INIT_FAILED = XMC_ERR_BASE_DMA + 1,
  XMC_ERR_DMA_BAD_ELEMENT_SIZE = XMC_ERR_BASE_DMA + 2,
  XMC_ERR_BASE_DISPLAY = 0x1100,
  XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT = XMC_ERR_BASE_DISPLAY + 1,
  XMC_ERR_BASE_SPEAKER = 0x1200,
  XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT = XMC_ERR_BASE_SPEAKER + 1,
  XMC_ERR_BASE_USER = 0x8000,
  XMC_USER_GENERIC_ERROR = XMC_ERR_BASE_USER + 1,
} xmc_status_t;

#define XMC_ERR_LOG(status) \
  do {                      \
  } while (0)

// if the status is not XMC_OK, log the error and return from the current
// function.
#define XMC_ERR_RET(status)   \
  do {                        \
    if ((status) != XMC_OK) { \
      XMC_ERR_LOG(status);    \
      return (status);        \
    }                         \
  } while (0)

// if the status is not XMC_OK, log the error and break from the current loop.
#define XMC_ERR_BRK(sts_var, status)    \
  if ((sts_var = (status)) != XMC_OK) { \
    XMC_ERR_LOG(status);                \
    break;                              \
  }

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contents of a tight loop.
 */
void xmc_tight_loop_contents();

#ifdef __cplusplus
}
#endif

#endif
