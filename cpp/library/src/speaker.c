#include "xmc/speaker.h"
#include "xmc/hw/pins.h"
#include "xmc/ioex.h"

xmc_sdac_inst_t sdac;

xmc_status_t xmc_speaker_init(xmc_sample_format_t fmt, uint32_t rate,
                              uint32_t latency, float *actual_rate_hz) {
  xmc_stream_format_t sdac_fmt;
  xmc_sdac_config_t cfg;
  sdac_fmt.sample_format = fmt;
  sdac_fmt.sample_rate_hz = rate;
  cfg.format = sdac_fmt;
  cfg.latency_samples = latency;
  XMC_ERR_RET(xmc_ioex_write(XMC_IOEX_PIN_SPEAKER_MUTE, 1));
  XMC_ERR_RET(xmc_ioex_set_dir(XMC_IOEX_PIN_SPEAKER_MUTE, true));
  XMC_ERR_RET(xmc_sdac_init(&sdac, XMC_PIN_AUDIO_OUT, &cfg, actual_rate_hz));
  return XMC_OK;
}

xmc_status_t xmc_speaker_deinit(void) { return xmc_sdac_deinit(&sdac); }

xmc_status_t xmc_speaker_set_muted(bool muted) {
  return xmc_ioex_write(XMC_IOEX_PIN_SPEAKER_MUTE, muted ? 1 : 0);
}

xmc_status_t xmc_speaker_set_source(xmc_stream_source_t *src) {
  return xmc_sdac_set_source(&sdac, src);
}

xmc_status_t xmc_speaker_service(void) { return xmc_sdac_service(&sdac); }
