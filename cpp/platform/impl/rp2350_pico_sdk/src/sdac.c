#include "xmc/hw/sdac.h"
#include "xmc/hw/dma_irq.h"
#include "xmc/hw/pins.h"
#include "xmc/hw/ram.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>

typedef struct {
  xmc_sdac_config_t cfg;
  xmc_stream_source_t *source;
  int pwm_slice;
  int pwm_channel;
  int dma_ch;
  int next_write_bank;
  int next_read_bank;
  uint8_t *source_buff;
  uint32_t *dma_buff;
  int16_t error_accum;
  uint32_t lfsr;
} xmc_sdac_hw_t;

static void start_next_dma(xmc_sdac_inst_t *inst);
static void fill_buffer(xmc_sdac_inst_t *inst);
static void dma_handler(void *context);

static inline void update_lfsr(uint32_t *lfsr) {
  uint32_t bit =
      ((*lfsr >> 0) ^ (*lfsr >> 10) ^ (*lfsr >> 30) ^ (*lfsr >> 31)) & 1;
  *lfsr = (*lfsr >> 1) | (bit << 31);
}

#define SUPPORTED_FORMATS \
  (XMC_SAMPLE_LINEAR_PCM_U8_MONO | XMC_SAMPLE_LINEAR_PCM_U16_MONO)

xmc_sample_format_t xmc_sdac_get_supported_formats(void) {
  return SUPPORTED_FORMATS;
}

