#include "xmc/streaming_dac.hpp"
#include "xmc/gpio.hpp"
#include "xmc/pins.hpp"
#include "xmc/heap.hpp"

#include <driver/i2s_pdm.h>
#include <string.h>

namespace xmc::audio {

struct SdacHwEsp {
  SdacConfig cfg;
  SourcePort *source;
  i2s_chan_handle_t audio_i2s_ch;
  uint32_t writePos;
  uint32_t readPos;
  bool full;
  uint8_t *srcFmtBuff;
  int16_t *s16Buff;
};

static void maintain(StreamingDac &inst, bool preload);
static void fillBuffer(StreamingDac &inst, uint32_t fillSamples);
static void pdmWrite(StreamingDac &inst, uint32_t numSamples, bool preload);

static constexpr SampleFormat SUPPORTED_FORMATS =
    (SampleFormat::LINEAR_PCM_U8_MONO | SampleFormat::LINEAR_PCM_S16_MONO);

SampleFormat sdacGetSupportedFormats(void) { return SUPPORTED_FORMATS; }

uint32_t getPreferredSamplingRate(void) { return 24000; }

StreamingDac::StreamingDac(int pin) : pin(pin) {
  handle = xmcMalloc(sizeof(SdacHwEsp), XMC_HEAP_CAP_DMA);
  memset(handle, 0, sizeof(SdacHwEsp));
}

StreamingDac::~StreamingDac() {
  if (handle) {
    xmcFree(handle);
    handle = nullptr;
  }
}

XmcStatus StreamingDac::start(const SdacConfig &cfg, float *actualRateHz) {
  esp_err_t err;

  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);

  SdacHwEsp *hw = (SdacHwEsp *)handle;
  hw->cfg = cfg;
  hw->readPos = 0;
  hw->writePos = 0;
  hw->full = false;
  hw->source = nullptr;
  hw->srcFmtBuff = nullptr;
  hw->s16Buff = nullptr;

  if (!(cfg.format.sampleFormat & SUPPORTED_FORMATS)) {
    XMC_ERR_RET(XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT);
  }

  if (cfg.format.sampleFormat != SampleFormat::LINEAR_PCM_S16_MONO) {
    int bytesPerSample = getBytesPerSample(hw->cfg.format.sampleFormat);
    hw->srcFmtBuff = (uint8_t *)xmcMalloc(
        hw->cfg.latencySamples * bytesPerSample, XMC_HEAP_CAP_DMA);
    if (!hw->srcFmtBuff) {
      stop();
      XMC_ERR_RET(XMC_ERR_RAM_ALLOC_FAILED);
    }
  } else {
    hw->srcFmtBuff = nullptr;
  }

  hw->s16Buff = (int16_t *)xmcMalloc(hw->cfg.latencySamples * sizeof(int16_t),
                                     XMC_HEAP_CAP_DMA);
  if (!hw->s16Buff) {
    stop();
    XMC_ERR_RET(XMC_ERR_RAM_ALLOC_FAILED);
  }

  i2s_chan_config_t chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  err = i2s_new_channel(&chan_cfg, &hw->audio_i2s_ch, nullptr);
  if (err != ESP_OK) {
    stop();
    XMC_ERR_RET(XMC_ERR_SPEAKER_INIT_FAILED);
  }

