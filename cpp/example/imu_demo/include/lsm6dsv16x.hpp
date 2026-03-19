#ifndef LSM6DSV16X_HPP
#define LSM6DSV16X_HPP

#include <xmc/hw/i2c.h>

namespace lsm6dsv16x {

enum reg_t {
  REG_FUNC_CFG_ACCESS = 0x01,
  REG_FIFO_CTRL4 = 0x0A,
  REG_WHO_AM_I = 0x0F,
  REG_CTRL1 = 0x10,
  REG_CTRL2 = 0x11,
  REG_CTRL3 = 0x12,
  REG_CTRL6 = 0x15,
  REG_CTRL8 = 0x17,
  REG_CTRL9 = 0x18,
  REG_OUT_TEMP_L = 0x20,
  REG_OUT_TEMP_H = 0x21,
  REG_OUTX_L_G = 0x22,
  REG_OUTX_H_G = 0x23,
  REG_OUTY_L_G = 0x24,
  REG_OUTY_H_G = 0x25,
  REG_OUTZ_L_G = 0x26,
  REG_OUTZ_H_G = 0x27,
  REG_OUTX_L_A = 0x28,
  REG_OUTX_H_A = 0x29,
  REG_OUTY_L_A = 0x2A,
  REG_OUTY_H_A = 0x2B,
  REG_OUTZ_L_A = 0x2C,
  REG_OUTZ_H_A = 0x2D,
};

static constexpr uint8_t FUNC_CFG_ACCESS_SW_POR = 0b1 << 2;

enum fifo_ctrl4_t {
  FIFO_CTRL4_FIFO_MODE_BYPASS = 0b000,
  FIFO_CTRL4_FIFO_MODE_FIFO = 0b001,
  FIFO_CTRL4_FIFO_MODE_CONTINUOUS_WTM_TO_FULL = 0b010,
  FIFO_CTRL4_FIFO_MODE_CONTINUOUS_TO_FIFO = 0b011,
  FIFO_CTRL4_FIFO_MODE_BYPASS_TO_CONTINUOUS = 0b100,
  FIFO_CTRL4_FIFO_MODE_CONTINUOUS = 0b110,
  FIFO_CTRL4_FIFO_MODE_BYPASS_TO_FIFO = 0b111,
  FIFO_CTRL4_G_EIS_FIFO_EN = 0b1 << 3,
  FIFO_CTRL4_ODR_T_BATCH_OFF = 0b00 << 4,
  FIFO_CTRL4_ODR_T_BATCH_1_875HZ = 0b01 << 4,
  FIFO_CTRL4_ODR_T_BATCH_15HZ = 0b10 << 4,
  FIFO_CTRL4_ODR_T_BATCH_60HZ = 0b11 << 4,
  FIFO_CTRL4_DEC_TS_BATCH_OFF = 0b00 << 6,
  FIFO_CTRL4_DEC_TS_BATCH_DEC_1 = 0b01 << 6,
  FIFO_CTRL4_DEC_TS_BATCH_DEC_8 = 0b10 << 6,
  FIFO_CTRL4_DEC_TS_BATCH_DEC_32 = 0b11 << 6,
};

enum ctrl1_t {
  CTRL1_ODR_XL_POWER_DOWN = 0b0000,
  CTRL1_ODR_XL_LP_1_875HZ = 0b0001,
  CTRL1_ODR_XL_LP_7_5HZ = 0b0010,
  CTRL1_ODR_XL_LP_15_HZ = 0b0011,
  CTRL1_ODR_XL_LP_30_HZ = 0b0100,
  CTRL1_ODR_XL_LP_60_HZ = 0b0101,
  CTRL1_ODR_XL_LP_120_HZ = 0b0110,
  CTRL1_ODR_XL_LP_240_HZ = 0b0111,
  CTRL1_ODR_XL_HP_480_HZ = 0b1000,
  CTRL1_ODR_XL_HP_960_HZ = 0b1001,
  CTRL1_ODR_XL_HP_1920_HZ = 0b1010,
  CTRL1_ODR_XL_HP_3840_HZ = 0b1011,
  CTRL1_ODR_XL_HP_7680_HZ = 0b1100,
  CTRL1_OP_MODE_XL_HIGH_PERFORMANCE = 0b000 << 4,
  CTRL1_OP_MODE_XL_HIGH_ACCURACY = 0b001 << 4,
  CTRL1_OP_MODE_XL_ODR_TRIGGERED = 0b011 << 4,
  CTRL1_OP_MODE_XL_LOW_POWER_1 = 0b100 << 4,
  CTRL1_OP_MODE_XL_LOW_POWER_2 = 0b101 << 4,
  CTRL1_OP_MODE_XL_LOW_POWER_3 = 0b110 << 4,
  CTRL1_OP_MODE_XL_NORMAL = 0b111 << 4,
};

enum ctrl2_t {
  CTRL2_ODR_G_POWER_DOWN = 0b0000,
  CTRL2_ODR_G_LP_7_5HZ = 0b0010,
  CTRL2_ODR_G_LP_15_HZ = 0b0011,
  CTRL2_ODR_G_LP_30_HZ = 0b0100,
  CTRL2_ODR_G_LP_60_HZ = 0b0101,
  CTRL2_ODR_G_LP_120_HZ = 0b0110,
  CTRL2_ODR_G_LP_240_HZ = 0b0111,
  CTRL2_ODR_G_HP_480_HZ = 0b1000,
  CTRL2_ODR_G_HP_960_HZ = 0b1001,
  CTRL2_ODR_G_HP_1920_HZ = 0b1010,
  CTRL2_ODR_G_HP_3840_HZ = 0b1011,
  CTRL2_ODR_G_HP_7680_HZ = 0b1100,
  CTRL2_OP_MODE_G_HIGH_PERFORMANCE = 0b000 << 4,
  CTRL2_OP_MODE_G_HIGH_ACCURACY = 0b001 << 4,
  CTRL2_OP_MODE_G_ODR_TRIGGERED = 0b011 << 4,
  CTRL2_OP_MODE_G_SLEEP_MODE = 0b100 << 4,
  CTRL2_OP_MODE_G_LOW_POWER = 0b101 << 4,
};

enum ctrl3_t {
  CTRL3_SW_RESET = 0b1 << 0,
  CTRL3_IF_INC = 0b1 << 2,
  CTRL3_BDU = 0b1 << 6,
  CTRL3_BOOT = 0b1 << 7,
};

enum ctrl6_t {
  CTRL6_FS_G_DPS_125 = 0b0000,
  CTRL6_FS_G_DPS_250 = 0b0001,
  CTRL6_FS_G_DPS_500 = 0b0010,
  CTRL6_FS_G_DPS_1000 = 0b0011,
  CTRL6_FS_G_DPS_2000 = 0b0100,
  CTRL6_FS_G_DPS_4000 = 0b1100,
  CTRL6_LPF1_G_BW_0 = 0b001 << 4,
  CTRL6_LPF1_G_BW_1 = 0b010 << 4,
  CTRL6_LPF1_G_BW_2 = 0b100 << 4,
};

enum ctrl8_t {
  CTRL8_FS_XL_2G = 0b00,
  CTRL8_FS_XL_4G = 0b01,
  CTRL8_FS_XL_8G = 0b10,
  CTRL8_FS_XL_16G = 0b11,
  CTRL8_XL_DUALC_EN = 0b1 << 3,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_4 = 0b000 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_10 = 0b001 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_20 = 0b010 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_45 = 0b011 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_100 = 0b100 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_200 = 0b101 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_400 = 0b110 << 5,
  CTRL8_HP_LPF2_XL_BW_ODR_DIV_800 = 0b111 << 5,
};

static constexpr uint8_t CTRL9_USR_OFF_ON_OUT = 0b1 << 0;
static constexpr uint8_t CTRL9_USR_OFF_W = 0b1 << 1;
static constexpr uint8_t CTRL9_LPF_XL_EN = 0b1 << 4;
static constexpr uint8_t CTRL9_HP_SLOPE_XL_EN = 0b1 << 4;
static constexpr uint8_t CTRL9_XL_FASTSETTL_MODE = 0b1 << 5;
static constexpr uint8_t CTRL9_HP_REF_MODE_XL = 0b1 << 6;

class SensorI2C {
 public:
  const uint8_t dev_addr;
  const uint32_t baudrate_hz;

