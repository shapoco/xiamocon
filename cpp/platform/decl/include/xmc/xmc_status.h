#ifndef XMC_XMC_STATUS_H
#define XMC_XMC_STATUS_H

typedef enum {
  XMC_OK = 0,
  XMC_ERR_BAD_PARAMETER = 1,
  XMC_ERR_FUNCTION_NOT_SUPPORTED = 2,
  XMC_ERR_NOT_INITIALIZED = 3,
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
  XMC_ERR_PWM_INIT_FAILED = XMC_ERR_BASE_PWM + 1,
  XMC_ERR_BASE_DMA = 0x1000,
  XMC_ERR_DMA_INIT_FAILED = XMC_ERR_BASE_DMA + 1,
  XMC_ERR_DMA_BAD_ELEMENT_SIZE = XMC_ERR_BASE_DMA + 2,
  XMC_ERR_BASE_DISPLAY = 0x1100,
  XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT = XMC_ERR_BASE_DISPLAY + 1,
  XMC_ERR_BASE_SPEAKER = 0x1200,
  XMC_ERR_SPEAKER_INIT_FAILED = XMC_ERR_BASE_SPEAKER + 1,
  XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT = XMC_ERR_BASE_SPEAKER + 2,
  XMC_ERR_SPEAKER_STREAM_BROKEN = XMC_ERR_BASE_SPEAKER + 3,
  XMC_ERR_BASE_USER = 0x8000,
  XMC_USER_GENERIC_ERROR = XMC_ERR_BASE_USER + 1,
} XmcStatus;

#define XMC_ERR_LOG(status)                             \
  do {                                                  \
    xmcSetLastError((status), __FILE_NAME__, __LINE__); \
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

extern XmcStatus xmc_last_error_code;
extern const char *xmc_last_error_file;
extern int xmc_last_error_line;

static inline void xmcSetLastError(XmcStatus code, const char *file, int line) {
  xmc_last_error_code = code;
  xmc_last_error_file = file;
  xmc_last_error_line = line;
}

static inline void xmcGetLastError(XmcStatus *code, const char **file,
                                   int *line) {
  if (code) *code = xmc_last_error_code;
  if (file) *file = xmc_last_error_file;
  if (line) *line = xmc_last_error_line;
}

#ifdef __cplusplus
}
#endif

#endif