  i2s_pdm_tx_clk_config_t clk_cfg =
      I2S_PDM_TX_CLK_DAC_DEFAULT_CONFIG(cfg.format.rateHz);
  i2s_pdm_tx_slot_config_t slot_cfg = I2S_PDM_TX_SLOT_DAC_DEFAULT_CONFIG(
      I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
  i2s_pdm_tx_config_t tx_cfg{
      .clk_cfg = clk_cfg,
      .slot_cfg = slot_cfg,
      .gpio_cfg =
          {
              .clk = I2S_GPIO_UNUSED,
              .dout = (gpio_num_t)pin,
              .dout2 = I2S_GPIO_UNUSED,
              .invert_flags =
                  {
                      .clk_inv = false,
                  },
          },
  };
  err = i2s_channel_init_pdm_tx_mode(hw->audio_i2s_ch, &tx_cfg);
  if (err != ESP_OK) {
    stop();
    XMC_ERR_RET(XMC_ERR_SPEAKER_INIT_FAILED);
  }

  if (actualRateHz) {
    // todo: calculate actualRateHz based on cfg.format.rateHz and
    // hardware capabilities
    *actualRateHz = cfg.format.rateHz;
  }

  maintain(*this, true);

  err = i2s_channel_enable(hw->audio_i2s_ch);
  if (err != ESP_OK) {
    stop();
    XMC_ERR_RET(XMC_ERR_SPEAKER_INIT_FAILED);
  }

  return XMC_OK;
}

XmcStatus StreamingDac::stop() {
  if (handle) {
    SdacHwEsp *hw = (SdacHwEsp *)handle;
    if (hw->audio_i2s_ch) {
      i2s_channel_disable(hw->audio_i2s_ch);
      i2s_del_channel(hw->audio_i2s_ch);
      hw->audio_i2s_ch = nullptr;
    }
    if (hw->srcFmtBuff) {
      xmcFree(hw->srcFmtBuff);
      hw->srcFmtBuff = nullptr;
    }
    if (hw->s16Buff) {
      xmcFree(hw->s16Buff);
      hw->s16Buff = nullptr;
    }
    gpio::setDir(pin, false);
  }
  return XMC_OK;
}

StreamFormat StreamingDac::getStreamFormat() const {
  if (!handle) return {};
  SdacHwEsp *hw = (SdacHwEsp *)handle;
  return hw->cfg.format;
}

XmcStatus StreamingDac::setSource(SourcePort *src) {
  this->source = *src;
  return XMC_OK;
}

XmcStatus StreamingDac::service() {
  maintain(*this, false);
  return XMC_OK;
}

static void maintain(StreamingDac &inst, bool preload) {
  if (!inst.handle) return;

  SdacHwEsp *hw = (SdacHwEsp *)inst.handle;

  int fillSize, playSize;

  playSize = hw->writePos - hw->readPos;
  if (playSize < 0 || (playSize == 0 && hw->full)) {
    pdmWrite(inst, hw->cfg.latencySamples - hw->readPos, preload);
  }
  playSize = hw->writePos - hw->readPos;
  if (playSize > 0) {
    pdmWrite(inst, playSize, preload);
  }

  fillSize = hw->readPos - hw->writePos;
  if (fillSize < 0 || (fillSize == 0 && !hw->full)) {
    fillBuffer(inst, hw->cfg.latencySamples - hw->writePos);
  }
  fillSize = hw->readPos - hw->writePos;
  if (fillSize > 0) {
    fillBuffer(inst, fillSize);
  }
}

static void fillBuffer(StreamingDac &inst, uint32_t fillSamples) {
  SdacHwEsp *hw = (SdacHwEsp *)inst.handle;
  uint32_t bytesPerSample = getBytesPerSample(hw->cfg.format.sampleFormat);
  uint32_t fillBytes = fillSamples * bytesPerSample;
  if (inst.source.requestData) {
    switch (hw->cfg.format.sampleFormat) {
      default:
      case SampleFormat::LINEAR_PCM_S16_MONO:
        memset(hw->s16Buff + hw->writePos, 0x00, fillBytes);
        inst.source.requestData(hw->s16Buff + hw->writePos, fillSamples,
                                inst.source.context);
        break;
      case SampleFormat::LINEAR_PCM_U8_MONO:
        memset(hw->srcFmtBuff + hw->writePos, 0x80, fillBytes);
        inst.source.requestData(hw->srcFmtBuff + hw->writePos, fillSamples,
                                inst.source.context);
        for (int i = 0; i < fillSamples; i++) {
          hw->s16Buff[hw->writePos + i] =
              ((int16_t)hw->srcFmtBuff[hw->writePos + i] - 128) << 8;
        }
        break;
    }
  } else {
    memset(hw->s16Buff + hw->writePos, 0x00, fillBytes);
  }
  hw->writePos = (hw->writePos + fillSamples) % hw->cfg.latencySamples;
  if (hw->writePos == hw->readPos) {
    hw->full = true;
  }
}

static void pdmWrite(StreamingDac &inst, uint32_t numSamples, bool preload) {
  esp_err_t err;
  SdacHwEsp *hw = (SdacHwEsp *)inst.handle;
  uint32_t bytesPerSample = getBytesPerSample(hw->cfg.format.sampleFormat);
  size_t bytesToWrite = numSamples * bytesPerSample;
  size_t bytesWritten = 0;
  const void *data = &hw->s16Buff[hw->readPos];
  if (preload) {
    err = i2s_channel_preload_data(hw->audio_i2s_ch, data, bytesToWrite,
                                   &bytesWritten);
  } else {
    err = i2s_channel_write(hw->audio_i2s_ch, data, bytesToWrite, &bytesWritten,
                            0);
  }
  if (err != ESP_OK) {
    XMC_ERR_LOG(XMC_ERR_SPEAKER_STREAM_BROKEN);
  }
  if (bytesWritten > 0) {
    hw->readPos =
        (hw->readPos + bytesWritten / bytesPerSample) % hw->cfg.latencySamples;
    hw->full = false;
  }
}

}  // namespace xmc::audio
