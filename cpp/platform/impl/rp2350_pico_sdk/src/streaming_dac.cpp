#include "xmc/hw/streaming_dac.hpp"
#include "xmc/hw/dma_irq.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/ram.hpp"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <string.h>

namespace xmc::audio {

static const uint32_t PDM_MIN_PDM_FREQ_HZ = 3000000;

struct SdacHwRp {
  SdacConfig cfg;
  SourcePort *source;
  PIO pio;
  int pioSm;
  int pioOffset;
  int dmaCh;
  int nextWriteBank;
  int nextReadBank;
  uint8_t *srcFmtBuff;
  int16_t *s16Buff;
  uint32_t *dmaBuff;
  uint32_t extraOversample;
  int32_t pdmLastInput;
  int32_t pdmLastQt;
  int32_t pdmWork0;
  int32_t pdmWork1;
  uint32_t pdmLfsr;
};

static const uint16_t pioInsts[] = {
    0x6001,
};

static const struct pio_program pioProgram = {
    .instructions = pioInsts,
    .length = 1,
    .origin = -1,
};

static void startNextDma(StreamingDac &inst);
static void fillBuffer(StreamingDac &inst);
static void dmaHandlerFast(void *context);
static void dmaHandlerSlow(void *context);

static XMC_INLINE void updateLfsr(uint32_t *lfsr) {
  uint32_t bit =
      ((*lfsr >> 0) ^ (*lfsr >> 10) ^ (*lfsr >> 30) ^ (*lfsr >> 31)) & 1;
  *lfsr = (*lfsr >> 1) | (bit << 31);
}

static constexpr SampleFormat SUPPORTED_FORMATS =
    (SampleFormat::LINEAR_PCM_U8_MONO | SampleFormat::LINEAR_PCM_S16_MONO);

SampleFormat sdacGetSupportedFormats(void) { return SUPPORTED_FORMATS; }

uint32_t getPreferredSamplingRate(void) {
  // Select a frequency that minimizes aliasing around 20-25kHz
  return 20833;  // = 250MHz / 32 / 375
}

StreamingDac::StreamingDac(int pin) : pin(pin) {
  handle = malloc(sizeof(SdacHwRp));
  memset(handle, 0, sizeof(SdacHwRp));
}

StreamingDac::~StreamingDac() {
  if (handle) {
    free(handle);
    handle = nullptr;
  }
}

XmcStatus StreamingDac::start(const SdacConfig &cfg, float *actualRateHz) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);

  SdacHwRp *hw = (SdacHwRp *)handle;
  hw->cfg = cfg;
  hw->pdmWork0 = 0;
  hw->pdmWork1 = 0;
  hw->pdmLastInput = 0;
  hw->pdmLfsr = 0xFFFFFFFF;

  uint32_t sysClkFreq = clock_get_hz(clk_sys);
  uint32_t pdmClkFreq = hw->cfg.format.rateHz * 32;
  hw->extraOversample = (PDM_MIN_PDM_FREQ_HZ + pdmClkFreq - 1) / pdmClkFreq;
  if (hw->extraOversample < 1) hw->extraOversample = 1;
  pdmClkFreq *= hw->extraOversample;

  *actualRateHz = (float)pdmClkFreq / hw->extraOversample / 32;

  if (!(cfg.format.sampleFormat & SUPPORTED_FORMATS)) {
    XMC_ERR_RET(XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT);
  }

  if (cfg.format.sampleFormat != SampleFormat::LINEAR_PCM_S16_MONO) {
    int bytesPerSample = getBytesPerSample(hw->cfg.format.sampleFormat);
    hw->srcFmtBuff = (uint8_t *)malloc(hw->cfg.latencySamples * bytesPerSample);
    if (!hw->srcFmtBuff) {
      stop();
      return XMC_ERR_RAM_ALLOC_FAILED;
    }
  } else {
    hw->srcFmtBuff = NULL;
  }

  hw->s16Buff = (int16_t *)malloc(hw->cfg.latencySamples * sizeof(int16_t));
  if (!hw->s16Buff) {
    stop();
    return XMC_ERR_RAM_ALLOC_FAILED;
  }

  uint32_t dmaBuffSize =
      hw->cfg.latencySamples * hw->extraOversample * sizeof(uint32_t) * 2;
  hw->dmaBuff = (uint32_t *)xmcMalloc(dmaBuffSize, XMC_RAM_CAP_DMA);
  if (!hw->dmaBuff) {
    stop();
    return XMC_ERR_RAM_ALLOC_FAILED;
  }

  hw->nextWriteBank = 0;
  hw->nextReadBank = 0;

  gpio_set_function(pin, GPIO_FUNC_PWM);
  gpio_set_dir(pin, GPIO_OUT);

  hw->pio = pio0;
  hw->pioOffset = pio_add_program(hw->pio, &pioProgram);
  hw->pioSm = pio_claim_unused_sm(hw->pio, true);

  pio_sm_config pioCfg = pio_get_default_sm_config();
  sm_config_set_wrap(&pioCfg, hw->pioOffset + 0, hw->pioOffset + 0);
  sm_config_set_out_pins(&pioCfg, pin, 1);
  sm_config_set_fifo_join(&pioCfg, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&pioCfg, true, true, 32);
  sm_config_set_clkdiv(&pioCfg, (float)sysClkFreq / pdmClkFreq);
  pio_sm_init(hw->pio, hw->pioSm, hw->pioOffset, &pioCfg);

  pio_gpio_init(hw->pio, pin);
  pio_sm_set_consecutive_pindirs(hw->pio, hw->pioSm, pin, 1, true);
  pio_sm_set_enabled(hw->pio, hw->pioSm, true);

  hw->dmaCh = dma_claim_unused_channel(true);
  if (hw->dmaCh < 0) {
    stop();
    return XMC_ERR_DMA_INIT_FAILED;
  }
  dma_channel_set_irq0_enabled(hw->dmaCh, true);
  dma::registerIrqHandler(hw->dmaCh, dmaHandlerFast, dmaHandlerSlow, this);
  irq_set_exclusive_handler(DMA_IRQ_0, xmcDmaIrqHandler);
  // irq_add_shared_handler(DMA_IRQ_0, dma::xmcDmaIrqHandler,
  // PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  irq_set_enabled(DMA_IRQ_0, true);

  dma_channel_config dma_cfg = dma_channel_get_default_config(hw->dmaCh);
  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
  channel_config_set_read_increment(&dma_cfg, true);
  channel_config_set_write_increment(&dma_cfg, false);
  channel_config_set_dreq(&dma_cfg, pio_get_dreq(hw->pio, hw->pioSm, true));
  dma_channel_configure(hw->dmaCh, &dma_cfg, &hw->pio->txf[hw->pioSm],
                        hw->dmaBuff,
                        hw->cfg.latencySamples * hw->extraOversample, false);

  fillBuffer(*this);
  fillBuffer(*this);
  startNextDma(*this);

  return XMC_OK;
}

