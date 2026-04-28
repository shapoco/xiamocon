#include "xmc/spi.hpp"
#include "xmc/gpio.hpp"
#include "xmc/pins.hpp"
#include "xmc/semaphore.hpp"

#include <Arduino.h>

// undefine DISPLAY to avoid conflict with macro in Arduino.h
#undef DISPLAY

#include <ESP32DMASPIMaster.h>

namespace xmc::spi {

static constexpr size_t TRANSFER_SIZE = 16 * 1024;

static constexpr size_t QUEUE_SIZE = (2 * 256 * 256) / TRANSFER_SIZE;

ESP32DMASPI::Master spiDma;
static Semaphore *semaphore = nullptr;

static int csPin = -1;
static uint32_t baudrate = 10000000;

bool dmaInited = false;

uint8_t *txBuff = nullptr;
uint8_t *rxBuff = nullptr;

static void initDma();
static void deinitDma();

uint32_t getPreferredFrequency(Chipset device) {
  switch (device) {
    case Chipset::DISPLAY: return 60000000;
    case Chipset::MEMORY_CARD: return 10000000;
    default: return 1000000;
  }
}

XmcStatus init() {
  csPin = -1;
  txBuff = spiDma.allocDMABuffer(TRANSFER_SIZE);
  rxBuff = spiDma.allocDMABuffer(TRANSFER_SIZE);
  initDma();
  return XMC_OK;
}

void deinit() {
  deinitDma();
  if (txBuff) free(txBuff);
  if (rxBuff) free(rxBuff);
}

bool tryLock() {
  if (!semaphore) return true;
  return semaphore->tryTake();
}

XmcStatus unlock() {
  XmcStatus ret = dmaComplete();
  if (semaphore) semaphore->give();
  return ret;
}

XmcStatus setBaudrate(uint32_t baud) {
  if (baud == baudrate) return XMC_OK;
  baudrate = baud;
  deinitDma();
  initDma();
  return XMC_OK;
}

XmcStatus writeBlocking(const uint8_t *data, uint32_t size) {
  dma::Config dmaCfg = {
      .ptr = const_cast<uint8_t *>(data),
      .elementSize = 1,
      .length = size,
  };
  dmaWriteStart(&dmaCfg, -1);
  dmaComplete();
  return XMC_OK;
}

XmcStatus readBlocking(uint8_t repeated_byte, uint8_t *data, uint32_t size) {
  dmaComplete();
  initDma();
  while (size > 0) {
    size_t chunkSize = size < TRANSFER_SIZE ? size : TRANSFER_SIZE;
    memset(txBuff, repeated_byte, chunkSize);
    spiDma.queue(txBuff, rxBuff, chunkSize);
    spiDma.wait();
    memcpy(data, rxBuff, chunkSize);
    data += chunkSize;
    size -= chunkSize;
  }
  return XMC_OK;
}

XmcStatus dmaWriteStart(const dma::Config *cfg, int cs) {
  dmaComplete();
  initDma();
  if (cs >= 0) {
    gpio::write(cs, 0);
  }
  csPin = cs;
  uint8_t *data = (uint8_t *)cfg->ptr;
  size_t size = cfg->elementSize * cfg->length;
  while (size > 0) {
    size_t chunkSize = size < TRANSFER_SIZE ? size : TRANSFER_SIZE;
    spiDma.queue(data, nullptr, chunkSize);
    data += chunkSize;
    size -= chunkSize;
  }
  spiDma.trigger();
  return XMC_OK;
}

XmcStatus dmaComplete() {
  while (dmaIsBusy()) {
    tightLoopContents();
  }
  if (csPin >= 0) {
    gpio::write(csPin, 1);
    csPin = -1;
  }
  return XMC_OK;
}

bool dmaIsBusy() { return dmaInited && spiDma.numTransactionsInFlight() > 0; }

static void initDma() {
  if (dmaInited) return;
  spiDma.setDataMode(SPI_MODE0);
  spiDma.setFrequency(baudrate);
  spiDma.setMaxTransferSize(TRANSFER_SIZE);
  spiDma.setQueueSize(QUEUE_SIZE);
  spiDma.begin(FSPI, XMC_PIN_SPI_SCK, XMC_PIN_SPI_MISO, XMC_PIN_SPI_MOSI,
               -1);
  dmaInited = true;
}

static void deinitDma() {
  if (!dmaInited) return;
  dmaComplete();
  spiDma.end();
  dmaInited = false;
}

}  // namespace xmc::spi
