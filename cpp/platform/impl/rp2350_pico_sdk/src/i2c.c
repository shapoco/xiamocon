#include "xmc/hw/i2c.h"
#include "xmc/hw/lock.h"
#include "xmc/hw/pins.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/stdlib.h>

static i2c_inst_t *const i2c_inst = i2c1;
static xmc_semaphore_t semaphore;
static bool in_transaction = false;

uint32_t xmc_i2c_get_preferred_frequency(xmc_i2c_device_t device) {
  switch (device) {
    case XMC_I2C_DEV_IOEX:
    case XMC_I2C_DEV_BAT_MON: return 1000000;
    default: return 100000;
  }
}

xmc_status_t xmc_i2c_init() {
  i2c_init(i2c_inst, xmc_i2c_get_preferred_frequency(XMC_I2C_DEV_IOEX));
  gpio_set_function(XMC_PIN_I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(XMC_PIN_I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(XMC_PIN_I2C_SDA);
  gpio_pull_up(XMC_PIN_I2C_SCL);
  XMC_ERR_RET(xmc_semaphore_init(&semaphore));
  return XMC_OK;
}

void xmc_i2c_deinit() {
  xmc_i2c_end_transaction();
  i2c_deinit(i2c_inst);
  gpio_deinit(XMC_PIN_I2C_SDA);
  gpio_deinit(XMC_PIN_I2C_SCL);
  xmc_semaphore_deinit(&semaphore);
}

xmc_status_t xmc_i2c_start_transaction() {
  if (in_transaction) return XMC_OK;
  xmc_semaphore_take(&semaphore);
  in_transaction = true;
  return XMC_OK;
}

xmc_status_t xmc_i2c_end_transaction() {
  if (!in_transaction) return XMC_OK;
  in_transaction = false;
  xmc_semaphore_give(&semaphore);
  return XMC_OK;
}

xmc_status_t xmc_i2c_set_baudrate(uint32_t baudrate) {
  i2c_set_baudrate(i2c_inst, baudrate);
  return XMC_OK;
}

xmc_status_t xmc_i2c_write_blocking(uint8_t dev_addr, const uint8_t *data,
                                    uint32_t size, bool nostop) {
  int n = i2c_write_blocking(i2c_inst, dev_addr, data, size, nostop);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_I2C_WRITE_FAILED);
  }
  return XMC_OK;
}

xmc_status_t xmc_i2c_read_blocking(uint8_t dev_addr, uint8_t *data,
                                   uint32_t size, bool nostop) {
  int n = i2c_read_blocking(i2c_inst, dev_addr, data, size, nostop);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_I2C_READ_FAILED);
  }
  return XMC_OK;
}