XmcStatus StreamingDac::stop() {
  if (handle) {
    SdacHwRp *hw = (SdacHwRp *)handle;
    if (hw->dmaCh >= 0) {
      dma_channel_wait_for_finish_blocking(hw->dmaCh);
      dma_channel_unclaim(hw->dmaCh);
      dma_channel_set_irq0_enabled(hw->dmaCh, false);
      dma::unregisterIrqHandler(hw->dmaCh);
      irq_set_enabled(DMA_IRQ_0, false);
      hw->dmaCh = -1;
    }
    if (hw->srcFmtBuff) {
      free(hw->srcFmtBuff);
      hw->srcFmtBuff = NULL;
    }
    if (hw->s16Buff) {
      free(hw->s16Buff);
      hw->s16Buff = NULL;
    }
    if (hw->dmaBuff) {
      xmcFree(hw->dmaBuff);
      hw->dmaBuff = NULL;
    }
    pio_sm_set_enabled(hw->pio, hw->pioSm, false);
    pio_sm_unclaim(hw->pio, hw->pioSm);
    pio_remove_program(hw->pio, &pioProgram, hw->pioOffset);
  }

  // drain charge from pin
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, 0);

  // disable pin
  gpio_deinit(pin);
  return XMC_OK;
}

StreamFormat StreamingDac::getStreamFormat() const {
  if (!handle) return {};
  SdacHwRp *hw = (SdacHwRp *)handle;
  return hw->cfg.format;
}

