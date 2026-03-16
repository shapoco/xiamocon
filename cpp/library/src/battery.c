#include "xmc/battery.h"
#include "xmc/hw/i2c.h"
#include "xmc/hw/timer.h"

static const uint8_t ADC101C021_I2C_ADDR = 0x52;

typedef enum {
  ADC101C021_ADDR_CONV_RESULT = 0x00,
  ADC101C021_ADDR_ALERT_STATUS = 0x01,
  ADC101C021_ADDR_CONFIG = 0x02,
  ADC101C021_ADDR_LOW_LIMIT = 0x03,
  ADC101C021_ADDR_HIGH_LIMIT = 0x04,
  ADC101C021_ADDR_HYSTERESIS = 0x05,
  ADC101C021_ADDR_LOWEST_CONV = 0x06,
  ADC101C021_ADDR_HIGHEST_CONV = 0x07,
} adc101c021_reg_t;

static uint64_t next_update_ms = 0;
static uint16_t last_mv = 0;

static xmc_status_t write_reg_u8(adc101c021_reg_t reg, uint8_t value);
static bool try_read_reg_u16(adc101c021_reg_t reg, uint16_t *value);

xmc_status_t xmc_battery_init() {
  uint8_t config = 0;
  config |= (0 << 5);
  XMC_ERR_RET(write_reg_u8(ADC101C021_ADDR_CONFIG, config));
  xmc_battery_service();
  return XMC_OK;
}

xmc_status_t xmc_battery_deinit() {
  uint8_t config = 0;
  config |= (0 << 5);
  XMC_ERR_RET(write_reg_u8(ADC101C021_ADDR_CONFIG, config));
  return XMC_OK;
}

xmc_status_t xmc_battery_service() {
  uint64_t now_ms = xmc_get_time_ms();
  if (now_ms < next_update_ms) {
    return XMC_OK;
  }
  next_update_ms = now_ms + 1000;
  uint16_t raw;
  if (try_read_reg_u16(ADC101C021_ADDR_CONV_RESULT, &raw)) {
    last_mv = (uint32_t)((raw >> 2) & 0x3FF) * 2 * 3300 / 1024;
  }
  return XMC_OK;
}

uint16_t xmc_battery_get_voltage_mv() { return last_mv; }

static xmc_status_t write_reg_u8(adc101c021_reg_t reg, uint8_t value) {
  xmc_status_t status = XMC_OK;
  uint8_t buf[2] = {(uint8_t)reg, value};
  XMC_ERR_RET(xmc_i2c_lock());
  do {
    XMC_ERR_BRK(status, xmc_i2c_set_baudrate(xmc_i2c_get_preferred_frequency(
                            XMC_I2C_DEV_BAT_MON)));
    XMC_ERR_BRK(status, xmc_i2c_write_blocking(ADC101C021_I2C_ADDR, buf,
                                               sizeof(buf), false));
  } while (0);
  XMC_ERR_RET(xmc_i2c_unlock());
  return XMC_OK;
}

static bool try_read_reg_u16(adc101c021_reg_t reg, uint16_t *value) {
  xmc_status_t status = XMC_OK;
  uint8_t reg_addr = (uint8_t)reg;
  uint8_t data[2];
  if (!xmc_i2c_try_lock()) {
    return false;
  }
  do {
    XMC_ERR_BRK(status, xmc_i2c_set_baudrate(xmc_i2c_get_preferred_frequency(
                            XMC_I2C_DEV_BAT_MON)));
    XMC_ERR_BRK(status, xmc_i2c_write_blocking(ADC101C021_I2C_ADDR, &reg_addr,
                                               1, false));
    XMC_ERR_BRK(status,
                xmc_i2c_read_blocking(ADC101C021_I2C_ADDR, data, 2, false));
    *value = ((uint16_t)data[0] << 8) | data[1];
  } while (0);
  xmc_i2c_unlock();
  return status == XMC_OK;
}
