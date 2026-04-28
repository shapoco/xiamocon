/**
 * @file i2c.h
 * @brief I2C hardware interface
 */

#ifndef XMC_HW_I2C_H
#define XMC_HW_I2C_H

#include "xmc/hw/hw_common.hpp"

#include <stdbool.h>
#include <stdint.h>

namespace xmc::i2c {

static constexpr int BUS_RESET_RETRY_COUNT = 3;

/**
 * Get the preferred I2C frequency for a given device. This can be used to set
 * the baud rate when initializing the I2C peripheral.
 * @param device The I2C device identifier.
 * @return The preferred I2C frequency in Hz.
 */
uint32_t getPreferredFrequency(Chipset device);

/**
 * Initialize the I2C peripheral and configure the GPIO pins for I2C
 * functionality. This must be called before any other I2C functions except
 * getPreferredFrequency.
 * @return XMC_OK if the I2C peripheral was successfully initialized.
 */
XmcStatus init();

/** Deinitialize the I2C peripheral and release any resources. */
void deinit();

/**
 * Try to start an I2C transaction without blocking. This can be used in
 * situations where blocking is not acceptable, such as in an interrupt handler
 * or a critical section. If the transaction cannot be started immediately
 * because another transaction is in progress, this function will return false.
 * Otherwise, it will return true and the caller can proceed with I2C
 * operations.
 * @return true if the transaction was successfully started, false otherwise.
 */
bool tryLock();

/**
 * Start an I2C transaction.
 *
 * This function must be called before accessing I2C devices or changing I2C
 * peripheral settings. I2C transactions from other tasks will be blocked until
 * unlock is called.
 *
 * @return XMC_OK if the transaction was successfully started.
 */
static inline XmcStatus lock() {
  while (!tryLock()) {
    tightLoopContents();
  }
  return XMC_OK;
}

/**
 * End an I2C transaction.
 *
 * This function must be called after accessing I2C devices or changing I2C
 * peripheral settings.
 */
XmcStatus unlock();

/**
 * Set the I2C baud rate.
 * @param baudrate The desired I2C baud rate in Hz. The actual baud rate may be
 * adjusted to the nearest supported value.
 */
XmcStatus setBaudrate(uint32_t baudrate);

/**
 * Reset the I2C bus by manually toggling the SCL line until the SDA line is
 * released by any stuck slave devices. This can be used to recover from a
 * bus lockup condition.
 * @return XMC_OK if the bus was successfully reset, or an error code if the
 * reset failed.
 */
XmcStatus resetBus();

/**
 * Write data to an I2C device in a blocking manner.
 * @param devAddr The 7-bit I2C address of the target device.
 * @param data Pointer to the data buffer to be sent.
 * @param size The number of bytes to write from the data buffer.
 * @param nostop If true, do not send a STOP condition after the write.
 */
XmcStatus writeBlocking(uint8_t devAddr, const uint8_t *data, uint32_t size,
                        bool nostop);

/**
 * Read data from an I2C device in a blocking manner.
 * @param devAddr The 7-bit I2C address of the target device.
 * @param data Pointer to the data buffer to store the received data.
 * @param size The number of bytes to read into the data buffer.
 * @param nostop If true, do not send a STOP condition after the read.
 */
XmcStatus readBlocking(uint8_t devAddr, uint8_t *data, uint32_t size,
                       bool nostop);

}  // namespace xmc::i2c

#endif
