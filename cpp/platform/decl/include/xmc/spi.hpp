/**
 * @file spi.h
 * @brief SPI hardware interface
 */

#ifndef XMC_HW_SPI_H
#define XMC_HW_SPI_H

#include "xmc/hw/dma.hpp"
#include "xmc/hw/hw_common.hpp"

#include <stdbool.h>
#include <stdint.h>

namespace xmc::spi {

/**
 * Get the preferred SPI frequency for a given device. This can be used to set
 * the baud rate when initializing the SPI peripheral.
 * @param device The SPI device identifier.
 * @return The preferred SPI frequency in Hz.
 */
uint32_t getPreferredFrequency(Chipset device);

/**
 * Initialize the SPI peripheral and configure the GPIO pins for SPI
 * functionality. This must be called before any other SPI functions except
 * getPreferredFrequency.
 * @return XMC_OK if the SPI peripheral was successfully initialized.
 */
XmcStatus init();

/**
 * Deinitialize the SPI peripheral and release any resources.
 */
void deinit();

/**
 * Try to begin an SPI transaction without blocking. This will attempt to
 * acquire the lock for starting an SPI transaction, but will return immediately
 * if the lock is not available.
 * @return True if the transaction was successfully started, false otherwise.
 */
bool tryLock();

/**
 * Begin an SPI transaction. This will acquire a lock to prevent other tasks
 * from starting SPI transactions until unlock is called.
 * @return XMC_OK if the transaction was successfully started.
 */
static inline XmcStatus lock() {
  while (!tryLock()) {
    tightLoopContents();
  }
  return XMC_OK;
}

/**
 * End an SPI transaction. This will release the lock acquired by
 * lock.
 * @return XMC_OK if the transaction was successfully ended.
 */
XmcStatus unlock();

/**
 * Set the SPI baud rate.
 * @param baudrate The desired SPI baud rate in Hz.
 */
XmcStatus setBaudrate(uint32_t baudrate);

/**
 * Write data to the SPI bus in a blocking manner.
 * @param data Pointer to the data buffer to be sent.
 * @param size The number of bytes to write from the data buffer.
 */
XmcStatus writeBlocking(const uint8_t *data, uint32_t size);

/**
 * Read data from the SPI bus in a blocking manner. The master will send the
 * specified byte repeatedly while reading.
 * @param repeated_byte The byte to send repeatedly while reading.
 * @param data Pointer to the data buffer to store the received data.
 * @param size The number of bytes to read into the data buffer.
 */
XmcStatus readBlocking(uint8_t repeated_byte, uint8_t *data, uint32_t size);

/**
 * Start a DMA-based SPI write operation. This will return immediately after
 * starting the DMA transfer, and the caller must call dmaComplete() to
 * wait for the transfer to finish.
 * @param cfg Pointer to the DMA configuration for the transfer.
 * @param csPin The GPIO pin number to use for the chip select (CS) line during
 * the transfer, or -1 to not control any CS line. If a valid pin number is
 * provided, the CS line will be pulled low at the start of the transfer and
 * pulled high when the transfer is complete.
 * @return XMC_OK if the DMA transfer was successfully started, or an error code
 * if there was a problem with the configuration.
 */
XmcStatus dmaWriteStart(const dma::Config *cfg, int csPin);

/**
 * Wait for any ongoing DMA-based SPI transfer to complete. If a CS pin was
 * specified in dmaWriteStart, it will be released (set high) after
 * the transfer is complete.
 * @return XMC_OK if the transfer completed successfully.
 */
XmcStatus dmaComplete();

/**
 * Check if a DMA-based SPI transfer is currently in progress.
 * @return True if a DMA-based SPI transfer is in progress, false otherwise.
 */
bool dmaIsBusy();

}  // namespace xmc::spi

#endif
