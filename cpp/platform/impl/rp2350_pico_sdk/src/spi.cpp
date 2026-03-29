#include "xmc/hw/spi.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/semaphore.hpp"

#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>

namespace xmc::spi {

static uint dma_tx;
static dma_channel_config dma_cfg;

static spi_inst_t *const spiBusInst = spi0;
static Semaphore *semaphore = nullptr;

static int csPin = -1;
static uint32_t baudrate = 10000000;

static bool isSpiShiftBusy();

uint32_t getPreferredFrequency(Chipset device) {
  switch (device) {
    case Chipset::DISPLAY: return 62500000;
    case Chipset::MEMORY_CARD: return 10000000;
    default: return 1000000;
  }
}

XmcStatus init() {
  // SPI init
  spi_init(spiBusInst, baudrate);
  gpio_set_function(XMC_PIN_SPI_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(XMC_PIN_SPI_MISO, GPIO_FUNC_SPI);
  gpio_set_function(XMC_PIN_SPI_SCK, GPIO_FUNC_SPI);

  // LCD control pins
  gpio_init(XMC_PIN_DISPLAY_CS);
  gpio_put(XMC_PIN_DISPLAY_CS, 1);
  gpio_set_dir(XMC_PIN_DISPLAY_CS, GPIO_OUT);

  gpio_init(XMC_PIN_DISPLAY_DC);
  gpio_put(XMC_PIN_DISPLAY_DC, 1);
  gpio_set_dir(XMC_PIN_DISPLAY_DC, GPIO_OUT);

  // TF card CS pin
  gpio_init(XMC_PIN_TFCARD_CS);
  gpio_put(XMC_PIN_TFCARD_CS, 1);
  gpio_set_dir(XMC_PIN_TFCARD_CS, GPIO_OUT);

  dma_tx = dma_claim_unused_channel(true);
  dma_cfg = dma_channel_get_default_config(dma_tx);
  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
  channel_config_set_read_increment(&dma_cfg, true);
  channel_config_set_write_increment(&dma_cfg, false);
  channel_config_set_dreq(&dma_cfg, spi_get_dreq(spiBusInst, true));

  if (!semaphore) {
    semaphore = new Semaphore();
    if (!semaphore->isInitialized()) {
      delete semaphore;
      semaphore = nullptr;
      XMC_ERR_RET(XMC_ERR_SEMAPHORE_INIT_FAILED);
    }
  }

  return XMC_OK;
}

void deinit() {
  dma_channel_unclaim(dma_tx);
  spi_deinit(spiBusInst);

  const int pins[] = {
      XMC_PIN_DISPLAY_CS, XMC_PIN_DISPLAY_DC, XMC_PIN_TFCARD_CS,
      XMC_PIN_SPI_MISO,   XMC_PIN_SPI_MOSI,   XMC_PIN_SPI_SCK,
  };
  const int num_pins = sizeof(pins) / sizeof(pins[0]);
  // drain charge from pins
  for (int i = 0; i < num_pins; i++) {
    gpio_init(pins[i]);
    gpio_set_dir(pins[i], GPIO_OUT);
    gpio_put(pins[i], 0);
  }
  // disable pins
  for (int i = 0; i < num_pins; i++) {
    gpio_deinit(pins[i]);
  }

  if (semaphore) {
    delete semaphore;
    semaphore = nullptr;
  }
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
  dmaComplete();
  baudrate = baud;
  spi_set_baudrate(spiBusInst, baudrate);
  return XMC_OK;
}

XmcStatus writeBlocking(const uint8_t *data, uint32_t size) {
  dmaComplete();
  int n = spi_write_blocking(spiBusInst, data, size);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_SPI_WRITE_FAILED);
  }
  return XMC_OK;
}

XmcStatus readBlocking(uint8_t repeated_byte, uint8_t *data, uint32_t size) {
  dmaComplete();
  int n = spi_read_blocking(spiBusInst, repeated_byte, data, size);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_SPI_READ_FAILED);
  }
  return XMC_OK;
}

XmcStatus dmaWriteStart(const dma::Config *cfg, int cs) {
  dmaComplete();

  dma_channel_transfer_size_t tx_size;
  switch (cfg->elementSize) {
    case 1: tx_size = DMA_SIZE_8; break;
    case 2: tx_size = DMA_SIZE_16; break;
    case 4: tx_size = DMA_SIZE_32; break;
    default: return XMC_ERR_DMA_BAD_ELEMENT_SIZE;
  }
  channel_config_set_transfer_data_size(&dma_cfg, tx_size);
  channel_config_set_read_increment(&dma_cfg, true);

  if (cs >= 0) {
    gpio::write(cs, 0);
  }
  csPin = cs;
  dma_channel_configure(dma_tx, &dma_cfg, &spi_get_hw(spiBusInst)->dr, cfg->ptr,
                        cfg->length, true);
  return XMC_OK;
}

XmcStatus dmaComplete() {
  while (dmaIsBusy()) {
    tight_loop_contents();
  }
  if (csPin >= 0) {
    gpio::write(csPin, 1);
    csPin = -1;
  }
  return XMC_OK;
}

bool dmaIsBusy() { return dma_channel_is_busy(dma_tx) || isSpiShiftBusy(); }

static bool isSpiShiftBusy() {
  return (spi_get_hw(spiBusInst)->sr & SPI_SSPSR_BSY_BITS) != 0;
}

}  // namespace xmc::spi