xmc_status_t xmc_sdac_init(xmc_sdac_inst_t *inst, int pin,
                           const xmc_sdac_config_t *cfg,
                           float *actual_rate_hz) {
  xmc_sdac_hw_t *hw = malloc(sizeof(xmc_sdac_hw_t));
  if (!hw) {
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  inst->hw = hw;
  inst->pin = pin;

  hw->cfg = *cfg;
  hw->error_accum = 0;
  hw->lfsr = 0xFFFFFFFF;

  if ((cfg->format.sample_format & SUPPORTED_FORMATS) == 0) {
    XMC_ERR_RET(XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT);
  }

  int bytes_per_sample =
      xmc_audio_get_bytes_per_sample(hw->cfg.format.sample_format);
  hw->source_buff = malloc(hw->cfg.latency_samples * bytes_per_sample);
  if (!hw->source_buff) {
    xmc_sdac_deinit(inst);
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  hw->dma_buff = xmc_malloc(hw->cfg.latency_samples * sizeof(uint32_t) * 2,
                            XMC_RAM_CAP_DMA);
  if (!hw->dma_buff) {
    xmc_sdac_deinit(inst);
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  hw->next_write_bank = 0;
  hw->next_read_bank = 0;

  gpio_set_function(inst->pin, GPIO_FUNC_PWM);
  gpio_set_dir(inst->pin, GPIO_OUT);

  const int PWM_PERIOD = 256;
  int sys_clk_freq = clock_get_hz(clk_sys);
  float pwm_clkdiv =
      (float)sys_clk_freq / (hw->cfg.format.sample_rate_hz * PWM_PERIOD);
  hw->pwm_slice = pwm_gpio_to_slice_num(inst->pin);
  hw->pwm_channel = pwm_gpio_to_channel(inst->pin);
  pwm_set_gpio_level(inst->pin, 0);

  pwm_config pwm_cfg = pwm_get_default_config();
  pwm_config_set_clkdiv(&pwm_cfg, pwm_clkdiv);
  pwm_config_set_wrap(&pwm_cfg, PWM_PERIOD - 1);
  pwm_init(hw->pwm_slice, &pwm_cfg, true);

  hw->dma_ch = dma_claim_unused_channel(true);
  if (hw->dma_ch < 0) {
    xmc_sdac_deinit(inst);
    return XMC_ERR_DMA_INIT_FAILED;
  }
  dma_channel_set_irq0_enabled(hw->dma_ch, true);
  xmc_dma_register_irq_handler(hw->dma_ch, dma_handler, inst);
  irq_set_exclusive_handler(DMA_IRQ_0, xmc_dma_irq_handler);
  // irq_add_shared_handler(DMA_IRQ_0, xmc_dma_irq_handler,
  // PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  irq_set_enabled(DMA_IRQ_0, true);

  dma_channel_config dma_cfg = dma_channel_get_default_config(hw->dma_ch);
  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
  channel_config_set_read_increment(&dma_cfg, true);
  channel_config_set_write_increment(&dma_cfg, false);
  channel_config_set_dreq(&dma_cfg, DREQ_PWM_WRAP0 + hw->pwm_slice);
  dma_channel_configure(hw->dma_ch, &dma_cfg, &pwm_hw->slice[hw->pwm_slice].cc,
                        hw->dma_buff, hw->cfg.latency_samples, false);

  fill_buffer(inst);
  fill_buffer(inst);
  start_next_dma(inst);

  return XMC_OK;
}

xmc_status_t xmc_sdac_deinit(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;
  if (hw) {
    if (hw->dma_ch >= 0) {
      dma_channel_wait_for_finish_blocking(hw->dma_ch);
      dma_channel_unclaim(hw->dma_ch);
      dma_channel_set_irq0_enabled(hw->dma_ch, false);
      xmc_dma_unregister_irq_handler(hw->dma_ch);
      irq_set_enabled(DMA_IRQ_0, false);
      hw->dma_ch = -1;
    }
    if (hw->source_buff) {
      free(hw->source_buff);
      hw->source_buff = NULL;
    }
    if (hw->dma_buff) {
      xmc_free(hw->dma_buff);
      hw->dma_buff = NULL;
    }
    pwm_set_enabled(hw->pwm_slice, false);
    free(hw);
    inst->hw = NULL;
  }

  // drain charge from pin
  gpio_init(inst->pin);
  gpio_set_dir(inst->pin, GPIO_OUT);
  gpio_put(inst->pin, 0);

  // disable pin
  gpio_deinit(inst->pin);
  return XMC_OK;
}

xmc_status_t xmc_sdac_set_source(xmc_sdac_inst_t *inst,
                                 xmc_stream_source_t *src) {
  inst->source = *src;
  return XMC_OK;
}

xmc_status_t xmc_sdac_service(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;
  if (hw->next_read_bank == hw->next_write_bank) {
    fill_buffer(inst);
  }
  return XMC_OK;
}

static void dma_handler(void *context) {
  xmc_sdac_inst_t *inst = (xmc_sdac_inst_t *)context;
  start_next_dma(inst);
}

static void fill_buffer(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;

  uint32_t *dst =
      hw->dma_buff + (hw->next_write_bank * hw->cfg.latency_samples);

  if (inst->source.request_data) {
    inst->source.request_data(hw->source_buff, hw->cfg.latency_samples,
                              inst->source.context);
    switch (hw->cfg.format.sample_format) {
      default:
      case XMC_SAMPLE_LINEAR_PCM_U8_MONO: {
        if (hw->pwm_channel == 0) {
          for (int i = 0; i < hw->cfg.latency_samples; i++) {
            dst[i] = (uint32_t)hw->source_buff[i];
          }
        } else {
          for (int i = 0; i < hw->cfg.latency_samples; i++) {
            dst[i] = (uint32_t)hw->source_buff[i] << 16;
          }
        }
      } break;
      case XMC_SAMPLE_LINEAR_PCM_U16_MONO: {
        uint16_t *src = (uint16_t *)hw->source_buff;
        int32_t e = hw->error_accum;
        for (int i = 0; i < hw->cfg.latency_samples; i++) {
          int32_t raw = src[i];
          int32_t out = raw;
          const int32_t ANTIALIAS_FACTOR = (1 << 1);
          const int32_t k = hw->lfsr & (ANTIALIAS_FACTOR - 1);
          update_lfsr(&hw->lfsr);
          out += e * (ANTIALIAS_FACTOR - k) / ANTIALIAS_FACTOR;
          e = e * k / ANTIALIAS_FACTOR;
          out += 0x80;
          out >>= 8;
          if (out < 0) {
            out = 0;
          } else if (out > 0xFF) {
            out = 0xFF;
          }
          e += raw - (out << 8);
          if (hw->pwm_channel == 0) {
            dst[i] = out;
          } else {
            dst[i] = out << (hw->pwm_channel * 16);
          }
        }
        hw->error_accum = e;
      } break;
    }
  } else {
    uint32_t sample = (uint32_t)0x80 << (hw->pwm_channel * 16);
    for (int i = 0; i < hw->cfg.latency_samples; i++) {
      dst[i] = sample;
    }
  }
  hw->next_write_bank = (hw->next_write_bank + 1) % 2;
}

static void start_next_dma(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;

  dma_channel_set_read_addr(
      hw->dma_ch, hw->dma_buff + (hw->next_read_bank * hw->cfg.latency_samples),
      true);
  hw->next_read_bank = (hw->next_read_bank + 1) % 2;
}
