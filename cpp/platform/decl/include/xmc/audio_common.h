#ifndef XMC_AUDIO_COMMON_H
#define XMC_AUDIO_COMMON_H

#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
  XMC_SAMPLE_LINEAR_PCM_U8_MONO = (1 << 0),
  XMC_SAMPLE_LINEAR_PCM_U16_MONO = (1 << 1),
} xmc_sample_format_t;

typedef struct {
  xmc_sample_format_t sample_format;
  uint32_t sample_rate_hz;
} xmc_stream_format_t;

typedef void (*xmc_stream_request_t)(void *buffer, uint32_t num_samples,
                                     void *context);

typedef struct {
  xmc_stream_request_t request_data;
  void *context;
} xmc_stream_source_t;

static inline int xmc_audio_get_bytes_per_sample(xmc_sample_format_t format) {
  switch (format) {
    case XMC_SAMPLE_LINEAR_PCM_U8_MONO: return 1;
    case XMC_SAMPLE_LINEAR_PCM_U16_MONO: return 2;
    default: return 0;
  }
}

#if defined(__cplusplus)
}
#endif

#endif