 private:
  bool inited = false;

 public:
  // todo: test 1MHz
  SensorI2C(uint8_t addr = 0x6A, uint32_t baud = 400000)
      : dev_addr(addr), baudrate_hz(baud) {}

  xmc_status_t init() {
    inited = false;

    for (int i = 10; i >= 0; i--) {
      uint8_t ctrl3;
      xmc_sleep_ms(100);
      xmc_status_t sts = read_reg(REG_CTRL3, &ctrl3);
      if (sts == XMC_OK && (ctrl3 & (CTRL3_BOOT | CTRL3_SW_RESET)) == 0) {
        break;
      } else if (i == 0) {
        return XMC_USER_GENERIC_ERROR;
      }
    }

    {
      uint8_t who_am_i;
      XMC_ERR_RET(read_reg(REG_WHO_AM_I, &who_am_i));
      if (who_am_i != 0x70) {
        return XMC_USER_GENERIC_ERROR;
      }
    }

    // XMC_ERR_RET(write_reg(REG_FUNC_CFG_ACCESS, FUNC_CFG_ACCESS_SW_POR));
    // xmc_sleep_ms(100);
    // XMC_ERR_RET(write_reg(REG_CTRL3, CTRL3_SW_RESET));
    // xmc_sleep_ms(100);
    //  XMC_ERR_RET(write_reg(REG_CTRL3, CTRL3_IF_INC | CTRL3_BDU));

    // XMC_ERR_RET(write_reg(
    //     REG_CTRL1, (CTRL1_ODR_XL_LP_60_HZ |
    //     CTRL1_OP_MODE_XL_HIGH_ACCURACY)));
    // XMC_ERR_RET(write_reg(
    //     REG_CTRL2, (CTRL2_ODR_G_LP_60_HZ | CTRL2_OP_MODE_G_HIGH_ACCURACY)));
    // XMC_ERR_RET(write_reg(REG_CTRL6, CTRL6_FS_G_DPS_2000));
    // XMC_ERR_RET(
    //     write_reg(REG_CTRL8, CTRL8_FS_XL_8G |
    //     CTRL8_HP_LPF2_XL_BW_ODR_DIV_100));

    XMC_ERR_RET(write_reg(REG_CTRL1, 0b00001001));
    XMC_ERR_RET(write_reg(REG_CTRL2, 0b00001001));
    // XMC_ERR_RET(write_reg(REG_CTRL6, 0b00000000));
    XMC_ERR_RET(write_reg(REG_CTRL6, 0b00000001));
    // XMC_ERR_RET(write_reg(REG_CTRL6, 0b00000100));
    //  XMC_ERR_RET(write_reg(REG_CTRL8, 0b10000000));
    XMC_ERR_RET(write_reg(REG_CTRL8, 0b10000001));
    // XMC_ERR_RET(write_reg(REG_CTRL8, 0b10000010));

    // XMC_ERR_RET(write_reg(REG_CTRL9, CTRL9_LPF_XL_EN | CTRL9_HP_SLOPE_XL_EN |
    // CTRL9_XL_FASTSETTL_MODE));

    //  write_reg((reg_t)0x12, 0x01);  // SW_RESET
    //  write_reg((reg_t)0x12, 0x44);  // BDU + IF_INC
    //  write_reg((reg_t)0x17, 0x40);  // ±4g, LPF2 = ODR/20
    //  write_reg((reg_t)0x18, 0x08);  // LPF2_XL_EN
    //  write_reg((reg_t)0x16, 0x01);  // LPF1_G_EN
    //  write_reg((reg_t)0x15, 0x61);  // ±2000 dps, LPF1 BW
    //  write_reg((reg_t)0x10, 0x06);  // ACC ODR = 120 Hz
    //  write_reg((reg_t)0x11, 0x06);  // GYRO ODR = 120 Hz

    // write_reg((reg_t) 0x12, 0x01); xmc_sleep_ms(50);    // SW_RESET
    // write_reg((reg_t) 0x12, 0x44);                   // BDU + IF_INC
    // write_reg((reg_t) 0x17, 0x40);                   // ±4g, LPF2 = ODR/20
    // write_reg((reg_t) 0x18, 0x08);                   // LPF2_XL_EN
    // write_reg((reg_t) 0x16, 0x01);                   // LPF1_G_EN
    // write_reg((reg_t) 0x15, 0x61);                   // ±2000 dps, LPF1 BW
    // write_reg((reg_t) 0x10, 0x06);                   // ACC ODR = 120 Hz
    // write_reg((reg_t) 0x11, 0x06);                   // GYRO ODR = 120 Hz
    // write_reg((reg_t) 0x01, 0x80);                   // FUNC_CFG_ACCESS on
    // write_reg((reg_t) 0x05, 0x00);                   // MLC/FSM aus
    // write_reg((reg_t) 0x04, 0x02);                   // SFLP Game Rotation
    // aktivieren write_reg((reg_t) 0x44, 0x01);                   // SFLP
    // Quaternion in FIFO write_reg((reg_t) 0x5E, 0x06);                   //
    // SFLP-Batch ODR = 120 Hz write_reg((reg_t) 0x66, 0x08); // SFLP Init
    // write_reg((reg_t) 0x01, 0x00);                   // FUNC_CFG_ACCESS aus
    // write_reg((reg_t) 0x09, 0x00);                   // Kein ACC/GYRO direkt
    // in FIFO write_reg((reg_t) 0x0A, 0x06);                   // FIFO Stream
    // Mode

    xmc_sleep_ms(10);

    inited = true;

    return XMC_OK;
  }

