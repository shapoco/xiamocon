#ifndef XMC_SPEAKER_H
#define XMC_SPEAKER_H

#include "xmc/hw/sdac.h"

#if defined(__cplusplus)
extern "C" {
#endif

static inline xmc_audio_sample_format_t xmc_speaker_get_supported_formats(void) {
  return xmc_sdac_get_supported_formats();
}

xmc_status_t xmc_speaker_init(xmc_audio_sample_format_t format, uint32_t sample_rate_hz, uint32_t latency_samples,
                              float *actual_rate_hz);
xmc_status_t xmc_speaker_deinit(void);
xmc_status_t xmc_speaker_set_muted(bool muted);
xmc_status_t xmc_speaker_set_source_port(xmc_audio_source_port_t *src);
xmc_status_t xmc_speaker_service(void);

#if defined(__cplusplus)
}
#endif

#endif