XmcStatus StreamingDac::setSource(SourcePort *src) {
  this->source = *src;
  return XMC_OK;
}

XmcStatus StreamingDac::service() {
  SdacHwRp *hw = (SdacHwRp *)handle;
  if (hw->nextReadBank == hw->nextWriteBank) {
    fillBuffer(*this);
  }
  return XMC_OK;
}

static void dmaHandlerFast(void *context) {
  startNextDma(*(StreamingDac *)context);
}

static void dmaHandlerSlow(void *context) {
  fillBuffer(*(StreamingDac *)context);
}

static void fillBuffer(StreamingDac &inst) {
  SdacHwRp *hw = (SdacHwRp *)inst.handle;

  uint32_t dstSamples = hw->cfg.latencySamples * hw->extraOversample;
  uint32_t *dst = hw->dmaBuff + (hw->nextWriteBank * dstSamples);

  if (inst.source.requestData) {
    uint32_t buffSizeBytes =
        hw->cfg.latencySamples * getBytesPerSample(hw->cfg.format.sampleFormat);
    switch (hw->cfg.format.sampleFormat) {
      default:
      case SampleFormat::LINEAR_PCM_S16_MONO:
        memset(hw->s16Buff, 0x00, buffSizeBytes);
        inst.source.requestData(hw->s16Buff, hw->cfg.latencySamples,
                                inst.source.context);
        break;
      case SampleFormat::LINEAR_PCM_U8_MONO:
        memset(hw->srcFmtBuff, 0x80, buffSizeBytes);
        inst.source.requestData(hw->srcFmtBuff, hw->cfg.latencySamples,
                                inst.source.context);
        for (uint32_t i = 0; i < hw->cfg.latencySamples; i++) {
          hw->s16Buff[i] = ((int16_t)hw->srcFmtBuff[i] - 128) << 8;
        }
        break;
    }

    int16_t *src = (int16_t *)hw->s16Buff;
    int32_t lastInput = hw->pdmLastInput;
    int32_t lastQt = hw->pdmLastQt;
    for (uint32_t isrc = 0; isrc < hw->cfg.latencySamples; isrc++) {
      int32_t newInput = src[isrc];
      for (uint32_t ieos = 0; ieos < hw->extraOversample; ieos++) {
        uint32_t pdmOutput = 0;
#if 0
        int32_t overSample = (newInput * j + lastInput * (32 - j)) / 32;
#else
        int32_t overSample = newInput;
#endif
        updateLfsr(&(hw->pdmLfsr));
        int32_t dither = (int32_t)(hw->pdmLfsr & 0xF);
        overSample += dither;
        for (int ibit = 0; ibit < 32; ibit++) {
          hw->pdmWork0 += overSample - lastQt;
          hw->pdmWork1 += hw->pdmWork0 - lastQt;

          pdmOutput >>= 1;
          if (hw->pdmWork1 >= 0) {
            pdmOutput |= 0x80000000;
            lastQt = 0x10000;
          } else {
            pdmOutput &= 0x7FFFFFFF;
            lastQt = -0x10000;
          }
        }
        lastInput = newInput;
        dst[isrc * hw->extraOversample + ieos] = pdmOutput;
      }
      hw->pdmLastInput = lastInput;
      hw->pdmLastQt = lastQt;
    }
  } else {
    uint32_t sample = 0x55555555;
    for (uint32_t i = 0; i < dstSamples; i++) {
      dst[i] = sample;
    }
  }
  hw->nextWriteBank = (hw->nextWriteBank + 1) % 2;
}

static void startNextDma(StreamingDac &inst) {
  SdacHwRp *hw = (SdacHwRp *)inst.handle;

  uint32_t dstSamples = hw->cfg.latencySamples * hw->extraOversample;
  dma_channel_set_read_addr(
      hw->dmaCh, hw->dmaBuff + (hw->nextReadBank * dstSamples), true);
  hw->nextReadBank = (hw->nextReadBank + 1) % 2;
}

}  // namespace xmc::audio