  xmc_status_t deinit() {
    XMC_ERR_RET(write_reg(REG_CTRL1, CTRL1_ODR_XL_POWER_DOWN));
    XMC_ERR_RET(write_reg(REG_CTRL2, CTRL2_ODR_G_POWER_DOWN));
    inited = false;
    return XMC_OK;
  }

  xmc_status_t read_sensor(float *values) {
    int16_t raw_values[7] = {0};
    XMC_ERR_RET(read_sensor_raw(raw_values));
    values[0] = raw_values[0] / 256.0f + 25.0f;
    // values[1] = raw_values[1] * (125.0f / 32768.0f);
    // values[2] = raw_values[2] * (125.0f / 32768.0f);
    // values[3] = raw_values[3] * (125.0f / 32768.0f);
    values[1] = raw_values[1] * (8.75f / 1000);
    values[2] = raw_values[2] * (8.75f / 1000);
    values[3] = raw_values[3] * (8.75f / 1000);
    // values[1] = raw_values[1] * (2000.0f / 32768.0f);
    // values[2] = raw_values[2] * (2000.0f / 32768.0f);
    // values[3] = raw_values[3] * (2000.0f / 32768.0f);
    // values[1] = raw_values[1] * 0.07f;
    // values[2] = raw_values[2] * 0.07f;
    // values[3] = raw_values[3] * 0.07f;
    //  values[4] = raw_values[4] * (2.0f / 32768.0f);
    //  values[5] = raw_values[5] * (2.0f / 32768.0f);
    //  values[6] = raw_values[6] * (2.0f / 32768.0f);
    values[4] = raw_values[4] * (4.0f / 32768.0f);
    values[5] = raw_values[5] * (4.0f / 32768.0f);
    values[6] = raw_values[6] * (4.0f / 32768.0f);
    // values[4] = raw_values[4] * (8.0f / 32768.0f);
    // values[5] = raw_values[5] * (8.0f / 32768.0f);
    // values[6] = raw_values[6] * (8.0f / 32768.0f);
    return XMC_OK;
  }

