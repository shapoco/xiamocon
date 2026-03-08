#ifndef XMC_HW_AUDIO_DAC_H
#define XMC_HW_AUDIO_DAC_H

#include "xmc/audio_common.h"
#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
  void *hw;
  int pin;
  xmc_audio_source_port_t source;
} xmc_sdac_inst_t;

typedef struct {
  xmc_audio_format_t format;
  uint32_t latency_samples;
} xmc_sdac_config_t;

xmc_audio_sample_format_t xmc_sdac_get_supported_formats(void);
xmc_status_t xmc_sdac_init(xmc_sdac_inst_t *inst, int pin,
                           const xmc_sdac_config_t *cfg, float *actual_rate_hz);
xmc_status_t xmc_sdac_deinit(xmc_sdac_inst_t *inst);
xmc_status_t xmc_sdac_set_source(xmc_sdac_inst_t *inst,
                                 xmc_audio_source_port_t *src);
xmc_status_t xmc_sdac_service(xmc_sdac_inst_t *inst);

#if defined(__cplusplus)
}
#endif

#endif
