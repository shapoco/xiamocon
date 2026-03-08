#include "xmc/hw/spi.h"
#include "xmc/hw/gpio.h"
#include "xmc/hw/lock.h"
#include "xmc/hw/pins.h"

#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>

static uint dma_tx;
static dma_channel_config dma_cfg;

static spi_inst_t *const spibus_inst = spi0;
static xmc_semaphore_t semaphore;
static volatile bool in_transaction = false;

static int cs_pin = -1;
static uint32_t baudrate = 10000000;

static bool is_spi_shift_busy();

uint32_t xmc_spi_get_preferred_frequency(xmc_spi_device_t device) {
  switch (device) {
    case XMC_SPI_DEV_DISPLAY: return 62500000;
    case XMC_SPI_DEV_TFCARD: return 10000000;
    default: return 1000000;
  }
}

xmc_status_t xmc_spi_init() {
  // SPI init
  spi_init(spibus_inst, baudrate);
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
  channel_config_set_dreq(&dma_cfg, spi_get_dreq(spibus_inst, true));

  XMC_ERR_RET(xmc_semaphore_init(&semaphore));

  return XMC_OK;
}

void xmc_spi_deinit() {
  xmc_spi_end_transaction();
  dma_channel_unclaim(dma_tx);
  spi_deinit(spibus_inst);

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

  xmc_semaphore_deinit(&semaphore);
}

xmc_status_t xmc_spi_begin_transaction() {
  if (in_transaction) return XMC_OK;
  xmc_semaphore_take(&semaphore);
  in_transaction = true;
  return XMC_OK;
}

xmc_status_t xmc_spi_end_transaction() {
  if (!in_transaction) return XMC_OK;
  in_transaction = false;
  xmc_status_t ret = xmc_spi_dma_complete();
  xmc_semaphore_give(&semaphore);
  return ret;
}

xmc_status_t xmc_spi_set_baudrate(uint32_t baud) {
  if (baud == baudrate) return XMC_OK;
  xmc_spi_dma_complete();
  baudrate = baud;
  spi_set_baudrate(spibus_inst, baudrate);
  return XMC_OK;
}

xmc_status_t xmc_spi_write_blocking(const uint8_t *data, uint32_t size) {
  xmc_spi_dma_complete();
  int n = spi_write_blocking(spibus_inst, data, size);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_SPI_WRITE_FAILED);
  }
  return XMC_OK;
}

xmc_status_t xmc_spi_read_blocking(uint8_t repeated_byte, uint8_t *data,
                                   uint32_t size) {
  xmc_spi_dma_complete();
  int n = spi_read_blocking(spibus_inst, repeated_byte, data, size);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_SPI_READ_FAILED);
  }
  return XMC_OK;
}

xmc_status_t xmc_spi_dma_write_start(const xmc_dma_config_t *cfg, int cs) {
  xmc_spi_dma_complete();

  dma_channel_transfer_size_t tx_size;
  switch (cfg->element_size) {
    case 1: tx_size = DMA_SIZE_8; break;
    case 2: tx_size = DMA_SIZE_16; break;
    case 4: tx_size = DMA_SIZE_32; break;
    default: return XMC_ERR_DMA_BAD_ELEMENT_SIZE;
  }
  channel_config_set_transfer_data_size(&dma_cfg, tx_size);
  channel_config_set_read_increment(&dma_cfg, true);

  if (cs >= 0) {
    xmc_gpio_write(cs, 0);
  }
  cs_pin = cs;
  dma_channel_configure(dma_tx, &dma_cfg, &spi_get_hw(spibus_inst)->dr,
                        cfg->ptr, cfg->length, true);
  return XMC_OK;
}

xmc_status_t xmc_spi_dma_complete() {
  while (xmc_spi_dma_is_busy()) {
    tight_loop_contents();
  }
  if (cs_pin >= 0) {
    xmc_gpio_write(cs_pin, 1);
    cs_pin = -1;
  }
  return XMC_OK;
}

bool xmc_spi_dma_is_busy() {
  return dma_channel_is_busy(dma_tx) || is_spi_shift_busy();
}

static bool is_spi_shift_busy() {
  return (spi_get_hw(spibus_inst)->sr & SPI_SSPSR_BSY_BITS) != 0;
}