  xmc_status_t read_sensor_raw(int16_t *values) {
    if (!inited) {
      return XMC_USER_GENERIC_ERROR;
    }
    uint8_t buf[14] = {0};
    XMC_ERR_RET(read_reg(REG_OUT_TEMP_L, buf, sizeof(buf)));
    values[0] = (int16_t)(buf[0] | (buf[1] << 8));
    values[1] = (int16_t)(buf[2] | (buf[3] << 8));
    values[2] = (int16_t)(buf[4] | (buf[5] << 8));
    values[3] = (int16_t)(buf[6] | (buf[7] << 8));
    values[4] = (int16_t)(buf[8] | (buf[9] << 8));
    values[5] = (int16_t)(buf[10] | (buf[11] << 8));
    values[6] = (int16_t)(buf[12] | (buf[13] << 8));
    return XMC_OK;
  }

  xmc_status_t read_reg(reg_t reg, uint8_t *value, uint32_t num_bytes = 1) {
    xmc_status_t sts = XMC_OK;
    XMC_ERR_RET(xmc_i2c_lock());
    do {
      XMC_ERR_BRK(sts, xmc_i2c_set_baudrate(baudrate_hz));
      XMC_ERR_BRK(sts,
                  xmc_i2c_write_blocking(dev_addr, (uint8_t *)&reg, 1, false));
      XMC_ERR_BRK(sts,
                  xmc_i2c_read_blocking(dev_addr, value, num_bytes, false));
    } while (0);
    XMC_ERR_RET(xmc_i2c_unlock());
    return sts;
  }

  xmc_status_t write_reg(reg_t reg, uint8_t data) {
    return write_reg(reg, &data, 1);
  }

  xmc_status_t write_reg(reg_t reg, const uint8_t *value, uint32_t num_bytes) {
    xmc_status_t sts = XMC_OK;
    XMC_ERR_RET(xmc_i2c_lock());
    do {
      XMC_ERR_BRK(sts, xmc_i2c_set_baudrate(baudrate_hz));
      uint8_t buf[1 + num_bytes];
      buf[0] = (uint8_t)reg;
      for (uint32_t i = 0; i < num_bytes; i++) {
        buf[1 + i] = value[i];
      }
      XMC_ERR_BRK(sts,
                  xmc_i2c_write_blocking(dev_addr, buf, sizeof(buf), false));
    } while (0);
    XMC_ERR_RET(xmc_i2c_unlock());
    return sts;
  }
};

}  // namespace lsm6dsv16x

#endif  // LSM6DSV16X_HPP
